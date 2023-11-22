#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <pthread.h>
#include <semaphore.h>

#include "parser.hpp"
#include "Helper.hpp"

class Logger(){

public:

	Logger(const char *oPath): filePath(oPath), outputFile(filePath){
		createFile();
		pthread_mutex_init(&logsLock, NULL);
		sem_init(&spotsLeft, 0, MAX_QUEUE_SIZE);
		sem_init(&spotsFilled, 0, 0);

		std::thread consumerThread(&Logger::flush, this);
		consumerThread.detach();
	}

	~Logger(){
		sem_destroy(&spotsLeft);
		sem_destroy(&spotsFilled);
		pthread_mutex_destroy(&logsLock);
	}

	void log(string msg, bool delivery, unsigned long senderId){
		sem_wait(&spotsLeft);
		pthread_mutex_lock(&logsLock);
		if(delivery)
			logs.push("d " + std::to_string(senderId) + " " + msg);
		else
			logs.push("b " + msg);
		pthread_mutex_unlock(&logsLock);
		sem_post(&spotsFilled);
	}

	void stopAll(){
		flushing = false;
		emptyLogs();
	}


private:
	const int MAX_QUEUE_SIZE = 100000;
	std::queue<std::string> logs;
	pthread_mutex_t logsLock;
	sem_t spotsLeft, spotsFilled;
	std::ofstream outputFile;
	bool flushing = true;

	void createFile(){
		if(!outputFile.is_open()){
			std::cerr << "Error creating the file: " << filePath << std::endl;
			return;
		}
	}

	void flush(){
		while(flushing){
			sem_wait(&spotsFilled);
			pthread_mutex_lock(&logsLock);
			std::string msg = logs.front();
			outputFile<<msg<<std::endl;
			logs.pop();
			pthread_mutex_unlock(&logsLock);
			sem_post(&spotsLeft);
		}
	}

	// Make this the last operation
	void emptyLogs(){
		while(!logs.empty()){
			sem_wait(&spotsFilled);
			pthread_mutex_lock(&logsLock);
			std::string msg = logs.front();
			outputFile<<msg<<std::endl;
			logs.pop();
			pthread_mutex_unlock(&logsLock);
			sem_post(&spotsLeft);
		}
	}

};
