#pragma once

#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include "../parse_config/inc/FillServer.hpp"
#include "../parse_config/inc/FillLocation.hpp"

class HttpRequest
{
		bool isprintSTR(std::string);
		std::string requestLine;
		bool parse_requestLine();
		std::string header;
		bool parse_headers();
		void extract_query();
		bool invalid_value(std::string);
		bool parse_body(size_t, std::string);
		int errorCode;
		enum {CHUNKED, NORMAL} bodyType;
		enum {INHEADER, INBODY} parseState;

	public:
		HttpRequest();
		enum {INCOMPLETE, DONE, ERROR, ANOTHER} rtype;
		bool keepAlive;
		std::string method;
		std::string request_target;
		std::string query;
		std::string httpVersion;
		std::map<std::string, std::string> headers;
		std::string body;
		void parse_request(std::string, serverConf *);
		~HttpRequest();
};
