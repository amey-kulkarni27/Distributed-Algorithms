#pragma once

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cassert>
#include <mutex>

#include "parser.hpp"
#include "Helper.hpp"


class FLSend{

public:
	FLSend(std::vector<Parser::Host> hosts_){

		for(auto host: hosts_)
			id_to_host[host.id] = host;

		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if(sock == -1){
        perror("Failed to create socket");
    }

	}

	int getSocket(){
		return sock;
	}

	int fp2pSend(unsigned long target_id, std::string msg){

		// Fill up serverAddress details using target_id and host information
		sockaddr_in serverAddress;
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(id_to_host[target_id].portReadable());
		serverAddress.sin_addr.s_addr = inet_addr(id_to_host[target_id].ipReadable().c_str());

		std::lock_guard<std::mutex> lock(socketLock);
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
	std::unordered_map<unsigned long, Parser::Host> id_to_host;
	std::mutex socketLock;

};
