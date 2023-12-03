#pragma once

#include <iostream>
#include <string>
#include <set>
#include <cassert>
#include <mutex>
#include <semaphore.h>

#include "parser.hpp"
#include "Helper.hpp"

#include "PLBroadcast.hpp"
#include "Stubborn.hpp"
#include "FLSend.hpp"

class FUBroadcast{
	
public:

	FUBroadcast(unsigned long id_, std::vector<Parser::Host> hosts) : plb(id_, hosts), id(id_), MAX_MSGS(static_cast<unsigned int>(1000/hosts.size())){
		sem_init(&spotsLeft, 0, MAX_MSGS);
		sem_init(&spotsFilled, 0, 0);
	}

	~FUBroadcast(){
		sem_destroy(&spotsLeft);
		sem_destroy(&spotsFilled);
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

	void broadcast(std::string msg){
		// append self id
		msg = std::to_string(id) + "_" + msg;

		// append timestamp for FIFO property
		msg = std::to_string(ts) + "_" + msg;
		ts++;

		// append the sender id (here same as the self id) for the URB property (I am not really using it)
		msg = std::to_string(id) + "_" + msg;


		sem_wait(&spotsLeft);
		// Get the perfect links to broadcast the message
		(this->plb).broadcast(msg);
		sem_post(&spotsFilled);
	}

	void msgDelivered(){
		// One less message in the network
		sem_wait(&spotsFilled);
		sem_post(&spotsLeft);
	}

	void stopAll(){
		(this->plb).stopAll();
	}


private:
	PLBroadcast plb;
	const unsigned long id;
	unsigned long ts = 1;
	sem_t spotsLeft, spotsFilled;
	unsigned int MAX_MSGS = 500;

};
