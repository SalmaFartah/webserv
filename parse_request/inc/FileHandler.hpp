#pragma once
#include <string>
#include <map>
#include "../inc/HttpRequest.hpp"
#include "../../parse_config/inc/Fill.hpp"

class FileHandler
{
	public:
		// Save uploaded file from POST request
		// Returns status code: 201 (Created), 400 (Bad Request), 403 (Forbidden), 413 (Too Large), 500 (Server Error)
		static int handleUpload(const HttpRequest& request, const locationConf& loc);
		
		// Generate HTTP response body for upload status
		static std::string generateUploadResponse(int statusCode, const std::string& filename);
		
		// Check if upload directory exists and is writable
		static bool isUploadDirValid(const std::string& uploadDir);
		
		// Generate a unique filename for the uploaded file
		static std::string generateFilename();
};
