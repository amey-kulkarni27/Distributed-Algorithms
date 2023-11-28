#pragma once

#include <iostream>
#include <functional>
#include <utility>
#include <string>
#include <unordered_map>

#include "parser.hpp"
#include "PLBroadcast.hpp"
#include "FIFOReceive.hpp"
#include "Helper.hpp"
#include "Logger.hpp"

struct LongHash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return h1 ^ h2;  
    }
};

class URBReceive{

public:
	URBReceive(PLBroadcast &plb_, unsigned long n, unsigned long selfId_, Logger &lg): fifor(lg, n), plb(plb_), N(n), selfId(selfId_){
	}

	void deliver(std::string msg){

		size_t firstUnderscore = msg.find('_');
		std::string tsStr = msg.substr(0, firstUnderscore);
		size_t secondUnderscore = msg.find('_', firstUnderscore + 1);
		std::string idStr = msg.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
		std::string contents = msg.substr(secondUnderscore + 1); 
		unsigned long ts = std::stoul(tsStr);
		unsigned long originalId = std::stoul(idStr);
		std::pair<unsigned long, unsigned long> uniqueMsgId = std::make_pair(originalId, ts);

		if(acks.find(uniqueMsgId) != acks.end() && acks[uniqueMsgId] > N / 2)
			return;

		if(acks.find(uniqueMsgId) == acks.end()){
			acks[uniqueMsgId] = 1; // itself. ACK that it itself has received it
			std::string msgToBroadcast = std::to_string(selfId) + "_" + msg;
			if(selfId != originalId) // in case they are equal, we have already broadcasted this message
				(this->plb).broadcast(msgToBroadcast);
		}

		acks[uniqueMsgId]++;
		if(acks[uniqueMsgId] > N / 2)
			(this->fifor).deliver(originalId, ts, contents);

	}

	void stopAll(){
		(this->fifor).stopAll();
	}

private:
	FIFOReceive fifor;
	PLBroadcast &plb;
	const unsigned long N;
	const unsigned long selfId;
	std::unordered_map<std::pair<unsigned long, unsigned long>, unsigned long , LongHash> acks;

};
