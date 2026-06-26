#include <iostream>
#include <string>
#include <vector>
#include <map>

// Simplified demonstration without needing full compilation
void demonstrateArchitecture()
{
	std::cout << "\n";
	std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
	std::cout << "║         WEBSERVER - HTTP/CGI/UPLOAD ARCHITECTURE               ║\n";
	std::cout << "║                                                                ║\n";
	std::cout << "║  This shows the 4 major components built for Person C:         ║\n";
	std::cout << "║  1. HTTP Request Parser (with getter methods)                  ║\n";
	std::cout << "║  2. File Upload Handler                                        ║\n";
	std::cout << "║  3. CGI Script Detection                                       ║\n";
	std::cout << "║  4. CGI Script Executor (fork/pipe/execve)                     ║\n";
	std::cout << "╚════════════════════════════════════════════════════════════════╝\n";
	
	std::cout << "\n========================================\n";
	std::cout << "       COMPONENT 1: HTTP PARSER\n";
	std::cout << "========================================\n\n";
	
	std::cout << "Location: parse_request/src/HttpRequest.cpp\n";
	std::cout << "Public methods (getters):\n";
	std::cout << "  • std::string getMethod()        -> \"GET\", \"POST\", \"DELETE\"\n";
	std::cout << "  • std::string getTarget()        -> \"/api/upload\"\n";
	std::cout << "  • std::string getQuery()         -> \"id=123&action=submit\"\n";
	std::cout << "  • std::string getBody()          -> Raw POST body data\n";
	std::cout << "  • std::string getHeader(key)     -> Value of specific header\n";
	std::cout << "  • map<str,str> getAllHeaders()   -> All headers as map\n";
	std::cout << "  • size_t getBodySize()           -> Size of request body\n\n";
	
	std::cout << "Workflow:\n";
	std::cout << "  1. Server receives raw HTTP bytes from socket\n";
	std::cout << "  2. Creates HttpRequest object\n";
	std::cout << "  3. Calls parse_request(rawData, serverConfig)\n";
	std::cout << "  4. Internal state machine parses headers and body\n";
	std::cout << "  5. Server calls getter methods to access parsed data\n\n";
	
	std::cout << "Example:\n";
	std::cout << "  Raw request:\n";
	std::cout << "    POST /upload HTTP/1.1\\r\\n\n";
	std::cout << "    Host: localhost:8080\\r\\n\n";
	std::cout << "    Content-Length: 11\\r\\n\n";
	std::cout << "    \\r\\n\n";
	std::cout << "    name=John&age=30\n\n";
	std::cout << "  After parsing:\n";
	std::cout << "    req.getMethod()     -> \"POST\"\n";
	std::cout << "    req.getTarget()     -> \"/upload\"\n";
	std::cout << "    req.getBody()       -> \"name=John&age=30\"\n";
	std::cout << "    req.getBodySize()   -> 16\n\n";
	
	std::cout << "\n========================================\n";
	std::cout << "       COMPONENT 2: UPLOAD HANDLER\n";
	std::cout << "========================================\n\n";
	
	std::cout << "Location: parse_request/src/FileHandler.cpp\n";
	std::cout << "Main function:\n";
	std::cout << "  static int handleUpload(const HttpRequest& req, const locationConf& loc)\n\n";
	
	std::cout << "Validation steps:\n";
	std::cout << "  1. Check if request method is POST\n";
	std::cout << "  2. Check if body is not empty\n";
	std::cout << "  3. Check if body size <= loc.body_size\n";
	std::cout << "  4. Check if upload directory exists (create if needed)\n";
	std::cout << "  5. Check if directory is writable\n\n";
	
	std::cout << "If all valid:\n";
	std::cout << "  1. Generate unique filename: \"upload_<timestamp>_<counter>\"\n";
	std::cout << "  2. Write body to disk in binary mode\n";
	std::cout << "  3. Return HTTP 201 (Created)\n\n";
	
	std::cout << "HTTP status codes:\n";
	std::cout << "  201 Created        - File uploaded successfully\n";
	std::cout << "  400 Bad Request    - Missing or invalid request\n";
	std::cout << "  405 Not Allowed    - Request method not POST\n";
	std::cout << "  413 Too Large      - Body exceeds size limit\n";
	std::cout << "  403 Forbidden      - Upload dir not writable\n";
	std::cout << "  500 Server Error   - Failed to write file\n\n";
	
	std::cout << "Usage:\n";
	std::cout << "  int status = FileHandler::handleUpload(request, location);\n";
	std::cout << "  std::string response = FileHandler::generateUploadResponse(\n";
	std::cout << "      status, filename);\n\n";
	
	std::cout << "\n========================================\n";
	std::cout << "       COMPONENT 3: CGI DETECTION\n";
	std::cout << "========================================\n\n";
	
	std::cout << "Location: parse_request/src/CGIHandler.cpp\n";
	std::cout << "Main function:\n";
	std::cout << "  bool isCGIRequest(const std::string& target, const locationConf& loc)\n\n";
	
	std::cout << "How it works:\n";
	std::cout << "  1. Extract file extension from request target\n";
	std::cout << "    Example: \"/api/script.php?id=123\" -> \".php\"\n";
	std::cout << "  2. Compare with location.cgi_extension\n";
	std::cout << "    Example: \".php\" == \".php\" -> true\n";
	std::cout << "  3. Return true if match, false otherwise\n\n";
	
	std::cout << "Helper function:\n";
	std::cout << "  std::string getFileExtension(const std::string& path)\n";
	std::cout << "    \"/var/www/api.php\" -> \".php\"\n";
	std::cout << "    \"/image.jpg\" -> \".jpg\"\n";
	std::cout << "    \"/noextension\" -> \"\"\n\n";
	
	std::cout << "Environment building:\n";
	std::cout << "  map<str,str> buildCGIEnv(...)\n";
	std::cout << "    Creates standard CGI variables:\n";
	std::cout << "      REQUEST_METHOD=POST\n";
	std::cout << "      QUERY_STRING=id=123\n";
	std::cout << "      CONTENT_LENGTH=256\n";
	std::cout << "      CONTENT_TYPE=application/json\n";
	std::cout << "      SCRIPT_FILENAME=/var/www/api.php\n";
	std::cout << "      HTTP_HOST=localhost:8080\n";
	std::cout << "      ... and more\n\n";
	
	std::cout << "\n========================================\n";
	std::cout << "       COMPONENT 4: CGI EXECUTOR\n";
	std::cout << "========================================\n\n";
	
	std::cout << "Location: parse_request/src/CGIExecutor.cpp\n";
	std::cout << "Main function:\n";
	std::cout << "  CGIResult executeCGI(const std::string& scriptPath,\n";
	std::cout << "                        const std::string& interpreterPath,\n";
	std::cout << "                        const map<str,str>& envVars,\n";
	std::cout << "                        const std::string& requestBody,\n";
	std::cout << "                        size_t timeout)\n\n";
	
	std::cout << "System programming steps:\n";
	std::cout << "  1. Create pipe(stdinPipe)  for sending data to script\n";
	std::cout << "  2. Create pipe(stdoutPipe) for reading output from script\n";
	std::cout << "  3. fork() to create child process\n\n";
	
	std::cout << "  CHILD PROCESS:\n";
	std::cout << "    1. dup2(stdinPipe[0], STDIN_FILENO)    - redirect stdin\n";
	std::cout << "    2. dup2(stdoutPipe[1], STDOUT_FILENO)  - redirect stdout\n";
	std::cout << "    3. execve(interpreterPath, args, envVars) - run script\n";
	std::cout << "       Example: execve(\"/usr/bin/php\", [\"php\", \"script.php\"], env)\n\n";
	
	std::cout << "  PARENT PROCESS:\n";
	std::cout << "    1. write(stdinPipe[1], requestBody, length) - send POST data\n";
	std::cout << "    2. close(stdinPipe[1]) - signal EOF to child\n";
	std::cout << "    3. select() with timeout - read output without blocking\n";
	std::cout << "    4. read(stdoutPipe[0], buffer) - capture script output\n";
	std::cout << "    5. waitpid(pid) - wait for child to finish\n";
	std::cout << "    6. Check exit status - success=0, error=500\n\n";
	
	std::cout << "Return value (CGIResult):\n";
	std::cout << "  statusCode  - 200 (success) or 500 (error)\n";
	std::cout << "  output      - HTML/JSON from the script\n";
	std::cout << "  error       - Error message if failed\n\n";
	
	std::cout << "\n========================================\n";
	std::cout << "     COMPLETE REQUEST WORKFLOW\n";
	std::cout << "========================================\n\n";
	
	std::cout << "When a client sends a request:\n\n";
	
	std::cout << "1. SERVER RECEIVES REQUEST\n";
	std::cout << "   Socket -> Raw bytes -> HttpRequest parser\n";
	std::cout << "   Output: Parsed request object with all methods available\n\n";
	
	std::cout << "2. SERVER DETERMINES REQUEST TYPE\n";
	std::cout << "   if (isCGIRequest(target, location))\n";
	std::cout << "       -> Handle as CGI script\n";
	std::cout << "   else if (method == \"POST\" && target == \"/upload\")\n";
	std::cout << "       -> Handle as file upload\n";
	std::cout << "   else\n";
	std::cout << "       -> Serve static file\n\n";
	
	std::cout << "3a. CGI REQUEST:\n";
	std::cout << "   env = buildCGIEnv(request, location, server, scriptPath)\n";
	std::cout << "   result = executeCGI(scriptPath, interpreter, env, body, timeout)\n";
	std::cout << "   response = \"HTTP/1.1 200 OK\\r\\nContent-Length: ...\\r\\n\\r\\n\" + result.output\n\n";
	
	std::cout << "3b. FILE UPLOAD REQUEST:\n";
	std::cout << "   status = handleUpload(request, location)\n";
	std::cout << "   response = generateUploadResponse(status, filename)\n";
	std::cout << "   Send HTTP 201 or error status\n\n";
	
	std::cout << "3c. STATIC FILE:\n";
	std::cout << "   Find file on disk\n";
	std::cout << "   Read file contents\n";
	std::cout << "   Send HTTP 200 with file data\n\n";
	
	std::cout << "\n========================================\n";
	std::cout << "          CODE STRUCTURE\n";
	std::cout << "========================================\n\n";
	
	std::cout << "parse_request/inc/\n";
	std::cout << "  ├── HttpRequest.hpp      - Request parser interface + getters\n";
	std::cout << "  ├── FileHandler.hpp      - Upload handler interface\n";
	std::cout << "  ├── CGIHandler.hpp       - CGI detection + environment\n";
	std::cout << "  └── CGIExecutor.hpp      - CGI execution (fork/pipe)\n\n";
	
	std::cout << "parse_request/src/\n";
	std::cout << "  ├── HttpRequest.cpp      - HTTP parser state machine\n";
	std::cout << "  ├── FileHandler.cpp      - File write + validation\n";
	std::cout << "  ├── CGIHandler.cpp       - Extension matching + env vars\n";
	std::cout << "  └── CGIExecutor.cpp      - fork/pipe/execve orchestration\n\n";
	
	std::cout << "\n========================================\n";
	std::cout << "           KEY CONCEPTS\n";
	std::cout << "========================================\n\n";
	
	std::cout << "Pipes:\n";
	std::cout << "  - int pipe[2] creates two file descriptors\n";
	std::cout << "  - pipe[0] = read end, pipe[1] = write end\n";
	std::cout << "  - Allows communication between parent and child process\n\n";
	
	std::cout << "Fork:\n";
	std::cout << "  - Creates identical copy of current process\n";
	std::cout << "  - Returns 0 in child, PID in parent\n";
	std::cout << "  - Child has own memory, but shares open file descriptors\n\n";
	
	std::cout << "execve:\n";
	std::cout << "  - Replaces process image with new program\n";
	std::cout << "  - Takes path, arguments, and environment\n";
	std::cout << "  - Does not return if successful\n\n";
	
	std::cout << "dup2:\n";
	std::cout << "  - Duplicates file descriptor\n";
	std::cout << "  - dup2(sourcefd, targetfd) - redirects targetfd to sourcefd\n";
	std::cout << "  - Used to redirect stdin/stdout/stderr to pipes\n\n";
	
	std::cout << "select:\n";
	std::cout << "  - Waits for file descriptors to be ready\n";
	std::cout << "  - Implements timeout to prevent blocking forever\n";
	std::cout << "  - Used to read output without hanging\n\n";
	
	std::cout << "\n========================================\n";
	std::cout << "       WHAT'S MISSING (Next Steps)\n";
	std::cout << "========================================\n\n";
	
	std::cout << "To complete a working server:\n\n";
	
	std::cout << "1. Main Event Loop (Person A):\n";
	std::cout << "   • Create listening socket\n";
	std::cout << "   • Use poll() to handle multiple clients\n";
	std::cout << "   • Accept connections\n";
	std::cout << "   • Parse HTTP from each client\n\n";
	
	std::cout << "2. Response Builder (Person B):\n";
	std::cout << "   • Format HTTP response headers\n";
	std::cout << "   • Set Content-Type, Content-Length\n";
	std::cout << "   • Handle error pages (404, 500, etc.)\n";
	std::cout << "   • Manage keep-alive connections\n\n";
	
	std::cout << "3. Request Router:\n";
	std::cout << "   • Dispatch request to correct handler\n";
	std::cout << "   • Check which location block matches\n";
	std::cout << "   • Check allowed methods\n\n";
	
	std::cout << "4. Testing:\n";
	std::cout << "   • Create test configs in conf_file_example/\n";
	std::cout << "   • Create test CGI scripts in uploads/ directory\n";
	std::cout << "   • Test with curl commands\n\n";
	
	std::cout << "\n========================================\n";
	std::cout << "      BUILD & COMPILATION STATUS\n";
	std::cout << "========================================\n\n";
	
	std::cout << "All components compile successfully with:\n";
	std::cout << "  $ make\n";
	std::cout << "  g++ -Wall -Wextra -Werror -std=c++98 ...\n\n";
	
	std::cout << "The Makefile includes:\n";
	std::cout << "  • All parse_request source files\n";
	std::cout << "  • All parse_config source files\n";
	std::cout << "  • Event loop (loopTools.cpp)\n";
	std::cout << "  • Proper header dependencies\n\n";
}

int main()
{
	demonstrateArchitecture();
	return 0;
}
