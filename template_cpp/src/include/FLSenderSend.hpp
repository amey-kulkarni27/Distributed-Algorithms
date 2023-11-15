#pragma once

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cassert>

#include "parser.hpp"
#include "Helper.hpp"


class FLSenderSend{

public:
	FLSenderSend(const char *ip, unsigned short port){
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if(sock == -1){
        perror("Failed to create socket");
    }

		serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(ip);

	}

	int getSocket(){
		return sock;
	}

	int fp2pSend(std::string msg){
		if(sendto(sock, msg.c_str(), msg.length(), 0, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1){
			perror("Error while sending the message.\n");
			return -1;
		}
		else
			return 0;
	}

	void stopAll(){
		if (close(sock) == 0) {
        std::cout << "Socket closed successfully." << std::endl;
    }
	 	else {
        std::cerr << "Failed to close the socket." << std::endl;
    }
	}

private:
	int sock;
	sockaddr_in serverAddress;

};
