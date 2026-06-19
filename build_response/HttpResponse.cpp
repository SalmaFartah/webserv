#include "HttpResponse.hpp"

std::string HttpResponse::getReasonPhrase(int code)
{
	std::string reason_phrase;
	switch (code)
	{
		case 400:
			reason_phrase = "Bad Request";
			break;
		case 403:
			reason_phrase = "Forbidden";
			break;
		case 404:
			reason_phrase = "Not Found";
			break;
		case 405:
			reason_phrase = "Method Not Allowed";
			break;
		case 408:
			reason_phrase = "Time out";
			break;
		case 413:
			reason_phrase = "Content Too Large";
			break;
		case 500:
			reason_phrase = "Internal Server Error";
			break;
		case 501:
			reason_phrase = "Not Implemented";
			break;
	}
	return reason_phrase;
}

void HttpResponse::initMimeTable()
{
    MIME_table["html"]  = "text/html";
    MIME_table["htm"]   = "text/html";
    MIME_table["css"]   = "text/css";
    MIME_table["js"]    = "application/javascript";
    MIME_table["json"]  = "application/json";
    MIME_table["txt"]   = "text/plain";
    MIME_table["xml"]   = "application/xml";
    MIME_table["jpg"]   = "image/jpeg";
    MIME_table["jpeg"]  = "image/jpeg";
    MIME_table["png"]   = "image/png";
    MIME_table["gif"]   = "image/gif";
    MIME_table["svg"]   = "image/svg+xml";
    MIME_table["ico"]   = "image/x-icon";
    MIME_table["webp"]  = "image/webp";
    MIME_table["pdf"]   = "application/pdf";
    MIME_table["zip"]   = "application/zip";
    MIME_table["mp4"]   = "video/mp4";
    MIME_table["mp3"]   = "audio/mpeg";
    MIME_table["woff"]  = "font/woff";
    MIME_table["woff2"] = "font/woff2";
    MIME_table["default"] = "application/octet-stream";
}

std::string HttpResponse::getErrorPage(int code, const std::string& reason_phrase)
{
	std::ostringstream oss;
	oss << "<html><body><h1>" << code << " " << reason_phrase << "</h1></body></html>";
	return oss.str();
}
std::string HttpResponse::get_errbody(int errorCode, const std::string& reason_phrase, serverConf& server, locationConf& location, std::string& contentType)
{
	std::string body, fileName;
	body = getErrorPage(errorCode, reason_phrase);
	contentType = "text/html";
	if (location.error_page.count(errorCode))
		fileName = location.error_page[errorCode];
	else if (server.error_page.count(errorCode))
		fileName = server.error_page[errorCode];
	if (!fileName.empty())
	{
		std::string ext("default");
		size_t pos = fileName.find(".");
		if (pos && pos != std::string::npos)
			ext = fileName.substr(pos + 1);
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		std::ifstream errFile(fileName);
		if (errFile.is_open())
		{
			if (!MIME_table.count(ext))
				ext = "default";
			contentType = MIME_table[ext];
			body.clear();
			std::string line;
			while (std::getline(errFile, line))
				body += line;
		}
	}
	return body;
}
std::string HttpResponse::error_response(int errorCode, serverConf& server, locationConf& location)
{
	std::string reason_phrase = getReasonPhrase(errorCode);
	std::string fileName;
	std::string body;
	std::string contentType;

	std::ostringstream response;
	/************************STATUS LINE**********************/
	response << "HTTP/1.1 " << errorCode << " " << reason_phrase << "\r\n";

	/***************************BODY**************************/
	body = get_errbody(errorCode, reason_phrase, server, location, contentType);
	response << "Server: webserv/1.0\r\n";
	response << "Content-Length: " << body.size() << "\r\n";
	response << "Content-Type: " << contentType << "\r\n";
	response << "Connection: close\r\n";
	if (errorCode == 405)
	{
		response << "Allow: ";
		std::set<std::string >::iterator it;
		for (it = location.methods.begin(); it != location.methods.end(); it++)
			response << *it << " ";
		response << "\r\n";
	}
	response << "\r\n";
	response << body;
	return response.str();
}

HttpResponse::HttpResponse()
{
	initMimeTable();
}
HttpResponse::~HttpResponse()
{
}
