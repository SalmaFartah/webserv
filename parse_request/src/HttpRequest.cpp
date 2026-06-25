#include "../inc/HttpRequest.hpp"

bool HttpRequest::parse_requestLine()
{
	std::string buff;
	std::stringstream line(requestLine);
	std::vector<std::string> segments;

	while (std::getline(line, buff, ' '))
		segments.push_back(buff);

	if (segments.size() != 3)
		return rtype = ERROR, errorCode = 400, false;

	req.method = segments[0];
	req.request_target = segments[1];
	req.httpVersion = segments[2];

	if (req.request_target.size() > MAX_URI_LENGTH)
		return errorCode = 414, rtype = ERROR, false;

	if ((req.method == "GET" || req.method == "POST" || req.method == "DELETE") \
	&& req.request_target.find("/") == 0 && req.request_target.find("//") == std::string::npos \
	&& req.httpVersion == "HTTP/1.1")
		return extract_query(), true;
	
	return errorCode = 400, rtype = ERROR, false;
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
	body_size = strtoul(value.c_str(), &end, 10);
	if (value[0] == '-' || value[0] == '+' || errno == ERANGE || *end)
		return true;
	return false;
}

bool HttpRequest::get_key(std::string &storeKey, const std::string &line, size_t keyEND)
{
	storeKey = line.substr(0, keyEND);
	std::transform(storeKey.begin(), storeKey.end(), storeKey.begin(), ::tolower);
	if (storeKey.find_first_of(" ") != std::string::npos || !isprintSTR(storeKey))
		return false;
	if (storeKey == "content-length")
		bodyType = NORMAL;
	if (storeKey == "transfer-encoding")
		bodyType = CHUNKED;
	return true;
}

bool HttpRequest::get_value(std::string &storeVal, const std::string &line, size_t valueStart)
{
	size_t start, end;
	storeVal = line.substr(valueStart, line.size());
	if ((!isprintSTR(storeVal) && storeVal.find("\t") == std::string::npos))
		return false;
	start = storeVal.find_first_not_of(" \t");
	end = storeVal.find_last_not_of(" \t");
	if (start == std::string::npos)
		storeVal = "";
	if (!storeVal.empty())
		storeVal = storeVal.substr(start, end - start + 1);
	return true;
}

bool HttpRequest::store_header(const std::string& key, const std::string& value, headerMap& headers)
{
	if ((key == "host" && (headers.count("host") || value.empty()))
	|| (key == "content-length" && (headers.count("content-length") || invalid_value(value)))
	|| (key == "transfer-encoding" && (value != "chunked" || headers.count("transfer-encoding")))
	|| (key == "content-Type" && headers.count("content-Type")))
		return false;
	if (key == "connection" && value == "close")
		req.connection = false;
	headers[key] = value;
	return true;
}

bool HttpRequest::parse_headers()
{
	size_t startLine = 0, eofLine, colon;
	std::string line, key, value;

	while (startLine < header.size())
	{
		eofLine = header.find("\r\n", startLine);
		line = header.substr(startLine, eofLine - startLine);
		colon = line.find(":");
		if (colon == std::string::npos || !colon)
			return false;
		if (!get_key(key, line, colon) \
		|| !get_value(value, line, colon + 1) \
		|| !store_header(key, value, req.headers))
			return false;
		startLine = eofLine + 2;
	}
	if (!req.headers.count("host") \
	|| (req.headers.count("content-length") \
	&& req.headers.count("transfer-encoding")))
		return false;
	return true;
}

bool HttpRequest::get_size(size_t& size, std::string bodyreq, size_t start, size_t end)
{
	std::string sizeSTR = bodyreq.substr(start, end - start);
	if (isspace(sizeSTR[0]) || sizeSTR[0] == '+' || sizeSTR[0] == '-')
		return rtype = ERROR, errorCode = 400, false;
	char *check = NULL;
	size = strtoul(sizeSTR.c_str(), &check, 16);
	if ((!size && sizeSTR.size() != 1) || (size && sizeSTR[0] == '0') || errno == ERANGE || *check)
		return errorCode = 400, rtype = ERROR, false;
	if (!size)
		bodyState = THE_END;
	return true;
}

bool HttpRequest::handle_chunked(std::string bodyreq)
{
		static size_t pos0 = 0, size = 0;
		size_t pos1 = 0, pos2;
		std::string chunk;
		while (1)
		{
			pos1 = bodyreq.find("\r\n", pos0);
			if (bodyState == INSIZE)
			{
				if (pos1 == std::string::npos)
					return rtype = INCOMPLETE, false;
				bodyState = IN_CHUNK;
				if (!get_size(size, bodyreq, pos0, pos1))
					return rtype = ERROR, errorCode = 400, false;
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
			}
			if (bodyState == THE_END)
			{
				std::string str = bodyreq.substr(pos1);
				if (str.size() < 4)
					return rtype = INCOMPLETE, false;
				if (str.size() >= 4 && str.find("\r\n\r\n"))
					return errorCode = 400, rtype = ERROR, false;
				if (!req.connection)
					rtype = DONE;
				current_pos += pos1 + 4;
				bodyState = INSIZE;
				bodyType = NONE;
				return true;
			}
			pos0 = pos1;
		}
}

bool HttpRequest::parse_body(size_t bodyStart, std::string request)
{
	current_pos += bodyStart;
	if (bodyType == NORMAL)
	{
		req.body = request.substr(bodyStart);
		if (req.body.size() < body_size)
			return rtype = INCOMPLETE, false;
		if (!req.connection)
			rtype = DONE;
		req.body = req.body.substr(0, body_size);
		bodyType = NONE;
		current_pos += body_size;
	}
	else if (bodyType == CHUNKED && !handle_chunked(request.substr(bodyStart)))
		return false;
	return true;
}

std::string HttpRequest::parse_request(std::string request, serverConf *conf)
{
	request = request.substr(current_pos);
	static size_t HeaderEnd;
	static size_t HeaderBegin;

	if (parseState == INHEADER)
	{
		HeaderEnd = request.find("\r\n\r\n");
		if (HeaderEnd == std::string::npos)
		{
			rtype = INCOMPLETE;
			return "";
		}
		HeaderBegin = request.find("\r\n");
		requestLine = request.substr(0, HeaderBegin);
		if (!parse_requestLine())
			return resp.error_response(*conf, empty, errorCode);
		HeaderBegin += 2;
		header = request.substr(HeaderBegin, HeaderEnd - HeaderBegin + 2);
		if (!header.size() || !parse_headers())
		{
			rtype = ERROR;
			return resp.error_response(*conf, empty, errorCode);
		}
		parseState = INBODY;
	}
	rtype = KEEP_ALIVE;
	if (!parse_body(HeaderEnd + 4, request))
		return resp.error_response(*conf, empty, errorCode);
	if (route.routeCheck(conf, req, 0) == -1)
		rtype = ERROR;

	parseState = INHEADER;
	req.connection = true;
	req.headers.clear();
	
	return route.getResponse(); // should rerurn the response from the route..
}

HttpRequest::HttpRequest()
{
	rtype = INCOMPLETE;
	current_pos = 0;
	bodyType = NONE;
	bodyState = INSIZE;
	parseState = INHEADER;
	req.connection = true;
}
HttpRequest::~HttpRequest(){}