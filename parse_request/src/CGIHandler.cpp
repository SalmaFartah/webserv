#include "../inc/CGIHandler.hpp"
#include <sstream>
#include <algorithm>

std::string CGIHandler::getFileExtension(const std::string& filename)
{
	size_t dotPos = filename.find_last_of(".");
	if (dotPos == std::string::npos || dotPos == filename.length() - 1)
		return "";
	return filename.substr(dotPos);
}

// Check if request target matches CGI extension in location config
bool CGIHandler::isCGIRequest(const std::string& requestTarget, const locationConf& loc)
{
	// If no CGI extension is configured, this is not a CGI request
	if (loc.cgi_extension.empty())
		return false;
	
	// Extract filename from request target (ignore query string and fragments)
	std::string path = requestTarget;
	size_t queryPos = path.find("?");
	if (queryPos != std::string::npos)
		path = path.substr(0, queryPos);
	
	std::string ext = getFileExtension(path);
	
	// Compare with configured CGI extension (case-insensitive)
	if (ext.empty())
		return false;
	
	std::string configExt = loc.cgi_extension;
	
	// Convert both to lowercase for comparison
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	std::transform(configExt.begin(), configExt.end(), configExt.begin(), ::tolower);
	
	return ext == configExt;
}

// Build CGI environment variables from request and config
std::map<std::string, std::string> CGIHandler::buildCGIEnv(
	const HttpRequest& request,
	const locationConf& /*loc*/,
	const serverConf& srv,
	const std::string& scriptPath
)
{
	std::map<std::string, std::string> env;
	
	// REQUEST_METHOD: GET, POST, DELETE, etc.
	env["REQUEST_METHOD"] = request.getMethod();
	
	// QUERY_STRING: everything after ? in the URL
	env["QUERY_STRING"] = request.getQuery();
	
	// CONTENT_LENGTH: size of the request body
	std::stringstream ss;
	ss << request.getBodySize();
	env["CONTENT_LENGTH"] = ss.str();
	
	// CONTENT_TYPE: from Content-Type header
	std::string contentType = request.getHeader("content-type");
	if (!contentType.empty())
		env["CONTENT_TYPE"] = contentType;
	
	// SCRIPT_FILENAME: full path to the CGI script
	env["SCRIPT_FILENAME"] = scriptPath;
	
	// REQUEST_URI: the full request path (without query string parameters parsed out)
	env["REQUEST_URI"] = request.getTarget();
	
	// PATH_INFO: path after the script name (for now, same as request target)
	env["PATH_INFO"] = request.getTarget();
	
	// SERVER_NAME: hostname from server config
	if (!srv.listen.empty())
		env["SERVER_NAME"] = srv.listen[0].first; // Use first listen address
	
	// SERVER_PORT: port from server config
	if (!srv.listen.empty())
	{
		std::stringstream portStr;
		portStr << srv.listen[0].second;
		env["SERVER_PORT"] = portStr.str();
	}
	
	// HTTP_HOST: from request headers (Host header)
	std::string host = request.getHeader("host");
	if (!host.empty())
		env["HTTP_HOST"] = host;
	
	// Add HTTP_* environment variables for each header
	std::map<std::string, std::string> allHeaders = request.getAllHeaders();
	for (std::map<std::string, std::string>::const_iterator it = allHeaders.begin(); 
		 it != allHeaders.end(); ++it)
	{
		// Convert header name to HTTP_* format: Content-Type -> HTTP_CONTENT_TYPE
		std::string headerKey = it->first;
		std::transform(headerKey.begin(), headerKey.end(), headerKey.begin(), ::toupper);
		
		// Replace hyphens with underscores
		for (size_t i = 0; i < headerKey.length(); ++i)
		{
			if (headerKey[i] == '-')
				headerKey[i] = '_';
		}
		
		env["HTTP_" + headerKey] = it->second;
	}
	
	return env;
}
