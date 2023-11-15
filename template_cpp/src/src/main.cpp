#include <chrono>
#include <iostream>
#include <thread>
#include <fstream>

#include "parser.hpp"
#include "hello.h"
#include <signal.h>

#include "HandlerSender1.hpp"
#include "HandlerReceiver1.hpp"

HandlerSender1 *sendPtr = nullptr;
HandlerReceiver1 *receivePtr = nullptr;

static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";
	if(receivePtr	!= nullptr){
		receivePtr -> stopExchange();
	}
	if(sendPtr != nullptr)
		sendPtr -> stopExchange();

  // write/flush output file if necessary
  std::cout << "Writing output.\n";

  // exit directly from signal handler
  exit(0);
}

int main(int argc, char **argv) {

	signal(SIGTERM, stop);
	signal(SIGINT, stop);

  // `true` means that a config file is required.
  // Call with `false` if no config file is necessary.
  bool requireConfig = true;

  Parser parser(argc, argv);
  parser.parse();

  hello();
  std::cout << std::endl;

  std::cout << "My PID: " << getpid() << "\n";
  std::cout << "From a new terminal type `kill -SIGINT " << getpid() << "` or `kill -SIGTERM "
            << getpid() << "` to stop processing packets\n\n";

  std::cout << "My ID: " << parser.id() << "\n\n";

  std::cout << "List of resolved hosts is:\n";
  std::cout << "==========================\n";
  auto hosts = parser.hosts();
  for (auto &host : hosts) {
    std::cout << host.id << "\n";
    std::cout << "Human-readable IP: " << host.ipReadable() << "\n";
    std::cout << "Machine-readable IP: " << host.ip << "\n";
    std::cout << "Human-readbale Port: " << host.portReadable() << "\n";
    std::cout << "Machine-readbale Port: " << host.port << "\n";
    std::cout << "\n";
  }
  std::cout << "\n";

  std::cout << "Path to output:\n";
  std::cout << "===============\n";
  std::cout << parser.outputPath() << "\n\n";

  std::cout << "Path to config:\n";
  std::cout << "===============\n";
  std::cout << parser.configPath() << "\n\n";

	// Finding out the parameters
	unsigned long target, num_messages;
	Helper::removeFile(parser.outputPath());
	if(Helper::readParams(parser.configPath(), num_messages, target) == false)
		std::cerr<<"Failed to read parameters from the config file "<<std::endl;

	Parser::Host targetDetails = Helper::getReceiverInfo(hosts, target);
	
  std::cout << "Doing some initialization...\n\n";

	unsigned long curId = parser.id();
	if(curId == target){
		HandlerReceiver1 h(parser.outputPath(), targetDetails.ipReadable().c_str(), targetDetails.portReadable());
		receivePtr = &h;
		while (true) {
			std::this_thread::sleep_for(std::chrono::hours(1));
		}
	}
	else{
		Parser::Host curDetails = Helper::getReceiverInfo(hosts, curId);
		HandlerSender1 h(curId, parser.outputPath(), num_messages, target, targetDetails.ipReadable().c_str(), targetDetails.portReadable(), curDetails.ipReadable().c_str(), curDetails.portReadable());
		// std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Broadcasting and delivering messages...\n\n";
		sendPtr = &h;
		h.startExchange();
		while (true) {
			std::this_thread::sleep_for(std::chrono::hours(1));
		}
	}


  // After a process finishes broadcasting,
  // it waits forever for the delivery of messages.

  return 0;
}
