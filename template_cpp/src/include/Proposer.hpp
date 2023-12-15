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
    Proposer(const char *configPath, const char *outputPath, std::vector<Parser::Host> hosts, unsigned long curId) : plb(curId, hosts), lg(outputPath, hosts.size()), selfId(curId){
        if(Helper::readParams(configPath, num_proposals, proposals) == false)
		    std::cerr<<"Failed to read parameters from the config file "<<std::endl;
        active.resize(num_proposals, true);
        active_proposal_ts.resize(num_proposals, 0); // every proposal has a time-stamp
        ack_count.resize(num_proposals, 1); // 1 for itself
        nack_count.resize(num_proposals, 0);
        num_active = num_proposals;
        n = hosts.size();

        // Create proposing thread
        std::thread proposingThread(&Proposer::propose, this);
        proposingThread.detach();
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

    void response(std::string responseMsg){

        size_t curPos = 0;
		size_t nxtPos = responseMsg.find('_', curPos);
        bool cObt = false, iObt = false, tsObt = false;
        std::string ret = "";
        unsigned long i = 0, ts = 0;
        std::unordered_set<unsigned long> curSet;
        // Code to read and process the feedback
		while(nxtPos != std::string::npos){
			std::string betweenUnders = msg.substr(curPos, nxtPos - curPos);
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
    std::vector<std::unordered_set<unsigned long>> proposals;
    std::vector<bool> active;
    std::vector<unsigned long> active_proposal_ts;
    std::vector<unsigned long> ack_count;
    std::vector<unsigned long> nack_count;
    unsigned long num_active;
    std::string msg;
    unsigned long n;
    std::mutex proposalLock;

    void packAndBroadcast(std::vector<unsigned long> &inds){
        std::string payload = std::to_string(selfId) + "_P_"; // indicating proposal and by who (for resending purposes)
        for(const unsigned long ind: inds){
            unsigned long ts = active_proposal_ts[ind];
            payload += std::to_string(ind) + "_" + std::to_string(ts) + "_";
            for(const unsigned long &num: proposals[ind]){
                payload += std::to_string(num) + "_";
            }
            payload += "|_";
        }
        (this -> plb).broadcast(payload);
    }

    bool check(std::vector<unsigned long> &inds, unsigned long i){
        const std::lock_guard<std::mutex> lock(proposalLock);
        if(!active[i])
            return false;
        if(active_proposal_ts[i] == 0 or (ack_count[i] + nack_count[i] > n / 2)){
            if(ack_count[i] > n / 2){
                (this->lg).logAndFlush(i, proposals[i]);

                active[i] = false;
            }
            else{
                inds.push_back(i);
                ack_count[i] = 0;
                nack_count[i] = 0;
                active_proposal_ts[i]++;
                if(inds.size() >= std::min(8ul, num_active))
                    return true;
            }
            
        }
        return false;
    }

    void propose(){
        // if the ts is 0, send it, otherwise check if ack_count + nack_count > n/2
        // if it is, reset the counts
        std::vector<unsigned long> inds;
        while(num_active > 0){
            for(unsigned long i = 0; i < num_proposals; i++){
                if(check(inds, i))
                    packAndBroadcast(inds);
            }
        }
    }

    void update(std::string &ret, unsigned long &i, unsigned long &ts, std::unordered_set<unsigned long> toAdd){
        if(ts < active_proposal_ts[i])
            return;
        assert(ts == active_proposal_ts[i]);
        const std::lock_guard<std::mutex> lock(proposalLock);
        if(ret == "Y")
            ack_count[i]++;
        else{
            nack_count[i]++;
            proposals[i].insert(toAdd.begin(), toAdd.end());
        }
    }
};