#pragma once

#include "HttpRequest.hpp"

class HttpResponse
{
	std::string RawResponse;
public:
	HttpResponse();
	void ERRORresp();
	~HttpResponse();
};

HttpResponse::HttpResponse()
{
}

HttpResponse::~HttpResponse()
{
}
