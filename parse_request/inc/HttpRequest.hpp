#pragma once

#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>

class HttpRequest
{
		bool isprintSTR(std::string);
		std::string requestLine;
		bool parse_requestLine();
		std::string header;
		bool parse_headers();
		void extract_query();
		bool valid_value(std::string);
		int errorCode;
	public:
		HttpRequest();
		enum {INCOMPLETE, DONE, ERROR} rtype;
		std::string method;
		std::string request_target;
		std::string query;
		std::string httpVersion;
		std::map<std::string, std::string> headers;
		std::string body;
		bool parse_request(std::string);
		~HttpRequest();
};
