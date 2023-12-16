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
	Acceptor(PLBroadcast &plb_, unsigned long selfId_, unsigned long num_proposals): plb(plb_), selfId(selfId_){
		accepteds.resize(num_proposals);
	}

	void process(std::string msg, unsigned long from){
		// std::cout<<"Received "<<msg<<" from "<<from<<std::endl;
		std::vector<std::string> returnChar; // ACK or NACK ('Y' or 'N')
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
				iObt = false, tsObt = false;
				std::set<unsigned long> diffResult;
				std::set_difference(accepteds[inds.back()].begin(), accepteds[inds.back()].end(), curSet.begin(), curSet.end(), std::inserter(diffResult, diffResult.begin()));
				if(diffResult.empty()){
					returnChar.push_back("Y");
					accepteds[inds.back()] = curSet;
				}
				else{
					returnChar.push_back("N");
					std::set<unsigned long> unionSet;
					std::set_union(accepteds[inds.back()].begin(), accepteds[inds.back()].end(), curSet.begin(), curSet.end(), std::inserter(unionSet, unionSet.begin()));
					accepteds[inds.back()] = unionSet;
				}
				toAdd.push_back(diffResult);
				curSet.clear();
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
			std::string r = returnChar[j];
			unsigned long i = inds[j], ts = timestamps[j];
			returnMsg += r + "_" + std::to_string(i) + "_" + std::to_string(ts) + "_";
			if(r == "N")
				for(const unsigned long &x: toAdd[j])
					returnMsg += std::to_string(x) + "_";
			returnMsg += "|_";
		}
		returnMsg = std::to_string(selfId) + "_" + returnMsg;
		// std::cout<<"Sending "<<returnMsg<<" to "<<from<<std::endl;
		(this->plb).send(returnMsg, from);
	}


private:
	PLBroadcast &plb;
    unsigned long selfId;
    std::vector<std::set<unsigned long>> accepteds;

};
