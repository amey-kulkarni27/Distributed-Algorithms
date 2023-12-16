#pragma once 
 
#include <iostream> 
#include <string>
#include <vector>
#include <unordered_set>
#include <mutex>

#include "Helper.hpp"
#include "Logger.hpp"

#include "PLBroadcast.hpp"

class Proposer{

public:
    Proposer(const char *configPath, const char *outputPath, std::vector<Parser::Host> hosts, unsigned long curId) : plb(curId, hosts), lg(outputPath, configPath), selfId(curId){
        Helper::readParams(configPath, num_proposals, vs, ds, proposals);
        active.resize(num_proposals, true);
        active_proposal_ts.resize(num_proposals, 0); // every proposal has a time-stamp
        ack_count.resize(num_proposals, 0); // 1 for itself
        nack_count.resize(num_proposals, 0);
        num_active = num_proposals;
        to_be_broadcast = num_proposals;
        n = hosts.size();

    }

	FLSend& getFLSend(){
		return (this->plb).getFLSend();
	}

	Stubborn& getStubborn(){
		return (this->plb).getStubborn();
	}

	PLBroadcast& getPLBroadcast(){
		return (this->plb);
	}

	int getSocket(){
		return (this->plb).getSocket();
	}

	unsigned long getProposals(){
		return this->num_proposals;
	}

    void propose(){
        // if the ts is 0, send it, otherwise check if ack_count + nack_count > n/2
        // if it is, reset the counts
        std::vector<unsigned long> inds;
        while(num_active > 0){
            for(unsigned long i = 0; i < num_proposals; i++){
                if(check(inds, i)){
					packAndBroadcast(inds);
				}
            }
        }
    }

    void response(std::string responseMsg){
        std::time_t t = std::time(nullptr);
		std::cout << "Current Time: " << std::asctime(std::localtime(&t));
		std::cout<<"Received "<<responseMsg<<std::endl;
        size_t curPos = 0;
		size_t nxtPos = responseMsg.find('_', curPos);
        bool cObt = false, iObt = false, tsObt = false;
        std::string ret = "";
        unsigned long i = 0, ts = 0;
        std::unordered_set<unsigned long> curSet;
        // Code to read and process the feedback
		while(nxtPos != std::string::npos){
			std::string betweenUnders = responseMsg.substr(curPos, nxtPos - curPos);
			if(betweenUnders == "|"){
                update(ret, i, ts, curSet);
                ret = "", cObt = false, iObt = false, tsObt = false;
				curSet.clear();
			}
            else if(!cObt){
                ret = betweenUnders;
                cObt = true;
            }
			else if(!iObt){
				i = std::stoul(betweenUnders);
				iObt = true;
			}
			else if(!tsObt){
				ts = std::stoul(betweenUnders);
				tsObt = true;
			}
			else
				curSet.insert(std::stoul(betweenUnders));
            curPos = nxtPos + 1;
            nxtPos = responseMsg.find('_', curPos);
		}
    }

    void stopAll(){
        (this->plb).stopAll();
    }

private:
    PLBroadcast plb;
    Logger lg;
    unsigned long selfId;
    unsigned long num_proposals;
    unsigned long vs;
    unsigned long ds;
    std::vector<std::unordered_set<unsigned long>> proposals;
    std::vector<bool> active;
    std::vector<unsigned long> active_proposal_ts;
    std::vector<unsigned long> ack_count;
    std::vector<unsigned long> nack_count;
    unsigned long num_active;
    unsigned long to_be_broadcast;
    std::string msg;
    unsigned long n;
    std::mutex proposalLock;

    void packAndBroadcast(std::vector<unsigned long> &inds){
		const std::lock_guard<std::mutex> lock(proposalLock);
        std::string payload = std::to_string(selfId) + "_P_"; // indicating proposal and by who (for resending purposes)
        for(const unsigned long ind: inds){
            unsigned long ts = active_proposal_ts[ind];
            payload += std::to_string(ind) + "_" + std::to_string(ts) + "_";
            for(const unsigned long &num: proposals[ind]){
                payload += std::to_string(num) + "_";
            }
            payload += "|_";
        }
		inds.clear();
        std::time_t t = std::time(nullptr);
		std::cout << "Current Time: " << std::asctime(std::localtime(&t));
		std::cout<<"Sending "<<payload<<" to all"<<std::endl;
        (this -> plb).broadcast(payload);
    }

    bool check(std::vector<unsigned long> &inds, unsigned long i){
        const std::lock_guard<std::mutex> lock(proposalLock);
        if(!active[i])
            return false;
		if(ack_count[i] > n / 2){
			num_active--;
			active[i] = false;
			(this->lg).logAndFlush(i, proposals[i]);
			return false;
		}
		if(inds.size() >= std::min(8ul, to_be_broadcast) and to_be_broadcast > 0){
			to_be_broadcast -= inds.size();
			return true;
		}
        if(active_proposal_ts[i] == 0 or (ack_count[i] + nack_count[i] > n / 2)){
			inds.push_back(i);
			ack_count[i] = 0;
			nack_count[i] = 0;
			if(active_proposal_ts[i] != 0)
				to_be_broadcast++;
			active_proposal_ts[i]++;
        }
        return false;
    }



    void update(std::string &ret, unsigned long &i, unsigned long &ts, std::unordered_set<unsigned long> toAdd){
        const std::lock_guard<std::mutex> lock(proposalLock);
        if(ts < active_proposal_ts[i])
            return;
        assert(ts == active_proposal_ts[i]);
        if(ret == "Y")
            ack_count[i]++;
        else{
            nack_count[i]++;
            proposals[i].insert(toAdd.begin(), toAdd.end());
        }
    }
};
