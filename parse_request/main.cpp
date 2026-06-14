#include "inc/HttpRequest.hpp"

int main()
{
	serverConf conf;
	conf.body_size = 45;
	HttpRequest req;
	req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked", &conf);
	req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n", &conf);
	req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n1\r\n \r\n", &conf);
	req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n1\r\n \r\n0", &conf);
	req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n1\r\n \r\n0\r\n\r\nGET", &conf);
	if (req.rtype == 3)
		req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n1\r\n \r\n0\r\n\r\nGET", &conf);
		req.parse_request("POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n1\r\n \r\n0\r\n\r\nGET /index.html HTTP/1.1\r\nsomth\r\n\r\n", &conf);

}


// 5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n
