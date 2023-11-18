#pragma once

#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <unordered_map>
#include <priority_queue>
#include <mutex>

#include "parser.hpp"
#include "Helper.hpp"
#include "Logger.hpp"

class FIFOReceive(){

public:
	FIFOReceive(Logger *lg_): lg(lg_){
		std::thread searching(&FIFOReceive::searchOrder, this);
		searching.detach();
	}

	void deliver(unsigned long id, unsigned long ts, std::string msg){
		const std::lock_guard<std::mutex> lock(orderLock);
		order[id].push(make_pair(ts, msg));
	}

private:
	Logger *lg;
	unordered_map<unsigned long, priority_queue<pair<unsigned long, std::string>> > order;
	std::mutex orderLock;
	bool sending = true;

	int iterateOrder(){
		const std::lock_guard<std::mutex> lock(orderLock);
		if(order.size() == 0)
			return 0;

		for(auto const& [key, val]: order)
			(self->lg) -> log(key, val.second, true)
		return 1;
	}

	void searchOrder(){
		while(sending)
			if(iterateOrder() == 0)
				std::this_thread::sleep_for(std::chrono::nanoseconds(10));
	}

};
