#pragma once 
 
#include <iostream> 

#include "Helper.hpp"

// Receiver side
#include "FLReceiverReceive.hpp" 


class HandlerReceiver1 {

public:

	// Constructor named initialise, because we wanted to create a global object
	HandlerReceiver1(const char *outputPath, const char *ip, unsigned short port) : frr(outputPath, ip, port){
	}

	void stopExchange(){
		(this->frr).stopAll();
	}

private:
	FLReceiverReceive frr;

};
