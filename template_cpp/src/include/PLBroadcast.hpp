#pragma once

#include <iostream>
#include <string>
#include <set>
#include <cassert>
#include <mutex>

#include "parser.hpp"
#include "Helper.hpp"

#include "Stubborn.hpp"
#include "FLSend.hpp"

class PLBroadcast{
	
public:

	PLBroadcast(unsigned long id_, std::vector<Parser::Host> hosts) : s(hosts), id(id_){
		for(auto host: hosts){
			unsigned long h_id = host.id;
			pl_ids[h_id] = 1;
			if(h_id != id)
				ids.push_back(h_id);
			// ids contains all the host ids except its own
		}
	}

	FLSend getFLSend(){
		return (this->s).getFLSend();
	}

	Stubborn getStubborn(){
		return this->s;
	}

	int getSocket(){
		return (this->s).getSocket();
	}

	void broadcast(std::string msg){
		// append pl_id to the start of the message so that the receiver knows
		// get stubborn links to infinitely send that message
		const std::lock_guard<std::mutex> lock(broadcastLock);
		for(const &int h_id: ids){
			msg = std::to_string(pl_ids[h_id]) + "_" + msg;
			(this->s).sp2pSend(h_id, pl_ids[h_id], msg);
			pl_ids[h_id]++;
		}
	}

	void stopAll(){
		(this->s).stopAll();
	}


private:
	Stubborn s;
	const unsigned long id;
	map<unsigned long, unsigned long long> pl_ids;
	std::vector<unsigned long> ids;
	std::mutex broadcastLock; // Since both URBSend and URBReceive call this broadcast function

};
