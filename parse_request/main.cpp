#include "inc/HttpRequest.hpp"

int main()
{
	HttpRequest req;
	req.parse_request("POST /upload HTTP/1.1\r\nHost: example.com\r\nTransfer-Encoding: chunked\r\nContent-Type: text/plain\r\n\r\n7\r\nMozilla\r\n9\r\nDeveloper\r\n7\r\nNetwork\r\n0\r\n\r\n", 0);
}
