#pragma once

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <cassert>

#include "parser.hpp"
#include "Helper.hpp"
#include "PLSenderReceive.hpp"
#include "Stubborn.hpp"


class FLSenderReceive{

public:
	FLSenderReceive(Stubborn *s, int sock_, const char *ip_self, unsigned short port_self) : sockNum(sock_), psr(s){

		// bind listener
		sockaddr_in myAddress;
		memset(&myAddress, 0, sizeof(myAddress));
		myAddress.sin_family = AF_INET;
		myAddress.sin_port = htons(port_self);
		myAddress.sin_addr.s_addr = inet_addr(ip_self);

    if (bind(sockNum, reinterpret_cast<struct sockaddr *>(&myAddress), sizeof(myAddress)) < 0) {
        perror("Bind failed");
        close(sockNum);
        exit(1);
    }


		// activate listening
		std::thread receiverThread(&FLSenderReceive::fp2pReceive, this);
		receiverThread.detach();
		// main function continues working as before

		// std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	void stopAll(){
		listen = false;
	}

private:
	const int sockNum;
	PLSenderReceive psr;
	bool listen = true;

	void fp2pReceive(){
		char buffer[1024];
		// std::this_thread::sleep_for(std::chrono::seconds(1));
		while(listen){
			ssize_t readLen = recvfrom(sockNum, buffer, sizeof(buffer), 0, NULL, NULL);
			if(readLen == -1){
				perror("Could not read the contents of the datagram(ACK) sent by the receiver.\n");
			}
			assert(readLen < 1024);
			buffer[readLen] = '\0';

			std::string recvMsg(buffer);
			(this->psr).pp2pReceive(recvMsg);
		}
	}

};
