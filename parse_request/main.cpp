#include "inc/HttpRequest.hpp"

int main()
{
	HttpRequest req;
	req.ParseUpHeader("GET /search HTTP/1.1\r\nHost:        localhost    \r\ncontent-length:       	12       \r\nconnection: keep-alive\r\n");
}
