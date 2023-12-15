#pragma once

#include <iostream>
#include <functional>
#include <utility>
#include <string>
#include <set>

#include "parser.hpp"
#include "PLBroadcast.hpp"
#include "Helper.hpp"

class Acceptor{

public:
	Acceptor(PLBroadcast &plb_, unsigned long selfId_): plb(plb_), selfId(selfId_){
	}

	void process(std::string msg, unsigned long from){
		std::vector<char> returnChar; // ACK or NACK ('Y' or 'N')
		std::vector<unsigned long> inds, timestamps; // what positions and timestamps
		std::vector<std::set<unsigned long>> toAdd; // the elements that should be added

		size_t curPos = 0;
		size_t nxtPos = msg.find('_', curPos);
		bool iObt = false, tsObt = false;
		std::set<unsigned long> curSet;
		
		// Code to read and process the proposal
		while(nxtPos != std::string::npos){
			std::string betweenUnders = msg.substr(curPos, nxtPos - curPos);
			if(betweenUnders == "|"){
				toAdd.push_back(curSet);
				curSet.clear();
				iObt = false, tsObt = false;
				std::set<unsigned long> diffResult;
				std::set_difference(accepteds[inds.back()].begin(), accepteds[inds.back()].end(), curSet.begin(), curSet.end(), std::inserter(diffResult, diffResult.begin()));
				if(diffResult.empty())
					returnChar.push_back('Y');
				else
					returnChar.push_back('N');
				toAdd.push_back(diffResult);
			}
			else if(!iObt){
				inds.push_back(std::stoul(betweenUnders));
				iObt = true;
			}
			else if(!tsObt){
				timestamps.push_back(std::stoul(betweenUnders));
				tsObt = true;
			}
			else
				curSet.insert(std::stoul(betweenUnders));

			curPos = nxtPos + 1;
            nxtPos = msg.find('_', curPos);
		}

		std::string returnMsg = "R_";		
		for(unsigned long j = 0; j < returnChar.size(); j++){
			char r = returnChar[j];
			unsigned long i = inds[j], ts = timestamps[j];
			returnMsg += r + "_" + std::to_string(i) + "_" + std::to_string(ts) + "_";
			if(r == 'N')
				for(const unsigned long &x: toAdd[j])
					returnMsg += std::to_string(x) + "_";
			returnMsg += "|_";
		}
		(this->plb).send(returnMsg, from);
	}


private:
	PLBroadcast &plb;
    unsigned long selfId;
    std::vector<std::set<unsigned long>> accepteds;

};
