#pragma once

#include <iostream>
#include <string>
#include <set>
#include <unordered_map>

#include "parser.hpp"
#include "Proposer.hpp"
#include "Stubborn.hpp"
#include "FLSend.hpp"
#include "Acceptor.hpp"
#include "Helper.hpp"


class PLReceive{
	
public:
	PLReceive(FLSend &fls_, Stubborn &s_, PLBroadcast &plb, Proposer &prop_, unsigned long n, unsigned long curId, unsigned long num_proposals): acc(plb, curId, num_proposals), fls(fls_), s(s_), prop(prop_), selfId(curId) {
	}

	void pp2pReceive(std::string recvMsg){

		// Two types of recvMsg 1) A_hid_plid, 2) Actual message = plid_hid_P/R

		if(recvMsg[0] == 'A'){
			size_t first_underscore = recvMsg.find('_');
			size_t second_underscore = recvMsg.find('_', first_underscore + 1);
			std::string hidStr = recvMsg.substr(first_underscore + 1, second_underscore - first_underscore - 1);
			std::string plidStr = recvMsg.substr(second_underscore + 1);
			// get stubborn links to stop sending that message
			(this -> s).sp2pStop(std::stoul(hidStr), std::stoull(plidStr));
		}
		else{
			std::string msg = recvMsg;
			size_t firstUnderscore = msg.find('_');
			std::string plidStr = msg.substr(0, firstUnderscore);
			size_t secondUnderscore = msg.find('_', firstUnderscore + 1);
			std::string hidStr = msg.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
			std::string msgWithoutSenderDetails = msg.substr(secondUnderscore + 1); // there will always be something to the right of the second underscore
			std::string ackMsg = "A_" + std::to_string(selfId) + "_" + plidStr;
			unsigned long hid = std::stoul(hidStr);
			(this -> fls).fp2pSend(hid, ackMsg);
			unsigned long long plid = std::stoull(plidStr);
			if(delivered.find(hid) == delivered.end() || delivered[hid].find(plid) == delivered[hid].end()){
				delivered[hid].insert(plid);
				size_t underscore = msgWithoutSenderDetails.find('_');
				if(msgWithoutSenderDetails[0] == 'P')
					(this->acc).process(msgWithoutSenderDetails.substr(underscore + 1), hid);
				else
					(this->prop).response(msgWithoutSenderDetails.substr(underscore + 1));
			}
		}
	}


private:
	Acceptor acc;
	FLSend &fls;
	Stubborn &s;
	Proposer &prop;
	unsigned long selfId;
	std::unordered_map<unsigned long, std::set<unsigned long long>> delivered;

};
