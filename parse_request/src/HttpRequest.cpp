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
	content_length = strtoul(value.c_str(), &end, 10);
	if (value[0] == '-' || value[0] == '+' || errno == ERANGE || *end)
		return true;
	return false;
}

void HttpRequest::trim_WS(std::string& str)
{
	size_t start, end;
	start = str.find_first_not_of(" \t");
	end = str.find_last_not_of(" \t");
	if (start == std::string::npos)
		str = "";
	if (!str.empty())
		str = str.substr(start, end - start + 1);
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
	storeVal = line.substr(valueStart, line.size());
	std::transform(storeVal.begin(), storeVal.end(), storeVal.begin(), ::tolower);
	if ((!isprintSTR(storeVal) && storeVal.find("\t") == std::string::npos))
		return false;
	trim_WS(storeVal);
	return true;
}

bool HttpRequest::store_header(const std::string& key, const std::string& value, headerMap& headers)
{
	if (key == "content-type" && value.find("multipart/form-data") == 0)
	{
		bool x = false;
		std::string parameter = value.substr(std::string("multipart/form-data").size());
		if (!get_param(parameter, boundary, "boundary", x) \
		|| boundary.find_first_of(" \t") != std::string::npos)
			return boundary.clear(), false;
	}
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

bool HttpRequest::get_param(std::string& param, std::string& Val, std::string key, bool& keyFound)
{
	std::string holder, matchKey;
	trim_WS(param);
	if (param[0] != ';')
		return false;
	size_t paramEnd = param.find(";", 1);
	if (paramEnd == std::string::npos)
	{
		holder = param.substr(1); // ex: (  ; name="avatar") -> (name="avatar")
		param = "";
	}
	else
	{
		holder = param.substr(1, paramEnd - 1);// ex: (  ; name="avatar"; filename="somth") -> ( name="avatar")
		param = param.substr(paramEnd); // -> ; filename="somth")
	}
	trim_WS(holder); //  ex: (  name="avatar"   ) -> (name="avatar")
	size_t keyEnd = holder.find("=");
	if (keyEnd == std::string::npos\
	|| (matchKey = holder.substr(0, keyEnd)) != key )
	{
		keyFound = false;
		return false;
	}
	Val = holder.substr(keyEnd + 1);
	if (Val[0] == '"' && Val[Val.size() - 1] != '"')
		return false;
	
	if (Val[0] == '"')
		Val = Val.substr(1, Val.size() - 1);
	return true;
}

bool HttpRequest::parse_headers()
{
	size_t startLine = 0, eofLine, colon;
	std::string line, key, value;

	rtype = KEEP_ALIVE;
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
				rtype = KEEP_ALIVE;
				if (!req.connection)
					rtype = DONE;
				pos0 = 0;
				size = 0;
				current_pos += pos1 + 4;
				bodyState = INSIZE;
				bodyType = NONE;
				parseState = INHEADER;
				return true;
			}
			pos0 = pos1;
		}
}

bool HttpRequest::part_headers(std::string headPart)
{
	if (headPart.empty())
		return false;
	size_t startLine = 0, eofLine, colon;
	std::string line, key, value;
	while (startLine < headPart.size())
	{
		eofLine = headPart.find("\r\n", startLine);
		line = headPart.substr(startLine, eofLine - startLine);
		
		colon = line.find(":");
		if (colon == std::string::npos || !colon)
			return false;
		if (!get_key(key, line, colon) \
		|| !get_value(value, line, colon + 1))
			return false;
		if (key == "content-disposition")
		{
			std::string name, filename;
			bool keyFound = true;
			if (value.find("form-data"))
				return false;
			// if the name param is missing or has a wrong syntax
			//  like (name photo) or (name="photo) or (name="")
			std::string param = value.substr(std::string("form-data").size());
			if (!get_param(param, name, "name", keyFound))
				return false;
			 // if the filename param has a wrong syntax
			//  like (filename file.txt) or (filename="file.txt) or (filename="")
			if (!get_param(param, filename, "filename", keyFound) && keyFound)
				return false;
			
		}
		startLine = eofLine + 2;
	}
	return true;
}

bool HttpRequest::handle_multipart()
{
	std::string part;
	std::string firstDelim = "--" + boundary + "\r\n";
	std::string lastDelim = "\r\n--" + boundary + "--" + "\r\n";
	std::string delim = "\r\n" + firstDelim;
	size_t partStart, headerEnd, partEnd;

	if (req.body.find(firstDelim) != 0)
		return rtype = ERROR, false;

	partStart = firstDelim.size();
	std::cout << "size: " << partStart << " char: " << req.body.size() << "\n";
	while (partStart < req.body.size())
	{
		partEnd = req.body.find(delim, partStart);
		if (partEnd == std::string::npos)
		{
			delim = lastDelim;
			partEnd = req.body.find(lastDelim, partStart);
			if (partEnd == std::string::npos \
			|| (partEnd + lastDelim.size()) < req.body.size())
				return rtype = ERROR, false;
		}

		part = req.body.substr(partStart, partEnd - partStart);
		headerEnd = part.find("\r\n\r\n", partStart);
		if (headerEnd == std::string::npos)
			return rtype = ERROR, false;

		if (!part_headers(part.substr(0, headerEnd + 2)))
			return rtype = ERROR, false;
		
		headerEnd += 4;

		std::string RawBytes = part.substr(headerEnd);
		if (RawBytes.empty())
			return rtype = ERROR, false;
		req.uploads[filename] = RawBytes;
		partStart = partEnd + delim.size();
	}
	return true;
}

bool HttpRequest::parse_body(size_t bodyStart, std::string request)
{
	if (rtype != INCOMPLETE)
		current_pos += bodyStart;
	if (bodyType == NORMAL)
	{
		req.body = request.substr(bodyStart);
		if (req.body.size() < content_length)
			return rtype = INCOMPLETE, false;
		parseState = INHEADER;
		rtype = KEEP_ALIVE;
		if (!req.connection)
			rtype = DONE;
		req.body = req.body.substr(0, content_length);
		bodyType = NONE;
		current_pos += content_length;
	}
	else if ((bodyType == CHUNKED && !handle_chunked(request.substr(bodyStart))))
		return false;
	if (!boundary.empty() && !handle_multipart())
		return errorCode = 400, false;
	
	return true;
}

std::string HttpRequest::parse_request(std::string request, serverConf *conf)
{
	if (parseState == INHEADER)
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
			return resp.error_response(*conf, empty, 400);
		}
		parseState = INBODY;
		rtype = KEEP_ALIVE;
	}
	if (!parse_body(HeaderEnd + 4, request))
		return resp.error_response(*conf, empty, errorCode);
	if (route.routeCheck(conf, req, 0, CGIobj) == -1)
		rtype = ERROR;

	parseState = INHEADER;
	req.connection = true;
	req.headers.clear();

	return route.getResponse();
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