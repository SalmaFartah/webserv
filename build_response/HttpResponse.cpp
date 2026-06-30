#include "HttpResponse.hpp"

std::string HttpResponse::getReasonPhrase(int code)
{
	std::string reason_phrase;
	switch (code)
	{
		case 200:
			reason_phrase = "Ok";
			break;
		case 201:
			reason_phrase = "Created";
			break;
		case 204:
			reason_phrase = "No Content";
			break;
		case 301:
			reason_phrase = "Moved Permanently";
			break;
		case 302:
			reason_phrase = "Found";
			break;
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
		case 414:
			reason_phrase = "URI Too Long";
			break;
		case 500:
			reason_phrase = "Internal Server Error";
			break;
		case 501:
			reason_phrase = "Not Implemented";
			break;
		case 502:
			reason_phrase = "Bad Gateway";
			break;
		case 503:
			reason_phrase = "Service Unavailable";
		case 504:
			reason_phrase = "Gateway Timeout";
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


std::string HttpResponse::build(int code, const std::string& body, const std::string& ctype, bool con)
{
	std::stringstream response;
	std::string connType("close");
	if (con)
		connType = "Keep-alive";
	response << "HTTP/1.1 " << code << " " << getReasonPhrase(code) << "\r\n";
	response << "Server: webserv/1.0\r\n";
	response << "Content-Length: " << body.size() << "\r\n";
	if (!ctype.empty())
		response << "Content-Type: " << ctype << "\r\n";
	response << "Connection: " << connType << "\r\n";
	if (code == 405)
		response << "Allow: " << methods << "\r\n";
	if (code == 301 || code == 302)
		response << "Location: " << url << "\r\n";
	response << "\r\n";
	response << body;
	return response.str();
}

std::string HttpResponse::getErrorPage(int code, const std::string& reason_phrase)
{
	std::ostringstream oss;
	oss << "<html><body><h1>" << code << " " << reason_phrase << "</h1></body></html>";
	return oss.str();
}

std::string HttpResponse::error_response(serverConf& server, locationConf& location, int errorCode)
{
	for (std::set<std::string>::iterator it = location.methods.begin() ; it != location.methods.end(); it++)
		methods += *it + " ";
	error = true;
	std::string reason_phrase = getReasonPhrase(errorCode);
	std::string contentType, fileName;
	std::ostringstream body;

	body << getErrorPage(errorCode, reason_phrase);
	contentType = "text/html";
	if (location.error_page.count(errorCode))
		fileName = location.error_page[errorCode];
	else if (server.error_page.count(errorCode))
		fileName = server.error_page[errorCode];
	if (!fileName.empty())
	{
		std::cout << "is empty\n";
		std::string ext("default");
		size_t pos = fileName.find(".");
		if (pos && pos != std::string::npos)
			ext = fileName.substr(pos + 1);
		std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
		std::ifstream errFile(fileName, std::ios::binary);
		if (errFile.is_open())
		{
			if (!MIME_table.count(ext))
				ext = "default";
			contentType = MIME_table[ext];
			body.clear();
			body << errFile.rdbuf();
		}
	}
	return build(errorCode, body.str(), contentType, false);
}

std::string HttpResponse::delete_method(serverConf& serv, locationConf& loc, const std::string& path, bool con)
{
	int ret = unlink(path.c_str());
	if (ret == -1 && (errno == EACCES || errno == EROFS))
		return error_response(serv, loc, 403);
	if (ret == -1)
		return error_response(serv, loc, 500);
	return build(204, "", "", con);
}

std::string HttpResponse::static_file(serverConf& serv, locationConf& loc, const std::string& path, bool con)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
		return error_response(serv, loc, 500);
	std::ostringstream body;
	body << file.rdbuf();
	std::string ext("default");
	size_t dot = path.find_last_of(".");
	if (dot && dot != std::string::npos)
		ext = path.substr(dot + 1);
	if (!MIME_table.count(ext))
		ext = "default";
	std::string ctype = MIME_table[ext];
	return build(200, body.str(), ctype, con);
}

std::string HttpResponse::directory(serverConf& serv, locationConf& loc, std::string path, bool con)
{
	if (path[path.size() - 1] != '/')
		path += "/";
	if (!loc.index.empty())
	{
		struct stat st;
		for (std::vector<std::string>::iterator it = loc.index.begin(); it != loc.index.end(); it++)
		{
			if (stat((path + *it).c_str(), &st) == 0 && (st.st_mode & S_IFREG))
				return static_file(serv, loc, path + *it, con);
		}
	}
	if (!loc.autoindex)
		return error_response(serv, loc, 403);

	/****** OPEN DIRECTORY ******/
	DIR *direct = opendir(path.c_str());
	if (!direct && errno == EACCES)
		return error_response(serv, loc, 403);
	if (!direct && (errno == EMFILE || errno == ENFILE))
		return error_response(serv, loc, 500);
	/****** CREATE BODY ******/
	errno = 0;
	dirent *read;
	std::ostringstream body;

	while ((read = readdir(direct)))
	{
		if (std::string(read->d_name) != "." && std::string(read->d_name) != "..")
			body << "<a href=\"" << read->d_name << "\">" << read->d_name << "</a>";
	}

	/****** CLOSE DIRECTORY && BUILD RESPONSE ******/
	closedir(direct);
	if (errno)
		return error_response(serv, loc, 500);
	return build(200, body.str(), "text/html", con);
}

std::string HttpResponse::redirect(int code, const std::string& URL, bool con)
{
	this->url = URL;
	return build(code, "", "", con);
}

HttpResponse::HttpResponse()
{
	initMimeTable();
	error = false;
}

HttpResponse::~HttpResponse(){}
