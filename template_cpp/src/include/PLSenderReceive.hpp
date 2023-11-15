#pragma once

#include <iostream>
#include <string>
#include <set>

#include "parser.hpp"
#include "Helper.hpp"

#include "Stubborn.hpp"

// called by FLSenderReceive Exclusively
class PLSenderReceive{
	
public:
	PLSenderReceive(Stubborn *x) {
		s = x;
	}

	void pp2pReceive(std::string ackMsg){
		// get the ID of the message
		// get stubborn links to stop sending that message

		unsigned long ts = std::stoul(ackMsg); // ackMsg is just a single number, as a string
		std::pair<std::set<unsigned long>::iterator,bool> ret; // store return value of insert
		ret = acked.insert(ts);
		if(ret.second)
			(this->s) -> sp2pStop(ts);
	}


private:
	Stubborn *s;
	std::set<unsigned long> acked; // those messages that we have received an ACK for

};
