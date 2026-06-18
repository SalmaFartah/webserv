#pragma once
#include <string>
#include <map>
#include "../inc/HttpRequest.hpp"
#include "../../parse_config/inc/Fill.hpp"

class CGIHandler
{
	public:
		// Check if the request target is a CGI script based on file extension
		// Returns true if file extension matches cgi_extension in location config
		static bool isCGIRequest(const std::string& requestTarget, const locationConf& loc);
		
		// Extract file extension from a path
		static std::string getFileExtension(const std::string& filename);
		
		// Build CGI environment variables map
		// Maps standard CGI variables: REQUEST_METHOD, QUERY_STRING, SCRIPT_FILENAME, ...
		static std::map<std::string, std::string> buildCGIEnv(
			const HttpRequest& request,
			const locationConf& loc,
			const serverConf& srv,
			const std::string& scriptPath
		);
};
