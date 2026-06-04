#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>

enum tokenType
{
	WORD,
	OPEND_BC,
	CLOSED_BC,
	SEMI_COL
};

class conf
{
	private:
		std::vector<std::pair<tokenType, std::string> > vecToken;
		// std::vector<Token> vecToken;
	public:
		conf();
		void read_file(std::ifstream& conf);
		void fillWords(std::ifstream& confWords, int c); 
		// void print_tokenz();
		~conf();
};