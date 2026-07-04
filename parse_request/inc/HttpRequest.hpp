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

		enum {CHUNKED, NORMAL, NONE} bodyType;
		enum {INHEADER, INBODY} parseState;
		enum {INSIZE, IN_CHUNK, THE_END} bodyState;

		std::string requestLine, header, boundary, filename;
		size_t content_length, current_pos;
		HttpResponse resp;
		locationConf empty;

		bool isprintSTR(std::string);
		void trim_WS(std::string& str);

		bool parse_requestLine();
		void extract_query();

		bool parse_headers();
		bool get_key(std::string&, const std::string&, size_t);
		bool get_value(std::string&, const std::string&, size_t);
		bool store_header(const std::string&, const std::string&, headerMap&);
		bool invalid_value(std::string);

		bool parse_body(size_t, std::string);
		bool get_size(size_t&, std::string, size_t, size_t);
		bool handle_chunked(std::string);
		bool part_headers(std::string);
		bool handle_multipart();
		bool get_param(std::string&, std::string&, std::string, bool&);
	public:
		CGIResult CGIobj;
		HttpRequest();
		RouteResp	route;
		ReqContent req;
		int errorCode;
		enum {INCOMPLETE, DONE, ERROR, KEEP_ALIVE} rtype;
		std::string parse_request(std::string, serverConf *);

		~HttpRequest();
};