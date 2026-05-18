#include "../inc/HttpRequest.hpp"

bool HttpRequest::parse_requestLine()
{
	std::string buff;
	std::stringstream line(requestLine);
	size_t i = 0;

	//***************** SPLIT THE LINE BY SPACE ****************//
	std::vector<std::string> segments;
	while (std::getline(line, buff, ' '))
	{
		segments.push_back(buff);
		i++;
	}

	//* CHECK IF EXACTLY THREE SEGMENTS: METHOD, TARGET, VERSION *//
	if (i != 3)
		return false;

	//********** EXTRACT METHOD, TARGETA AND VERSION ************//
	method = segments[0];
	request_target = segments[1];
	httpVersion = segments[2];

	//******** CHECK IF METHOD, TARGET AND VERSION ARE VALID *****//
	if ((method == "GET" || method == "POST" || method == "DELETE") \
	&& request_target.find("/") == 0 && (httpVersion == "HTTP/1.0" \
	|| httpVersion == "HTTP/1.1"))
		return true;

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
		if (colon != std::string::npos)
		{
			std::string key = line.substr(0, colon);
			if (key.find_first_of(" \t") != std::string::npos)
				return false;
			std::string value = line.substr(colon + 1, line.size());
			// trim spaces from start and last of value
			value = value.substr(value.find_first_not_of(" \t"), value.find_last_not_of(" \t") - value.find_first_not_of(" \t") + 1);
			
		}
		eofLine += 2;
		startLine = eofLine;
	}
	return true;
}

void HttpRequest::ParseUpHeader(std::string request)
{
	ssize_t HeaderBeg = request.find("\r\n");

	if (HeaderBeg == request.npos)
	{
		// error
		std::cerr << "error: malformed request line.\n";
		return ;
	}
	requestLine = request.substr(0, HeaderBeg);
	if (parse_requestLine() == ERROR)
	{
		std::cerr << "error: malformed request line.\n";
		return ;
	}
	extract_query();

	// std::cout << "method: " << method << "\nrequest target: " << request_target << "\nhttpVersion: " << httpVersion << std::endl;
	// std::cout << "query: " << query << "\n";

	HeaderBeg += 2;
	header = request.substr(HeaderBeg, request.size());
	// std::cout << "headers: [" << header << "]\n";
	// std::cout << "size: " << header.size() << "\n";
	parse_headers();
}
HttpRequest::HttpRequest(){}
HttpRequest::~HttpRequest(){}