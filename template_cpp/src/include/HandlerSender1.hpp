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
#include "PLSenderSend.hpp" 
#include "FLSenderReceive.hpp" 


class HandlerSender1 {

public:
	PLSenderSend pss;

	// Constructor named initialise, because we wanted to create a global object
	HandlerSender1(unsigned long id, const char *outputPath, unsigned long num_messages_, unsigned long target_, const char *ip, unsigned short port, const char *ip_self, unsigned short port_self) : pss(id, ip, port), filePath(outputPath), outputFile(filePath){
		num_messages = num_messages_;
		target = target_;

		initReceiver(ip_self, port_self);

		createFile();
		pthread_mutex_init(&logsLock, NULL);
    sem_init(&spotsLeft, 0, MAX_QUEUE_SIZE);
    sem_init(&spotsFilled, 0, 0);

		// Create a consumer thread for writing the logs into the file
    std::thread consumerThread(&HandlerSender1::flush, this);
    consumerThread.detach();
	}

	~HandlerSender1(){
		 sem_destroy(&spotsLeft);
		 sem_destroy(&spotsFilled);
		 pthread_mutex_destroy(&logsLock);
	}


	void initReceiver(const char *ip_self, unsigned short port_self){
		this -> fsrptr= std::make_unique<FLSenderReceive>(&((this->pss).s), (this->pss).getSocket(), ip_self, port_self);
	}



	void startExchange(){
		sendMessage();
	}


	void stopExchange(){
		// stop perfect links
		(this->fsrptr) -> stopAll();
		(this->pss).stopAll();
		flushing = false;
		emptyLogs();
	}

private:
	std::unique_ptr<FLSenderReceive> fsrptr;
	std::string filePath;
	unsigned long num_messages;
	unsigned long target;
	const int MAX_QUEUE_SIZE = 100000;
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
			 outputFile << "b " << underlying_msg << std::endl; // Append the underlying_msg to the file
			 logs.pop(); // Remove the processed underlying_msg from the queue
			 pthread_mutex_unlock(&logsLock);
			 sem_post(&spotsLeft); // one more spot becomes available
		 }
 }

   void flush(){
		 while(flushing){
			 sem_wait(&spotsFilled);
			 pthread_mutex_lock(&logsLock);
			 const std::pair<unsigned long, std::string>  p = logs.front();
			 unsigned long id = p.first;
			 const std::string& underlying_msg = p.second;
			 outputFile << "b " << underlying_msg << std::endl; // Append the underlying_msg to the file
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
			logs.push(make_pair(1, msg));
			pthread_mutex_unlock(&logsLock);
			sem_post(&spotsFilled);
			payload += msg + "_";
			st++;
		}
		return payload;
	}

	void sendMessage(){
		// 1) Create packets containing 8 messages
		// 2) Log them and send them through the perfect links abstraction
		unsigned long i = 1;
		while(i <= num_messages){
			unsigned long end = std::min(i + 8, num_messages + 1);
			std::string msgToSend = createMsgAppendToLogs(i, end);
			(this->pss).pp2pSend(msgToSend);
			i = end;
		}
		
		// std::this_thread::sleep_for(std::chrono::seconds(1));

		// Perfect Links
		// A possible optimisation is:
		// A producer creating the packets
		// A consumer reading the packets and sending them to the Perfect Links

		// send the first packet till you receive an ACK

		// create a packet of 8 messages
		// log it
		// wait for an ACK
	}


};
