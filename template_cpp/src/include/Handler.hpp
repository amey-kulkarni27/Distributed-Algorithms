#pragma once 
 
#include <iostream> 
#include <string>
#include <map>
#include <queue>
#include <memory>

#include "parser.hpp" 
#include "Helper.hpp"
#include "Logger.hpp"

// Sender side
#include "Proposer.hpp" 
#include "FLReceive.hpp" 


class Handler{

public:

	// Constructor named initialise, because we wanted to create a global object
	Handler(const char *configPath, const char *outputPath, std::vector<Parser::Host> hosts, unsigned long curId) : p(configPath, outputPath, hosts, curId), flr((this->p).getFLSend(), (this->p).getStubborn(), (this->p).getPLBroadcast(), this->p, (this->p).getSocket(), curId, hosts){
	}

	void startExchange(){
	}

	void stopExchange(){
		// stop broadcasting
		(this->p).stopAll();
	}

private:
	Proposer p;
	FLReceive flr;
	




};
