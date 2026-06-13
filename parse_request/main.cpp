#include "inc/HttpRequest.hpp"

int main()
{
	serverConf conf;
	conf.body_size = 45;
	HttpRequest req;
	req.parse_request("POST /upload HTTP/1.1\r\nHost: example.com\r\nTransfer-Encoding: chunked\r\nContent-Type: text/plain\r\n\r\n30\r\n\r\n", &conf);
}
