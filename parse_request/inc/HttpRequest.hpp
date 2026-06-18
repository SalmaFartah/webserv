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

#define MAX_URI_LENGTH 8192

typedef struct
{
	std::string method;
	std::string request_target;
	std::string query;
	std::string httpVersion;
	std::map<std::string, std::string> headers;
	std::string body;
} ReqContent;

class HttpRequest
{
		ReqContent req;

		std::string requestLine;
		std::string header;

		enum {CHUNKED, NORMAL, NONE} bodyType;
		enum {INHEADER, INBODY} parseState;
		enum {INSIZE, IN_CHUNK, THE_END} bodyState;
		bool keepAlive;
		size_t body_size;
		size_t current_pos;

		bool isprintSTR(std::string);
		bool parse_requestLine();
		bool parse_headers();
		void extract_query();
		bool invalid_value(std::string);
		size_t get_size(std::string, size_t, size_t);
		bool parse_body(size_t, std::string);
		bool handle_chunked(std::string);
	public:
		HttpRequest();
		int errorCode;
		enum {INCOMPLETE, DONE, ERROR, KEEP_ALIVE} rtype;
		void parse_request(std::string, serverConf *);
		
		// Getters to access parsed request data
		std::string getMethod() const;
		std::string getTarget() const;
		std::string getQuery() const;
		std::string getBody() const;
		std::string getHeader(const std::string& key) const;
		std::map<std::string, std::string> getAllHeaders() const;
		size_t getBodySize() const;

		~HttpRequest();
};