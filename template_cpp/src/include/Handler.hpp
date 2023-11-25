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
#include "FUBroadcast.hpp" 
#include "FLReceive.hpp" 


class Handler{

public:

	// Constructor named initialise, because we wanted to create a global object
	Handler(unsigned long id, const char *outputPath, unsigned long num_messages_, std::vector<Parser::Host> hosts) : fub(id, hosts), lg(outputPath), flr((this->fub).getFLSend(), (this->fub).getStubborn(), (this->fub).getPLBroadcast(), (this->fub).getSocket(), id, hosts, this->lg){
		num_messages = num_messages_;
	}

	void startExchange(){
		broadcast();
	}

	void stopExchange(){
		// stop broadcasting
		(this->fub).stopAll();

		// stop receiving
		(this->flr).stopAll();
		
		// log till the end
		(this->lg).stopAll();
	}

private:
	FUBroadcast fub;
	Logger lg;
	FLReceive flr;
	unsigned long num_messages;



	std::string createMsgAppendToLogs(unsigned long st, unsigned long en){
		std::string payload = "";
		while(st < en){
			std::string msg = std::to_string(st);
			(this->lg).log(msg, false, -1);
			payload += msg + "_";
			st++;
		}
		return payload;
	}

	void broadcast(){
		// 1) Create packets containing 8 messages
		// 2) Log them and send them through the perfect links abstraction
		unsigned long i = 1;
		while(i <= num_messages){
			unsigned long end = std::min(i + 8, num_messages + 1);
			std::string msgToSend = createMsgAppendToLogs(i, end);
			(this->fub).broadcast(msgToSend);
			i = end;
		}
	}

};
