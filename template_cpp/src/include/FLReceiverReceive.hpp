#pragma once

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

#include <parser.hpp>
#include <PLReceiver.hpp>


class FLReceiverReceive{

public:
	FLReceiverReceive(const char *oPath, const char *ip, unsigned short port) : pr(oPath){
		sock = (this->pr).getSocket();

		sockaddr_in serverAddress;
		memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(ip);

		if (bind(sock, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) < 0) {
        perror("Bind failed");
        close(sock);
        exit(1);
    }

		std::thread receiverThread(&FLReceiverReceive::fp2pReceive, this);
		receiverThread.detach();
	}

	void stopAll(){
		(this->pr).stopAll();
		listen = false;
	}

private:
	PLReceiver pr;
	bool listen = true;
	int sock;

	void fp2pReceive(){
		char buffer[1024];

		sockaddr_in clientAddress;
		socklen_t cAddrLen = sizeof(clientAddress);

		while(listen){
			ssize_t readLen = recvfrom(sock, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr *>(&clientAddress), &cAddrLen);
			if(readLen == -1){
				perror("Could not read the contents of the datagram(ACK) sent by the receiver.\n");
				break;
			}
			assert(readLen < 1024);
			buffer[readLen] = '\0';

			std::string recvMsg(buffer);
			(this->pr).pp2pReceive(recvMsg, clientAddress);
		}
		if (close(sock) == 0) {
				std::cout << "Socket closed successfully." << std::endl;
		}
		else {
				std::cerr << "Failed to close the socket." << std::endl;
		}
	}

};
