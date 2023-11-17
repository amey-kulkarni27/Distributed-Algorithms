#pragma once

#include <iostream>
#include <string>
#include <set>
#include <cassert>

#include "parser.hpp"
#include "Helper.hpp"

#include "PLBroadcast.hpp"

class FUBroadcast{
	
public:

	FUBroadcast(unsigned long id_, std::vector<Parser::Host> hosts) : plb(id_, hosts), id(id_){
	}

	int getSocket(){
		return (this->plb).getSocket();
	}

	void broadcast(std::string msg){
		// append self id
		msg = std::to_string(id) + "_" + msg;

		// append timestamp for FIFO property
		msg = std::to_string(ts) + "_" + msg;
		ts++;

		// append the sender id (here same as the self id) for the URB property
		msg = std::to_string(id) + "_" + msg;


		// Get the perfect links to broadcast the message
		(this->plb).PLBroadcast(ts, msg);
	}

	void stopAll(){
		(this->plb).stopAll();
	}


private:
	PLBroadcast plb;
	const unsigned long id;
	unsigned long ts = 1;

};
