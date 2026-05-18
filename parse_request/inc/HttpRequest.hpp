#pragma once

#define ERROR 0
#define CHUNCKED 1
#define NORMAL 2
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <vector>

class HttpRequest
{
		std::string requestLine;
		bool parse_requestLine();
		std::string header;
		bool parse_headers();
		// bool get_key(std::string&, std::string, size_t);
		void extract_query();
	public:
		HttpRequest();
		int rtype;
		std::string method;
		std::string request_target;
		std::string query;
		std::string httpVersion;
		std::map<std::string, std::string> headers;
		std::string body;
		void ParseUpHeader(std::string);
		~HttpRequest();
};
