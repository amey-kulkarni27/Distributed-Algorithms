#pragma once

#include <iostream>
#include <string>
#include <set>
#include <cassert>

#include "parser.hpp"
#include "Helper.hpp"

#include "Stubborn.hpp"

class PLSenderSend{
	
public:
	Stubborn s;

	PLSenderSend(unsigned long id_, const char *ip, unsigned short port) : s(ip, port), id(id_){
	}

	int getSocket(){
		return (this->s).getSocket();
	}

	void pp2pSend(std::string msg){
		// append ts to the start of the message so that the receiver knows
		// get stubborn links to infinitely send that message
		msg = std::to_string(ts) + "_" + msg;
		// Also add the id so receiver knows who they have received the message from
		msg = std::to_string(id) + "_" + msg;
		(this->s).sp2pSend(ts, msg);
		ts++;
	}

	void stopAll(){
		(this->s).stopAll();
	}


private:
	const unsigned long id;
	unsigned long ts = 1;

};
