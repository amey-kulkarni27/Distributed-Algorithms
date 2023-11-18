#pragma once

#include <iostream>
#include <string>
#include <set>

#include "parser.hpp"
#include "PLBroadcast.hpp"
#include "Stubborn.hpp"
#include "FLSend.hpp"
#include "URBReceive.hpp"
#include "Helper.hpp"


class PLReceive{
	
public:
	PLReceive(FLSend *fls_, Stubborn *s_, PLBroadcast *plb_, unsigned long n, unsigned long curId, Logger *lg): urbr(plb_, n, curId, lg), fls(fls_), s(s_) {
	}

	void pp2pReceive(std::string recvMsg){

		// Two types of recvMsg 1) ACK_hid_plid, 2) Actual message

		if(recvMsg[0] == 'A'){
			size_t first_underscore = recvMsg.find('_');
			size_t second_underscore = recvMsg.find('_', first_underscore + 1);
			std::string hidStr = recvMsg.substr(first_underscore + 1, second_underscore - first_underscore - 1);
			std::string plidStr = recvMsg.substr(second_underscore + 1);
			// get stubborn links to stop sending that message
			(this -> s) -> sp2pStop(std::stoul(hidStr), std::stoull(plidStr));
		}
		else{
			size_t firstUnderscore = msg.find('_');
			std::string plidStr = msg.substr(0, firstUnderscore);
			size_t secondUnderscore = msg.find('_', firstUnderscore + 1);
			std::string hidStr = msg.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
			std::string msgWithoutSenderDetails = msg.substr(secondUnderscore + 1); // there will always be something to the right of the second underscore
			std::string ackMsg = "A_" + hidStr + "_" + plidStr;
			unsigned long hid = std::stoul(hidStr);
			(this -> fls) -> fp2pSend(hid, ackMsg);
			unsigned long long plid = std::stoull(plidStr);
			if(delivered.find(hid) == delivered.end() || delivered[hid].find(plid) == delivered[hid].end()){
				delivered[hid].insert(plid);
				(this->urbr).deliver(msgWithoutSenderDetails);
			}
		}
	}


private:
	URBReceive urbr;
	FLSend *fls;
	Stubborn *s;
	std::unordered_map<unsigned long, std::unordered_set<unsigned long long>> delivered;

};
