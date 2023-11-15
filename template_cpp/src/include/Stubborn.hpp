#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <cassert>
#include <thread>
#include <chrono>
#include <mutex>

#include "parser.hpp"
#include "Helper.hpp"

#include "FLSenderSend.hpp"

class Stubborn{

public:
	std::unordered_map<unsigned long, std::string> tsToMsg;

	// Using a member initialiser list to 
	Stubborn(const char *ip, unsigned short port) :fss(ip, port){
		prt = port;

		// use a separate thread
		std::thread contSending(&Stubborn::continuousSend, this);
		contSending.detach();
	}

	int getSocket(){
		return (this->fss).getSocket();
	}

	void sp2pSend(unsigned long ts, std::string msg){
		const std::lock_guard<std::mutex> lock(mapLock);
		tsToMsg[ts] = msg;
	}

	void sp2pStop(unsigned long ts){
		const std::lock_guard<std::mutex> lock(mapLock);
		tsToMsg.erase(ts);
	}

	void stopAll(){
		keep_sending = false;
		(this->fss).stopAll();
	}

private:
	FLSenderSend fss;
	std::mutex mapLock;
	bool keep_sending = true;
	unsigned short prt = 0;

	int flood(){
		const std::lock_guard<std::mutex> lock(mapLock);
		if(tsToMsg.size() == 0)
			return 0;
		// std::cout<<"THREAD: "<<tsToMsg.size()<<std::endl;
		for(auto const& [key, val]: tsToMsg){
			if((this->fss).fp2pSend(val) == -1)
				return 0;
		}
		return 1;
	}

	void continuousSend(){
		
		while(keep_sending){
			if(flood() == 0)
				std::this_thread::sleep_for(std::chrono::nanoseconds(10));
		}
		// Helper::printText("Cont Send Thread terminates");
	}
};
