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
#include "../../build_response/HttpResponse.hpp"
#include "../../route/RouteResp.hpp"
#define MAX_URI_LENGTH 8192

class HttpRequest
{
		typedef std::map<std::string, std::string> headerMap;

		std::string requestLine;
		std::string header;
		RouteResp	route;
		
		enum {CHUNKED, NORMAL, NONE} bodyType;
		enum {INHEADER, INBODY} parseState;
		enum {INSIZE, IN_CHUNK, THE_END} bodyState;
		size_t body_size;
		size_t current_pos;

		bool isprintSTR(std::string);
		bool parse_requestLine();
		bool parse_headers();
		bool get_key(std::string&, const std::string&, size_t);
		bool get_value(std::string&, const std::string&, size_t);
		void extract_query();
		bool invalid_value(std::string);
		bool get_size(size_t&, std::string, size_t, size_t);
		bool parse_body(size_t, std::string);
		bool handle_chunked(std::string);
		bool store_header(const std::string&, const std::string&, headerMap&);
	public:
		ReqContent req;
		HttpRequest();
		int errorCode;
		enum {INCOMPLETE, DONE, ERROR, KEEP_ALIVE} rtype;
		std::string parse_request(std::string, serverConf *);

		~HttpRequest();
};