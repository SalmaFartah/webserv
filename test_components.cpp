#include "parse_request/inc/HttpRequest.hpp"
#include "parse_request/inc/FileHandler.hpp"
#include "parse_request/inc/CGIHandler.hpp"
#include "parse_request/inc/CGIExecutor.hpp"
#include <iostream>
#include <fstream>

// Mock serverConf for testing (simplified)
void printTestInfo()
{
	std::cout << "\n========================================\n";
	std::cout << "       HTTP REQUEST PARSING TEST\n";
	std::cout << "========================================\n\n";
}

void testHttpParsing()
{
	printTestInfo();
	
	// Simulate a POST request with a body
	std::string httpRequest = 
		"POST /upload HTTP/1.1\r\n"
		"Host: localhost:8080\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Content-Length: 22\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		"name=John&age=30&id=42";
	
	std::cout << "TEST 1: Parsing POST Request\n";
	std::cout << "Raw Request:\n" << httpRequest << "\n\n";
	
	// We can't test actual HttpRequest parsing without a full serverConf
	// But we can show the flow
	std::cout << "Expected parsed data:\n";
	std::cout << "  Method: POST\n";
	std::cout << "  Target: /upload\n";
	std::cout << "  Version: HTTP/1.1\n";
	std::cout << "  Headers:\n";
	std::cout << "    Host: localhost:8080\n";
	std::cout << "    Content-Type: application/x-www-form-urlencoded\n";
	std::cout << "    Content-Length: 22\n";
	std::cout << "  Body: name=John&age=30&id=42\n";
	std::cout << "  Body size: 22\n\n";
}

void testCGIDetection()
{
	std::cout << "\n========================================\n";
	std::cout << "     CGI DETECTION TEST\n";
	std::cout << "========================================\n\n";
	
	// Create a sample location config
	locationConf loc;
	loc.cgi_extension = ".php";
	
	std::cout << "TEST 2: CGI Request Detection\n";
	std::cout << "Configured CGI extension: .php\n\n";
	
	// Test various request targets
	std::vector<std::string> testUrls;
	testUrls.push_back("/index.php");
	testUrls.push_back("/api/script.php?id=123");
	testUrls.push_back("/upload.html");
	testUrls.push_back("/test.py");
	testUrls.push_back("/image.jpg");
	
	for (size_t i = 0; i < testUrls.size(); ++i)
	{
		bool isCGI = CGIHandler::isCGIRequest(testUrls[i], loc);
		std::cout << "  " << testUrls[i] << " -> ";
		std::cout << (isCGI ? "IS CGI (PHP)" : "NOT CGI") << "\n";
	}
	std::cout << "\n";
}

void testFileExtraction()
{
	std::cout << "\n========================================\n";
	std::cout << "    FILE EXTENSION EXTRACTION TEST\n";
	std::cout << "========================================\n\n";
	
	std::cout << "TEST 3: Extract File Extensions\n\n";
	
	std::vector<std::string> paths;
	paths.push_back("/var/www/index.php");
	paths.push_back("/scripts/test.py");
	paths.push_back("/upload.html");
	paths.push_back("/noextension");
	paths.push_back("/multiple.dots.php");
	
	for (size_t i = 0; i < paths.size(); ++i)
	{
		std::string ext = CGIHandler::getFileExtension(paths[i]);
		std::cout << "  " << paths[i];
		std::cout << " -> Extension: \"" << ext << "\"\n";
	}
	std::cout << "\n";
}

void testCGIEnvironmentBuilding()
{
	std::cout << "\n========================================\n";
	std::cout << "  CGI ENVIRONMENT VARIABLES TEST\n";
	std::cout << "========================================\n\n";
	
	std::cout << "TEST 4: Build CGI Environment\n\n";
	
	std::cout << "A real CGI execution would build these environment variables:\n\n";
	
	std::cout << "Standard CGI Variables:\n";
	std::cout << "  REQUEST_METHOD=POST\n";
	std::cout << "  QUERY_STRING=id=123&action=submit\n";
	std::cout << "  CONTENT_LENGTH=256\n";
	std::cout << "  CONTENT_TYPE=application/x-www-form-urlencoded\n";
	std::cout << "  SCRIPT_FILENAME=/var/www/api.php\n";
	std::cout << "  REQUEST_URI=/api.php?id=123&action=submit\n";
	std::cout << "  SERVER_NAME=localhost\n";
	std::cout << "  SERVER_PORT=8080\n\n";
	
	std::cout << "HTTP Request Headers (as HTTP_* variables):\n";
	std::cout << "  HTTP_HOST=localhost:8080\n";
	std::cout << "  HTTP_USER_AGENT=Mozilla/5.0\n";
	std::cout << "  HTTP_ACCEPT=text/html,application/xhtml+xml\n\n";
}

void testUploadHandler()
{
	std::cout << "\n========================================\n";
	std::cout << "      UPLOAD HANDLER TEST\n";
	std::cout << "========================================\n\n";
	
	std::cout << "TEST 5: Upload Handler Flow\n\n";
	
	std::cout << "Upload handler does the following:\n";
	std::cout << "  1. Validates request is POST method\n";
	std::cout << "  2. Checks request body is not empty\n";
	std::cout << "  3. Validates body size <= client_max_body_size\n";
	std::cout << "  4. Verifies upload directory exists and is writable\n";
	std::cout << "  5. Generates unique filename (upload_<timestamp>_<counter>)\n";
	std::cout << "  6. Writes binary data to disk\n";
	std::cout << "  7. Returns HTTP status code\n\n";
	
	std::cout << "Status codes returned:\n";
	std::cout << "  201 Created      - File uploaded successfully\n";
	std::cout << "  400 Bad Request  - Missing or invalid request\n";
	std::cout << "  405 Not Allowed  - Request method not POST\n";
	std::cout << "  413 Too Large    - Body exceeds size limit\n";
	std::cout << "  403 Forbidden    - Upload dir not configured or not writable\n";
	std::cout << "  500 Server Error - Failed to write file\n\n";
}

void testCGIExecution()
{
	std::cout << "\n========================================\n";
	std::cout << "      CGI EXECUTOR TEST\n";
	std::cout << "========================================\n\n";
	
	std::cout << "TEST 6: CGI Script Execution Flow\n\n";
	
	std::cout << "The CGI executor performs these steps:\n";
	std::cout << "  1. Create stdin pipe (parent -> child)\n";
	std::cout << "  2. Create stdout pipe (child -> parent)\n";
	std::cout << "  3. Fork a child process\n";
	std::cout << "  4. Child: redirect stdin/stdout to pipes\n";
	std::cout << "  5. Child: execute script with execve()\n";
	std::cout << "  6. Parent: write request body to stdin\n";
	std::cout << "  7. Parent: read script output from stdout\n";
	std::cout << "  8. Parent: wait for child to finish\n";
	std::cout << "  9. Return output + status code\n\n";
	
	std::cout << "Example execution:\n";
	std::cout << "  Script: /var/www/test.sh\n";
	std::cout << "  Interpreter: /bin/bash\n";
	std::cout << "  Request Body: name=John&age=30\n";
	std::cout << "  Timeout: 30 seconds\n\n";
	
	std::cout << "Result:\n";
	std::cout << "  Status Code: 200 (success) or 500 (error)\n";
	std::cout << "  Output: HTML/JSON returned by the script\n";
	std::cout << "  Error: Error message if script failed\n\n";
}

void demonstrateWorkflow()
{
	std::cout << "\n========================================\n";
	std::cout << "    COMPLETE REQUEST WORKFLOW\n";
	std::cout << "========================================\n\n";
	
	std::cout << "Here's how all components work together:\n\n";
	
	std::cout << "1. SERVER receives HTTP request from client\n";
	std::cout << "   Input: Raw bytes from socket\n";
	std::cout << "   Uses: HttpRequest parser\n";
	std::cout << "   Output: ReqContent struct with method, body, headers, etc.\n\n";
	
	std::cout << "2. SERVER checks request type\n";
	std::cout << "   Uses: CGIHandler::isCGIRequest()\n";
	std::cout << "   If .php/.py file -> It's a CGI request\n";
	std::cout << "   If POST to /upload -> It's a file upload\n";
	std::cout << "   If GET to /page.html -> It's a static file request\n\n";
	
	std::cout << "3a. FOR FILE UPLOADS:\n";
	std::cout << "   Uses: FileHandler::handleUpload()\n";
	std::cout << "   Output: HTTP 201 Created or error\n\n";
	
	std::cout << "3b. FOR CGI SCRIPTS:\n";
	std::cout << "   Step 1: CGIHandler::buildCGIEnv() -> Build environment\n";
	std::cout << "   Step 2: CGIExecutor::executeCGI() -> Run script\n";
	std::cout << "   Output: CGI script's HTML output\n\n";
	
	std::cout << "4. SERVER builds HTTP response\n";
	std::cout << "   Input: Status code, headers, body from handlers\n";
	std::cout << "   Output: Raw HTTP response to send to client\n\n";
}

int main()
{
	std::cout << "\n";
	std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
	std::cout << "║         WEBSERVER - HTTP/CGI/UPLOAD COMPONENTS TEST            ║\n";
	std::cout << "║                                                                ║\n";
	std::cout << "║  This demonstrates the 4 major components built for Person C:  ║\n";
	std::cout << "║  1. HTTP Request Parsing                                       ║\n";
	std::cout << "║  2. File Upload Handling                                       ║\n";
	std::cout << "║  3. CGI Script Detection                                       ║\n";
	std::cout << "║  4. CGI Execution (fork/pipe/execve)                           ║\n";
	std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
	
	testHttpParsing();
	testCGIDetection();
	testFileExtraction();
	testCGIEnvironmentBuilding();
	testUploadHandler();
	testCGIExecution();
	demonstrateWorkflow();
	
	std::cout << "\n========================================\n";
	std::cout << "            KEY INSIGHTS\n";
	std::cout << "========================================\n\n";
	
	std::cout << "HttpRequest class:\n";
	std::cout << "  - Parses HTTP headers and body\n";
	std::cout << "  - Handles chunked transfer encoding\n";
	std::cout << "  - Public getters: getMethod(), getBody(), getQuery(), etc.\n\n";
	
	std::cout << "FileHandler class:\n";
	std::cout << "  - Validates POST request\n";
	std::cout << "  - Creates upload directory if needed\n";
	std::cout << "  - Generates unique filenames\n";
	std::cout << "  - Returns HTTP status codes\n\n";
	
	std::cout << "CGIHandler class:\n";
	std::cout << "  - Detects if URL is a CGI script\n";
	std::cout << "  - Extracts file extensions\n";
	std::cout << "  - Builds CGI environment variables\n\n";
	
	std::cout << "CGIExecutor class:\n";
	std::cout << "  - Forks child process\n";
	std::cout << "  - Creates pipes for IPC\n";
	std::cout << "  - Executes script with execve()\n";
	std::cout << "  - Handles timeout with select()\n";
	std::cout << "  - Reads script output\n";
	std::cout << "  - Waits for process completion\n\n";
	
	std::cout << "\n========================================\n";
	std::cout << "           NEXT STEPS\n";
	std::cout << "========================================\n\n";
	
	std::cout << "To complete the server, you need:\n";
	std::cout << "  1. Main event loop (socket accept, poll)\n";
	std::cout << "  2. Request router (dispatch to handlers)\n";
	std::cout << "  3. Response builder (format HTTP responses)\n";
	std::cout << "  4. Error handler (500, 404, etc.)\n";
	std::cout << "  5. Testing suite (curl commands)\n\n";
	
	return 0;
}
