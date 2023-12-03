#pragma once

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

#include "parser.hpp"
#include "PLReceive.hpp"
#include "PLBroadcast.hpp"
#include "Stubborn.hpp"
#include "FLSend.hpp"
#include "Logger.hpp"


class FLReceive{

public:
	FLReceive(FLSend &fls, Stubborn &s, PLBroadcast &plb, FUBroadcast &fub, int sock_, unsigned long curId, std::vector<Parser::Host> hosts, Logger &lg) : plr(fls, s, plb, fub, hosts.size(), curId, lg), sock(sock_){

		unsigned short port;
		std::string ip;

		for(auto host: hosts){
			if(host.id == curId){
				port = host.portReadable();
				ip = host.ipReadable();
				break;
			}
		}

		sockaddr_in serverAddress;
		memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());

		if (bind(sock, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) < 0) {
        perror("Bind failed");
        close(sock);
        exit(1);
    }

		std::thread receiverThread(&FLReceive::fp2pReceive, this);
		receiverThread.detach();
	}

	void stopAll(){
		(this->plr).stopAll();
		listen = false;
	}

private:
	PLReceive plr;
	int sock;
	bool listen = true;

	void fp2pReceive(){
		char buffer[1024];

		while(listen){
			ssize_t readLen = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
			if(readLen == -1){
				perror("Could not read the contents of the datagram(ACK) sent by the receiver.\n");
				break;
			}
			assert(readLen < 1024);
			buffer[readLen] = '\0';

			std::string recvMsg(buffer);
			(this->plr).pp2pReceive(recvMsg);
		}
		if (close(sock) == 0) {
				std::cout << "Socket closed successfully." << std::endl;
		}
		else {
				std::cerr << "Failed to close the socket." << std::endl;
		}
	}

};
