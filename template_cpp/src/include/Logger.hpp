#pragma once

#include <iostream>
#include <string>
#include <unordered_set>

#include "parser.hpp"
#include "Helper.hpp"

class Logger{

public:

	Logger(const char *oPath, const char *cPath): filePath(oPath), outputFile(filePath){
		num_proposals = Helper::getProposals(cPath);
		logs.resize(num_proposals);
		has.resize(num_proposals, false);
		ptr = 0;
		createFile();
	}


	void logAndFlush(unsigned long pos, std::unordered_set<unsigned long> nums){
		std::cout<<"Log"<<std::endl;
		logs[pos] = nums;
		has[pos] = true;
		while(ptr < num_proposals and has[ptr]){
			writeNewLine(logs[ptr]);
			logs[ptr].clear();
			ptr++;
		}
	}



private:
	std::vector<std::unordered_set<unsigned long>> logs;
	std::vector<bool> has;
	unsigned long ptr;
	std::string filePath;
	std::ofstream outputFile;
	unsigned long num_proposals;
	bool flushing = true;

	void createFile(){
		if(!outputFile.is_open()){
			std::cerr << "Error creating the file: " << filePath << std::endl;
			return;
		}
	}

	void writeNewLine(std::unordered_set<unsigned long> nums){
		for(const unsigned long &num: nums)
			outputFile<<num<<" ";
		outputFile<<std::endl;
	}

};
