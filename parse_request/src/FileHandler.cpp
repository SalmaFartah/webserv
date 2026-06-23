#include "../inc/FileHandler.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>

// Check if upload directory exists and is writable
bool FileHandler::isUploadDirValid(const std::string& uploadDir)
{
	DIR* dir = opendir(uploadDir.c_str());
	if (!dir)
	{
		// Try to create the directory
		if (mkdir(uploadDir.c_str(), 0755) == -1)
		{
			std::cerr << "Error creating upload directory: " << strerror(errno) << std::endl;
			return false;
		}
	}
	else
		closedir(dir);
	
	// Check if it's writable
	if (access(uploadDir.c_str(), W_OK) == -1)
	{
		std::cerr << "Upload directory is not writable: " << strerror(errno) << std::endl;
		return false;
	}
	return true;
}

// Generate a unique filename based on timestamp and counter
std::string FileHandler::generateFilename()
{
	static unsigned int counter = 0;
	std::stringstream ss;
	
	// Use timestamp + counter for uniqueness
	ss << "upload_" << time(NULL) << "_" << (counter++);
	return ss.str();
}

// Save uploaded file and return HTTP status code
int FileHandler::handleUpload(const HttpRequest& request, const locationConf& loc)
{
	// Step 1: Validate request method
	if (request.getMethod() != "POST")
		return 405; // Method Not Allowed
	
	// Step 2: Check if body is empty
	std::string body = request.getBody();
	if (body.empty())
		return 400; // Bad Request
	
	// Step 3: Check body size against limit
	size_t bodySize = request.getBodySize();
	if (loc.body_size > 0 && bodySize > loc.body_size)
		return 413; // Payload Too Large
	
	// Step 4: Validate upload directory
	if (loc.upload_store.empty())
		return 403; // Forbidden - no upload directory configured
	
	if (!isUploadDirValid(loc.upload_store))
		return 403; // Forbidden - directory not accessible
	
	// Step 5: Generate unique filename
	std::string filename = generateFilename();
	std::string filepath = loc.upload_store;
	
	// Add trailing slash if missing
	if (filepath[filepath.length() - 1] != '/')
		filepath += "/";
	filepath += filename;
	
	// Step 6: Write file
	std::ofstream outFile(filepath.c_str(), std::ios::binary);
	if (!outFile.is_open())
	{
		std::cerr << "Failed to create upload file: " << strerror(errno) << std::endl;
		return 500; // Internal Server Error
	}
	
	outFile.write(body.c_str(), body.length());
	if (!outFile.good())
	{
		std::cerr << "Error writing to upload file: " << strerror(errno) << std::endl;
		outFile.close();
		return 500; // Internal Server Error
	}
	
	outFile.close();
	
	std::cout << "File uploaded successfully to: " << filepath << std::endl;
	return 201; // Created
}
