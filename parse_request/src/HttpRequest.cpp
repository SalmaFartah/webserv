#include "../inc/HttpRequest.hpp"

bool HttpRequest::parse_requestLine()
{
	// request-line   = method SP request-target SP HTTP-version CRLF (carriage return line feed \r\n)
	std::string buff;
	std::stringstream line(requestLine);

	//***************** SPLIT THE LINE BY SPACE ****************//
	std::vector<std::string> segments;
	while (std::getline(line, buff, ' '))
		segments.push_back(buff);

	//* CHECK IF EXACTLY THREE SEGMENTS: METHOD, TARGET, VERSION *//
	if (segments.size() != 3)
		return rtype = ERROR, errorCode = 400, false;

	//********** EXTRACT METHOD, TARGETA AND VERSION ************//
	req.method = segments[0];
	req.request_target = segments[1];
	req.httpVersion = segments[2];

	//******** CHECK IF METHOD, TARGET AND VERSION ARE VALID *****//
	if ((req.method == "GET" || req.method == "POST" || req.method == "DELETE") \
	&& req.request_target.find("/") == 0 && req.httpVersion == "HTTP/1.1"
	&& req.request_target.size() <= MAX_URI_LENGTH)
		return extract_query(), true;
	errorCode = 400;
	return rtype = ERROR, false;
}


void HttpRequest::extract_query()
{
	size_t pos = req.request_target.find("?");
	if (pos != std::string::npos)
	{
		req.query = req.request_target.substr(pos + 1, req.request_target.size());
		req.request_target = req.request_target.substr(0, pos);
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

bool HttpRequest::invalid_value(std::string value)
{
	errno = 0;
	char *end = NULL;
	strtoul(value.c_str(), &end, 10);
	if (value[0] == '-' || value[0] == '+' || errno == ERANGE || *end)
		return true;
	return false;
}

bool HttpRequest::parse_headers()
{
	size_t startLine = 0;
	size_t eofLine;
	size_t colon;
	std::string line;
	while (startLine < header.size())
	{
		eofLine = header.find("\r\n", startLine);
		line = header.substr(startLine, eofLine - startLine);
		colon = line.find(":");
		// example "Host" or ": localhost"
		if (colon == std::string::npos || !colon)
			return false;
		std::string key = line.substr(0, colon);
		// transform the all characters in key string to lower case
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		// key should not contain space or a nonprintable character
		if (key.find_first_of(" ") != std::string::npos || !isprintSTR(key))
			return false;
		std::string value = line.substr(colon + 1, line.size());
		if ((!isprintSTR(value) && value.find("\t") == std::string::npos))
			return false;
		// trim spaces from start and end of value
		value = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);

		if ((key == "host" && (req.headers.count("host") || value.empty()))
		|| (key == "content-length" && (req.headers.count("content-length") || invalid_value(value)))
		|| (key == "transfer-encoding" && (value != "chunked" || req.headers.count("transfer-encoding")))
		|| (key == "content-Type" && req.headers.count("content-Type")))
			return false;
		if (key == "content-length")
			bodyType = NORMAL;
		if (key == "transfer-encoding")
			bodyType = CHUNKED;
		if (key == "connection" && value == "close")
			keepAlive = false;
		req.headers[key] = value;
		startLine = eofLine + 2;
	}
	if (!req.headers.count("host") \
	|| (req.headers.count("content-length") && req.headers.count("transfer-encoding")))
		return false;
	return true;
}

size_t HttpRequest::get_size(std::string bodyreq, size_t start, size_t end)
{
	std::string sizeSTR = bodyreq.substr(start, end - start);
	if (isspace(sizeSTR[0]) || sizeSTR[0] == '+' || sizeSTR[0] == '-')
		return rtype = ERROR, errorCode = 400, -1;
	size_t size;
	char *check = NULL;
	size = strtoul(sizeSTR.c_str(), &check, 16);
	if ((!size && sizeSTR.size() != 1) || (size && sizeSTR[0] == '0') || errno == ERANGE || *check)
		return errorCode = 400, rtype = ERROR, -1;
	if (!size)
		bodyState = THE_END;
	return size;
}

bool HttpRequest::parse_body(size_t bodystrat, std::string request, serverConf *conf)
{
	(void)conf;
	if (bodyType == NORMAL)
	{
		char *end = NULL;
		body_size = strtoul(req.headers["content-length"].c_str(), &end, 10);
		req.body = request.substr(bodystrat);
		if (req.body.size() < body_size)
			return rtype = INCOMPLETE, false;
		if (!keepAlive)
			rtype = DONE;
		req.body = req.body.substr(0, body_size);
		parseState = INHEADER;
		bodyType = NONE;
		current_pos += bodystrat + body_size;
	}
	else if (bodyType == CHUNKED)
	{
		std::string bodyreq = request.substr(bodystrat);
		static size_t pos0;
		size_t pos1 = 0;
		size_t pos2;
		static size_t size = 0;
		std::string chunk;
		while (1)
		{
			pos1 = bodyreq.find("\r\n", pos0);
			if (bodyState == INSIZE)
			{
				if (pos1 == std::string::npos)
					return rtype = INCOMPLETE, false;
				bodyState = IN_CHUNK;
				size = get_size(bodyreq, pos0, pos1);
				std::cout << "[" << size << "]" << std::endl;
				if (size == 0)
					return false;
			}
			if (bodyState == IN_CHUNK)
			{
				pos1 += 2;
				pos2 = bodyreq.find("\r\n", pos1);
				if (pos2 == std::string::npos)
					return rtype = INCOMPLETE, false;
				chunk = bodyreq.substr(pos1, pos2 - pos1);
				if (chunk.size() != size)
					return errorCode = 400, rtype = ERROR, false;
				req.body += chunk;
				pos1 = pos2 + 2;
				bodyState = INSIZE;
				std::cout << "chunk: [" << chunk << "]\n";
			}
			if (bodyState == THE_END)
			{
				std::string str = bodyreq.substr(pos1);
				if (str.size() < 4)
					return rtype = INCOMPLETE, false;
				if (str.size() >= 4 && str.find("\r\n\r\n"))
					return errorCode = 400, rtype = ERROR, false;
				if (!keepAlive)
					rtype = DONE;
				current_pos += bodystrat + pos1 + 4;
				parseState = INHEADER;
				bodyState = INSIZE;
				bodyType = NONE;
				return true;
			}
			pos0 = pos1;
		}
	}	
	return true;
}

void HttpRequest::parse_request(std::string request, serverConf *conf)
{

	request = request.substr(current_pos);
	static size_t HeaderEnd;
	static size_t HeaderBegin;

	if (parseState == INHEADER)
	{
		HeaderEnd = request.find("\r\n\r\n");
		if (HeaderEnd == std::string::npos) // mazal khasni data mn core
		{
			std::cout << "INCOMPLETE HEADER\n";
			rtype = INCOMPLETE;
			return ;
		}
		HeaderBegin = request.find("\r\n");
		requestLine = request.substr(0, HeaderBegin);
		if (!parse_requestLine())
			return ; // should return a responce with error page
		HeaderBegin += 2;
		header = request.substr(HeaderBegin, HeaderEnd - HeaderBegin + 2);
		if (!header.size() || !parse_headers())
		{
			rtype = ERROR;
			errorCode = 400;
			return ;
		}
		parseState = INBODY;
	}
	rtype = KEEP_ALIVE;
	if (!parse_body(HeaderEnd + 4, request, conf))
	{
		if (rtype == ERROR)
			std::cout << "error code: [" << errorCode << "]\n";
		else if (rtype == INCOMPLETE)
			std::cout << "INCOMPLETE\n";
		return ;
	}
	std::cout << "DONE: body: [" << req.body << "]\n";
}

HttpRequest::HttpRequest()
{
	current_pos = 0;
	bodyType = NONE;
	bodyState = INSIZE;
	parseState = INHEADER;
	keepAlive = true;
}
HttpRequest::~HttpRequest(){}

// Getter method implementations
std::string HttpRequest::getMethod() const
{
	return req.method;
}

std::string HttpRequest::getTarget() const
{
	return req.request_target;
}

std::string HttpRequest::getQuery() const
{
	return req.query;
}

std::string HttpRequest::getBody() const
{
	return req.body;
}

std::string HttpRequest::getHeader(const std::string& key) const
{
	std::string lowerKey = key;
	std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
	
	std::map<std::string, std::string>::const_iterator it = req.headers.find(lowerKey);
	if (it != req.headers.end())
		return it->second;
	return "";
}

std::map<std::string, std::string> HttpRequest::getAllHeaders() const
{
	return req.headers;
}

size_t HttpRequest::getBodySize() const
{
	return body_size;
}