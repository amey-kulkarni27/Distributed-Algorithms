#pragma once 
 
#include <iostream> 
#include <string>
#include <map>
#include <queue>
#include <memory>
#include <thread>
#include <pthread.h>
#include <semaphore.h>

#include "parser.hpp" 
#include "Helper.hpp"

// Sender side
#include "FUBroadcast.hpp" 
#include "FLReceive.hpp" 


class Handler{

public:

	// Constructor named initialise, because we wanted to create a global object
	Handler(unsigned long id, const char *outputPath, unsigned long num_messages_, std::vector<Parser::Host> hosts) : fub(id, hosts), flr(&((this->fub).s), (this->fub).getSocket(), curId, hosts), filePath(outputPath), outputFile(filePath){
		num_messages = num_messages_;

		createFile();
		pthread_mutex_init(&logsLock, NULL);
    sem_init(&spotsLeft, 0, MAX_QUEUE_SIZE);
    sem_init(&spotsFilled, 0, 0);

		// Create a consumer thread for writing the logs into the file
    std::thread consumerThread(&Handler::flush, this);
    consumerThread.detach();
	}

	~Handler(){
		 sem_destroy(&spotsLeft);
		 sem_destroy(&spotsFilled);
		 pthread_mutex_destroy(&logsLock);
	}


	void startExchange(){
		broadcast();
	}


	void stopExchange(){
		// stop broadcasting
		(this->fub).stopAll();

		// stop receiving
		(this->flr).stopAll();
		
		// stop writing
		flushing = false;
		emptyLogs();
	}

private:
	FUBroadcast fub;
	FLReceive flr;
	std::string filePath;
	unsigned long num_messages;
	const int MAX_QUEUE_SIZE = 100000;
	std::queue<std::string> logs;
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
		 const std::string underlying_msg = logs.front();
		 outputFile << underlying_msg << std::endl; // Append the underlying_msg to the file
		 logs.pop(); // Remove the processed underlying_msg from the queue
		 pthread_mutex_unlock(&logsLock);
		 sem_post(&spotsLeft); // one more spot becomes available
	 }
	}


	void flush(){
	 while(flushing){
		 sem_wait(&spotsFilled);
		 pthread_mutex_lock(&logsLock);
		 const std::string underlying_msg = logs.front();
		 outputFile << underlying_msg << std::endl; // Append the underlying_msg to the file
		 logs.pop(); // Remove the processed underlying_msg from the queue
		 pthread_mutex_unlock(&logsLock);
		 sem_post(&spotsLeft); // one more spot becomes available
	 }
	}


	std::string createMsgAppendToLogs(unsigned long st, unsigned long en){
		std::string payload = "";
		while(st < en){
			std::string msg = std::to_string(st);
			sem_wait(&spotsLeft);
			pthread_mutex_lock(&logsLock);
			logs.push("b " + msg);
			pthread_mutex_unlock(&logsLock);
			sem_post(&spotsFilled);
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
