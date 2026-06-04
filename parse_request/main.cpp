#include "inc/HttpRequest.hpp"

int main()
{
	HttpRequest req;
	req.parse_request("GET /search HTTP/1.1\r\nHost:h\r\ncontent-length: +12\r\nconnection: keep-alive\r\n\r\n");
}
