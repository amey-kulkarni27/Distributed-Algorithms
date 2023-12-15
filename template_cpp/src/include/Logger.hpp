#pragma once

#include <iostream>
#include <string>
#include <unordered_set>

#include "parser.hpp"
#include "Helper.hpp"

class Logger{

public:

	Logger(const char *oPath, unsigned long n_): filePath(oPath), outputFile(filePath), n(n_){
		createFile();
		logs.resize(n + 1);
		has.resize(n + 1, false);
		ptr = 1;
	}


	void logAndFlush(unsigned long pos, std::unordered_set<unsigned long> nums){
		logs[pos] = nums;
		while(ptr <= n and has[ptr]){
			writeNewLine(nums);
			logs[ptr].clear();
			ptr++;
		}
	}



private:
	std::vector<std::unordered_set<unsigned long>> logs;
	std::vector<bool> has;
	int ptr;
	std::string filePath;
	std::ofstream outputFile;
	unsigned long n;
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
