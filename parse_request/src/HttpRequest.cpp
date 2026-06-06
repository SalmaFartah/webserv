#include "../inc/HttpRequest.hpp"

bool HttpRequest::parse_requestLine()
{
	// request-line   = method SP request-target SP HTTP-version CRLF
	std::string buff;
	std::stringstream line(requestLine);

	//***************** SPLIT THE LINE BY SPACE ****************//
	std::vector<std::string> segments;
	while (std::getline(line, buff, ' '))
	{
		segments.push_back(buff);
	}

	//* CHECK IF EXACTLY THREE SEGMENTS: METHOD, TARGET, VERSION *//
	if (segments.size() != 3)
		return false;

	//********** EXTRACT METHOD, TARGETA AND VERSION ************//
	method = segments[0];
	request_target = segments[1];
	httpVersion = segments[2];

	//******** CHECK IF METHOD, TARGET AND VERSION ARE VALID *****//
	if ((method == "GET" || method == "POST" || method == "DELETE") \
	&& request_target.find("/") == 0 && httpVersion == "HTTP/1.1")
	{
		extract_query();
		return true;
	}

	method.clear();
	request_target.clear();
	httpVersion.clear();
	return false;
}


void HttpRequest::extract_query()
{
	size_t pos = request_target.find("?");
	if (pos != std::string::npos)
	{
		query = request_target.substr(pos + 1, request_target.size());
		request_target = request_target.substr(0, pos);
	}
}

bool HttpRequest::isprintSTR(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isprint((unsigned char)str[i]))
			return false;
	}
	return true;
}

bool HttpRequest::valid_value(std::string value)
{
	errno = 0;
	char *end = NULL;
	size_t holder = strtoull(value.c_str(), &end, 10);
	if (value[0] == '-' || value[0] == '+' || errno == ERANGE || *end)
		return false;
	return true;
}

bool HttpRequest::parse_headers()
{
	size_t startLine = 0;
	size_t eofLine;
	size_t colon;
	std::string line;
	int i = 0;
	while (startLine < header.size())
	{
		eofLine = header.find("\r\n", startLine);
		line = header.substr(startLine, eofLine - startLine);
		colon = line.find(":");
		// example "Host" or ": localhost"
		if (colon == std::string::npos || !colon)
			return false;
		std::string key = line.substr(0, colon);
		// transform the key string to lower case
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		// key should not contain space or a nonprintable character
		if (key.find_first_of(" ") != std::string::npos || !isprintSTR(key))
			return false;
		std::string value = line.substr(colon + 1, line.size());
		if ((!isprintSTR(value) && value.find("\t") == std::string::npos))
			return false;
		// trim spaces from start and end of value
		value = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);
	

		if ((key == "host" && (headers.count("host") || value.empty()))
		|| (key == "content-length" && (headers.count("content-length") || !valid_value(value)))
		|| (key == "transfer-encoding" && (value != "chunked" || headers.count("transfer-encoding"))))
		{
			errorCode = 400;
			return false;
		}
		std::cout << "key: [" << key << "]\n";
		std::cout << "value: [" << value << "]\n";
		if (key == "content-length")
			bodyType = NORMAL;
		if (key == "transfer-encoding")
			bodyType = CHUNKED;
		if (key == "connection" && value == "close")
			keepAlive = false;		
		headers[key] = value;
		startLine = eofLine + 2;
	}
	if (!headers.count("host") \
	|| (headers.count("content-length") && headers.count("transfer-encoding")) \
	|| (method == "POST" && !headers.count("content-length") && !headers.count("transfer-encoding")))
	{
		errorCode = 400;
		return false;
	}
	return true;
}

bool HttpRequest::parse_body(size_t pos, std::string request)
{
	if (bodyType == NORMAL)
	{
		char *end = NULL;
		size_t nbytes = strtoull(headers["content-length"].c_str(), &end, 10);
		body = request.substr(pos);
		if (body.size() < nbytes)
		{
			rtype = INCOMPLETE;
			return false;
		}
		else if (body.size() > nbytes && keepAlive)
		{
			body = body.substr(0, nbytes);
			rtype = ANOTHER;
		}
	}
	return true;
}

void HttpRequest::parse_request(std::string request)
{
	size_t HeaderEnd;
	size_t HeaderBegin;
	// hna ghan9aleb 3la \r\n ida ma l9ithash ghanreturni 1 u ghayzid l core i9ra data 
	// u isifthali u ghayb9a haka tanl9a \r\n
	if (parseState == INHEADER)
	{
		HeaderEnd = request.find("\r\n\r\n");
		parseState = INHEADER;
		if (HeaderEnd == std::string::npos) // mazal khasni data mn core
		{
			rtype = INCOMPLETE;
			return ;
		}
		HeaderBegin = request.find("\r\n");
		requestLine = request.substr(0, HeaderBegin);
		if (!parse_requestLine())
		{
			std::cerr << "error: malformed request line.\n";
			rtype = ERROR;
			return ;
			// should return a responce with error page
		}
		HeaderBegin += 2;
		header = request.substr(HeaderBegin, HeaderEnd - HeaderBegin + 2);
		if (!parse_headers())
		{
			std::cerr << "Error: headers\n";
			rtype = ERROR;
			return ;
		}
		parseState = INBODY;
	}
	rtype = DONE;
	if (!parse_body(HeaderEnd + 4, request))
		return ;
	
}

HttpRequest::HttpRequest()
{
	parseState = INHEADER;
	keepAlive = true;
}
HttpRequest::~HttpRequest(){}