#pragma once
#include <string>
#include <map>
#include <vector>

class CGIExecutor
{
	public:
		// Structure to hold CGI execution result
		struct CGIResult
		{
			int statusCode;        // HTTP status code (200, 500,..)
			std::string output;    // Output from the CGI script (HTML, ..)
			std::string error;     // Error message if execution failed
			
			CGIResult() : statusCode(500), output(""), error("") {}
		};
		
		// Execute a CGI script with environment variables and request body
		// Returns CGI script output or error response
		static CGIResult executeCGI(
			const std::string& scriptPath,
			const std::string& interpreterPath,
			const std::map<std::string, std::string>& envVars,
			const std::string& requestBody,
			size_t timeout
		);
		
	private:
		// Convert std::map to char** array format for execve()
		static char** mapToEnvArray(const std::map<std::string, std::string>& envVars);
		
		// Free allocated environment array
		static void freeEnvArray(char** envArray, size_t count);
		
		// Read all data from a file descriptor with timeout
		static std::string readWithTimeout(int fd, size_t timeout);
};
