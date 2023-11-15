#pragma once

#include <iostream>
#include <string>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <pthread.h>
#include <semaphore.h>

#include "parser.hpp"
#include "FLReceiverSend.hpp"
#include "Helper.hpp"



class PLReceiver{
	
public:
	PLReceiver(const char *oPath) : frs(), filePath(oPath), outputFile(filePath){
		createFile();
		pthread_mutex_init(&logsLock, NULL);
		sem_init(&spotsLeft, 0, MAX_QUEUE_SIZE);
		sem_init(&spotsFilled, 0, 0);

		// Create a consumer thread for writing the logs into the file
		std::thread consumerThread(&PLReceiver::flush, this);
		consumerThread.detach();
	}

	~PLReceiver(){
		sem_destroy(&spotsLeft);
		sem_destroy(&spotsFilled);
		pthread_mutex_destroy(&logsLock);

	}

	int getSocket(){
		return (this->frs).getSocket();
	}

	void pp2pReceive(std::string msg, sockaddr_in clientAddress){
		// get timestamp
		// send corresponding ACK
		// if this has not yet been delivered, deliver it
		size_t firstUnderscore = msg.find('_');
		std::string idStr = msg.substr(0, firstUnderscore);
		size_t secondUnderscore = msg.find('_', firstUnderscore + 1);
		std::string tsStr = msg.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
		std::string msgWithoutId = msg.substr(secondUnderscore + 1); // there will always be something to the right of the second underscore
		pp2pSend(tsStr, clientAddress);
    unsigned long id = std::stoul(idStr);
    unsigned long ts = std::stoul(tsStr);
    if(delivered.find(id) == delivered.end() || delivered[id].find(ts) == delivered[id].end()){
			delivered[id].insert(ts);
			deliver(msgWithoutId, id);
			// deliver the message
		}
	}

	void stopAll(){
		flushing = false;
		// stop running the other thread
		emptyLogs();
	}


private:
	FLReceiverSend frs;
	std::string filePath;
	std::unordered_map<unsigned long, std::unordered_set<unsigned long>> delivered;
	Parser::Host self;
	const int MAX_QUEUE_SIZE = 200000;
	std::queue<std::pair<unsigned long, std::string> > logs;
	sem_t spotsLeft; // for the producer, writing to the logs
	sem_t spotsFilled; // for the consumer, writing from logs to text file
	pthread_mutex_t logsLock;
	std::ofstream outputFile;
	bool flushing = true;

	void createFile(){

			if (!outputFile.is_open()) {
					std::cerr << "Error creating the file: " << filePath << std::endl;
					return;
			}
	}

	void emptyLogs(){
		while(!logs.empty()){
			sem_wait(&spotsFilled);
			pthread_mutex_lock(&logsLock);
			const std::pair<unsigned long, std::string>  p = logs.front();
			unsigned long id = p.first;
			const std::string& underlying_msg = p.second;
			outputFile << "d " << std::to_string(id) << " " << underlying_msg << std::endl; // Append the underlying_msg to the file
			logs.pop(); // Remove the processed underlying_msg from the queue
			pthread_mutex_unlock(&logsLock);
			sem_post(&spotsLeft); // one more spot becomes available
		}
	}

  void flush(){
		// Append each underlying_msg from the logs to the file
		while (flushing) {
			sem_wait(&spotsFilled);
			pthread_mutex_lock(&logsLock);
			const std::pair<unsigned long, std::string>  p = logs.front();
			unsigned long id = p.first;
			const std::string& underlying_msg = p.second;
			outputFile << "d " << std::to_string(id) << " " << underlying_msg << std::endl; // Append the underlying_msg to the file
			logs.pop(); // Remove the processed underlying_msg from the queue
			pthread_mutex_unlock(&logsLock);
			sem_post(&spotsLeft); // one more spot becomes available
		}
  }

	void pp2pSend(std::string ts_str, sockaddr_in clientAddress){
		// send an ack, ie, just a timestamp
		(this->frs).fp2pSend(ts_str, clientAddress);
	}

	void deliver(std::string msg, unsigned long id){
		// 1) Spawn a thread to deal with it (optimisation)
		// 2) Read the "_" separated messages (into strings) and log each of these messages into a queue

		// RECEIVER CODE GOES HERE
		size_t curpos = 0;
		size_t found = msg.find('_');
		while(found != std::string::npos){
			std::string underlying_msg = msg.substr(curpos, found - curpos);
			sem_wait(&spotsLeft);
			pthread_mutex_lock(&logsLock);
			logs.push(make_pair(id, underlying_msg));
			pthread_mutex_unlock(&logsLock);
			sem_post(&spotsFilled);
			curpos = found + 1;
			found = msg.find('_', curpos);
		}

		// Perfect Links
		// receive packet, resend an ACK

		// if packet received first time, go through the contents and log them, update the set of messages received, resend ACK
		// if packet already received, discard and resend ACK
	}

	void callFlush(){
	}

};
