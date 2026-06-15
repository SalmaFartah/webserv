#include "inc/HttpRequest.hpp"

int main()
{
	serverConf conf;
	conf.body_size = 45;
	HttpRequest req;
	req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding:chunked", &conf);
	req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding:chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n", &conf);
	req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding:chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n1\r\n \r\n", &conf);
	req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding:chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n1\r\n \r\n0", &conf);
	req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding:chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n1\r\n \r\n0\r\n\r\nGet", &conf);
	if (req.rtype == req.ERROR)
		std::cerr << "ERROR\n";
	if (req.rtype == req.INCOMPLETE)
		std::cerr << "INCOMPLETE\n";
	if (req.rtype == req.KEEP_ALIVE)
		std::cerr << "KEEP_ALIVE\n";
	req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding:chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n1\r\n \r\n0\r\n\r\nGET /path HTTP/1.1\r\nhost:\r\nhello\r\n\r\n", &conf);
	// if (req.rtype == req.ERROR)
	// 	std::cerr << "ERROR\n";
	// if (req.rtype == req.INCOMPLETE)
	// 	std::cerr << "INCOMPLETE\n";
	// if (req.rtype == req.KEEP_ALIVE)
	// 	std::cerr << "KEEP_ALIVE\n";
	

}


// 5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n
