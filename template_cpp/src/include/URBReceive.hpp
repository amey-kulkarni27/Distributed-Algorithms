#pragma once

#include <iostream>
#include <string>

#include "parser.hpp"
#include "PLBroadcast.hpp"
#include "FIFOReceive.hpp"
#include "Helper.hpp"

class URBReceive{

public:
	URBReceive(PLBroadcast *plb_, unsigned long n, unsigned long selfId_): fifor(), plb(plb_), N(n), selfId(selfId_){
	}

	void deliver(string msg){

		size_t firstUnderscore = msg.find('_');
		std::string tsStr = msg.substr(0, firstUnderscore);
		size_t secondUnderscore = msg.find('_', firstUnderscore + 1);
		std::string idStr = msg.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
		std::string contents = msg.substr(secondUnderscore + 1); 
		unsigned long ts = std::stoul(tsStr);
		unsigned long originalId = std::stoul(idStr);
		pair<unsigned long, unsigned long> uniqueMsgId = make_pair(originalId, ts);

		if(acks.find(uniqueMsgId) != acks.end() && acks[uniqueMsgId] > N / 2)
			return;

		if(acks.find(uniqueMsgId) == acks.end()){
			acks[uniqueMsgId] = 1; // itself
			std::string msgToBroadcast = std::to_string(selfId) + "_" + msg;
			(this->plb) -> broadcast(msgToBroadcast);
		}

		acks[uniqueMsgId]++;
		if(acks[uniqueMsgId] > N / 2)
			(this->fifor).deliver(originalId, ts, contents);

	}

private:
	FIFOReceive fifor;
	PLBroadcast *plb;
	const unsigned long N;
	const unsigned long selfId;
	unordered_map<pair<unsigned long, unsigned long>, unsigned long > acks;

};
