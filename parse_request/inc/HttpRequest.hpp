#pragma once

#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include "../../parse_config/inc/FillServer.hpp"
#include "../../parse_config/inc/FillLocation.hpp"

class HttpRequest
{
		bool isprintSTR(std::string);
		std::string requestLine;
		bool parse_requestLine();
		std::string header;
		bool parse_headers();
		void extract_query();
		bool invalid_value(std::string);
		bool parse_body(size_t, std::string, serverConf *);
		int errorCode;
		enum {CHUNKED, NORMAL} bodyType;
		enum {INHEADER, INBODY} parseState;
		bool keepAlive;
		std::string method;
		std::string request_target;
		std::string query;
		std::string httpVersion;
		std::map<std::string, std::string> headers;
		std::string body;
		size_t body_size;
	public:
		HttpRequest();
		enum {INCOMPLETE, DONE, ERROR, ANOTHER} rtype;
		void parse_request(std::string, serverConf *);
		~HttpRequest();
};
