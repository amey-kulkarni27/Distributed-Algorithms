#pragma once

#include <iostream>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <set>
#include <queue>
#include <map>
#include <cassert>
#include <sys/stat.h>
#include <cstdio>

#include "parser.hpp"


class Helper {

public:

	static void printThreadId(){
		std::cout<<std::this_thread::get_id()<<std::endl;
	}

	static void printText(std::string s){
		std::cout<<s<<std::endl;
	}

	static void printSet(std::set<unsigned long> s){
		if(s.empty()){
			std::cout<<"Empty Set"<<std::endl;
			return;
		}
		for(const unsigned long &x: s)
			std::cout<<x<<' ';
		std::cout<<std::endl;
	}

	static void printSet(std::unordered_set<unsigned long> s){
		if(s.empty()){
			std::cout<<"Empty Set"<<std::endl;
			return;
		}
		for(const unsigned long &x: s)
			std::cout<<x<<' ';
		std::cout<<std::endl;
	}

	static void removeFile(const char *outPath){
		if (std::remove(outPath) != 0) {
				// If the file could not be removed, an error occurred
				std::perror("Error deleting the file");
		} else {
				// File was successfully removed
				std::cout << "File deleted successfully" << std::endl;
		}
	}

	static Parser::Host getInfo(std::vector<Parser::Host> hosts, unsigned long target){
		Parser::Host dummy;
		for (auto &host : hosts)
		if(host.id == target)
			return host;
		assert(false);
		return dummy;
	}

	static unsigned long getProposals(const char *configPath){
		std::ifstream configFile(configPath);
		unsigned long num_proposals, vs, ds;
		if(configFile.is_open()){
			std::string firstLine;
			if(getline(configFile, firstLine)){
				std::istringstream iss(firstLine);
				if(iss>>num_proposals>>vs>>ds);
				else
					std::cerr<<"Failed to read in the three integers "<<std::endl;
			}
			else
				std::cerr<<"Failed to read the first line "<<std::endl;
		}
		return num_proposals;
	}

 	static void readParams (const char *configPath, unsigned long &num_proposals, unsigned long &vs, unsigned long &ds, std::vector<std::unordered_set<unsigned long>> &proposals){
		std::ifstream configFile(configPath);
		if(configFile.is_open()){
			std::string firstLine;
			if(getline(configFile, firstLine)){
				std::istringstream iss(firstLine);
				if(iss>>num_proposals>>vs>>ds);
				else
					std::cerr<<"Failed to read in the three integers "<<std::endl;
			}
			else
				std::cerr<<"Failed to read the first line "<<std::endl;
			
			// Read proposals one by one
			for(unsigned long i = 0; i < num_proposals; i++){
				std::string proposalLine;
				if(getline(configFile, proposalLine)){
					std::istringstream proposalIss(proposalLine);
					std::unordered_set<unsigned long> proposal;
					unsigned long num;
					while(proposalIss >> num)
						proposal.insert(num);
					proposals.push_back(proposal);
				}
				else
					std::cerr<<"Failed to read the proposal line "<<std::endl;
			}
		}
		else
			std::cerr<<"Failed to open config file "<<std::endl;
	}

};
