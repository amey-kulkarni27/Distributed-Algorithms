#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <utility>
#include <unordered_map>
#include <cassert>
#include <thread>
#include <chrono>
#include <mutex>

#include "parser.hpp"
#include "Helper.hpp"

#include "FLSend.hpp"

struct LongLongHash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return h1 ^ h2;  
    }
};

class Stubborn{

public:

	// Using a member initialiser list to 
	Stubborn(std::vector<Parser::Host> hosts) :fls(hosts){

		// use a separate thread
		std::thread contSending(&Stubborn::continuousSend, this);
		contSending.detach();
	}

	FLSend& getFLSend(){
		return (this->fls);
	}

	int getSocket(){
		return (this->fls).getSocket();
	}

	void sp2pSend(unsigned long h_id, unsigned long long ts, std::string msg){
		const std::lock_guard<std::mutex> lock(mapLock);
		tsToMsg[std::make_pair(h_id, ts)] = msg;
}

	void sp2pStop(unsigned long h_id, unsigned long long ts){
		const std::lock_guard<std::mutex> lock(mapLock);
		tsToMsg.erase(std::make_pair(h_id, ts));
	}

	void stopAll(){
		keep_sending = false;
		(this->fls).stopAll();
	}

private:
	FLSend fls;
	std::unordered_map<std::pair<unsigned long, unsigned long long>, std::string, LongLongHash> tsToMsg;
	std::mutex mapLock;
	bool keep_sending = true;
	unsigned short prt = 0;

	int flood(){
		const std::lock_guard<std::mutex> lock(mapLock);
		if(tsToMsg.size() == 0)
			return 0;
		for(auto const& [key, val]: tsToMsg){
			if((this->fls).fp2pSend(key.first, val) == -1) // if the port is closed by the main thread
				return 0;
		}
		return 1;
	}

	void continuousSend(){
		
		while(keep_sending){
			if(flood() == 0)
				std::this_thread::sleep_for(std::chrono::nanoseconds(10));
		}
	}
};
