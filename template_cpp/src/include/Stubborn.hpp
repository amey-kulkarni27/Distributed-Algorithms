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

#include "FLSend.hpp"

namespace std {
	template <>
	struct hash<std::pair<long unsigned int, long long unsigned int>> {
		size_t operator()(const std::pair<long unsigned int, long long unsigned int>& p) const {

			// The custom hash function separately hashes the first and second value of the pair
			return hash<long unsigned int>()(p.first) ^ (hash<long long unsigned int>()(p.second) << 1);
		}
	};
}

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
		tsToMsg[make_pair(h_id, ts)] = msg;
}

	void sp2pStop(unsigned long h_id, unsigned long long ts){
		const std::lock_guard<std::mutex> lock(mapLock);
		tsToMsg.erase(make_pair(h_id, ts));
	}

	void stopAll(){
		keep_sending = false;
		(this->fls).stopAll();
	}

private:
	FLSend fls;
	std::unordered_map<std::pair<unsigned long, unsigned long long>, std::string> tsToMsg;
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
