#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>

#include "parser.hpp"
#include "Helper.hpp"
#include "Logger.hpp"

class FIFOReceive{

public:
	FIFOReceive(Logger &lg_): lg(lg_){
		std::thread searching(&FIFOReceive::searchOrder, this);
		searching.detach();
	}

	void deliver(unsigned long id, unsigned long ts, std::string msg){
		const std::lock_guard<std::mutex> lock(orderLock);
		order[id].push(make_pair(ts, msg));
	}

	void stopAll(){
		
	}

private:
	Logger &lg;
	std::unordered_map<unsigned long, std::priority_queue<std::pair<unsigned long, std::string> > > order;
	std::unordered_map<unsigned long, unsigned long> expected;
	std::mutex orderLock;
	bool sending = true;

	void deliver(std::string msg, unsigned long id){

		size_t curpos = 0;
		size_t found = msg.find('_');
		while(found != std::string::npos){
			std::string underlying_msg = msg.substr(curpos, found - curpos);
			(this->lg).log(underlying_msg, true, id);
			curpos = found + 1;
			found = msg.find('_', curpos);
		}
	}

	int iterateOrder(){
		const std::lock_guard<std::mutex> lock(orderLock);
		if(order.size() == 0)
			return 0;

		for(auto const& [key, val]: order){
			if(expected[key] == val.top().first){
				deliver(val.top().second, key);
				order[key].pop();
				expected[key]++;
			}
		}
		return 1;
	}

	void searchOrder(){
		while(sending)
			if(iterateOrder() == 0)
				std::this_thread::sleep_for(std::chrono::nanoseconds(10));
	}

};
