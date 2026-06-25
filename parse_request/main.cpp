#include "inc/HttpRequest.hpp"

int main()
{
	serverConf conf;
	conf.body_size = 45;
	HttpRequest req;
	// req.parse_request("POST / HTTP/1.1\r\nHost: localhost\r\ntransfer-encoding: chunked\r\n\r\n5\r\nbye!!\r\n2\r\n m\r\n8\r\ny friend\r\n0\r\n\r\n", &conf);
	req.parse_request("POST /test/ HTTP/1.1\r\nHost: localhost\r\ncontent-length: 5\r\n\r\nbye!!", &conf);
	// if (req.rtype == req.ERROR)
	// 	std::cerr << "ERROR\n";
	// if (req.rtype == req.INCOMPLETE)
	// 	std::cerr << "INCOMPLETE\n";
	// if (req.rtype == req.KEEP_ALIVE)
	// 	std::cerr << "KEEP_ALIVE\n";
	// req.parse_request("POST / HTTP/1.1\r\nHost: localhost\r\ncontent-length: 5\r\n\r\nbye!!", &conf);
	// if (req.rtype == req.ERROR)
	// 	std::cerr << "ERROR\n";
	// if (req.rtype == req.INCOMPLETE)
	// 	std::cerr << "INCOMPLETE\n";
	// if (req.rtype == req.KEEP_ALIVE)
	// 	std::cerr << "KEEP_ALIVE\n";
}


// 5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n
