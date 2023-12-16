#pragma once 
 
#include <iostream>

#include "Helper.hpp"

// Sender side
#include "Proposer.hpp" 
#include "FLReceive.hpp" 


class Handler{

public:

	// Constructor named initialise, because we wanted to create a global object
	Handler(const char *configPath, const char *outputPath, std::vector<Parser::Host> hosts, unsigned long curId) : p(configPath, outputPath, hosts, curId), flr((this->p).getFLSend(), (this->p).getStubborn(), (this->p).getPLBroadcast(), this->p, (this->p).getSocket(), curId, hosts, (this->p).getProposals()){
	}

	void startPropose(){
		(this->p).propose();
	}

	void stopExchange(){
		(this->p).stopAll();
	}

private:
	Proposer p;
	FLReceive flr;

};
