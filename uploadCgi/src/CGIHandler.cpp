#include "../inc/CGIHandler.hpp"

std::map<std::string, std::string> CGIHandler::buildCGIEnv(ReqContent& request, locationConf& loc, serverConf& srv, std::string& scriptPath) 
{
    (void)loc;
    std::map<std::string, std::string> env;
    
    env["REQUEST_METHOD"] = request.method;
    env["QUERY_STRING"] = request.query;
    env["REQUEST_URI"] = request.request_target;
    env["PATH_INFO"] = request.request_target;
    env["SCRIPT_FILENAME"] = scriptPath;
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["REDIRECT_STATUS"] = "200";
    
    std::stringstream ss;
    ss << request.body.size();
    env["CONTENT_LENGTH"] = ss.str();
    
    std::map<std::string, std::string>::const_iterator it = request.headers.find("content-type");
    if (it != request.headers.end() && !it->second.empty())
        env["CONTENT_TYPE"] = it->second;

    if (!srv.listen.empty())
    {
        env["SERVER_NAME"] = srv.listen[0].first;
        std::stringstream portStr;
        portStr << srv.listen[0].second;
        env["SERVER_PORT"] = portStr.str();
    }

    it = request.headers.find("host");
    if (it != request.headers.end() && !it->second.empty()) 
        env["HTTP_HOST"] = it->second;

    for (std::map<std::string, std::string>::const_iterator it2 = request.headers.begin(); it2 != request.headers.end(); ++it2)
    {
        const std::string& key = it2->first;
        if (key == "content-type" || key == "content-length" || key == "host")
            continue;
        
        std::string envKey = "HTTP_";
        for (size_t i = 0; i < key.size(); ++i)
        {
            char c = key[i];
            envKey.push_back(c == '-' ? '_' : std::toupper(c));
        }
        env[envKey] = it2->second;
    }
    return env;
}


std::string CGIHandler::buildCGIResponse(const CGIResult& result, bool keepAlive)
{
    const std::string& output = result.output;
    if (output.find("HTTP/") == 0 || output.find("Status:") == 0) 
    {
        std::string response = output;
        if (!keepAlive) 
        {
            size_t pos = response.find("Connection: keep-alive");
            if (pos != std::string::npos)
                response.replace(pos, 22, "Connection: close");
        } 
        else 
        {
            size_t pos = response.find("Connection: close");
            if (pos != std::string::npos) 
                response.replace(pos, 17, "Connection: keep-alive");
        }
        return response;
    }

    std::string contentType = "text/html";
    std::string body = output;
    
    size_t pos = output.find("Content-Type:");
    if (pos != std::string::npos) 
    {
        size_t endLine = output.find("\r\n", pos);
        if (endLine != std::string::npos) 
        {
            std::string rawType = output.substr(pos + 14, endLine - pos - 14);
            size_t start = rawType.find_first_not_of(" \t");
            if (start != std::string::npos) {
                size_t end = rawType.find_last_not_of(" \t");
                if (end != std::string::npos)
                    contentType = rawType.substr(start, end - start + 1);
            }
            body = output.substr(endLine + 2);
        }
    }
    std::cout << "body response: " << body << "\n";
    return responseBuilder.build(200, body, contentType, keepAlive);
}


void CGIHandler::handleCGIRequest(ReqContent& request, serverConf& server, locationConf& location, CGIResult& CgiRes)
{
    // error = false;
        
    /*FORM THE SCRIPT PATH*/
    std::string scriptPath = location.root + request.request_target;
    
    /*HERE WE BUILD THE ENVIRONMENT VARIABLES WE WILL SEND TO PROCESS*/
    std::map<std::string, std::string> envVars = buildCGIEnv(request, location, server, scriptPath);

    /*EXECUTION*/
    executor.executeCGI(scriptPath, location.cgi_pass, envVars, request.body, CgiRes);

    // if (result.statusCode != 200)
    //     return error = true, responseBuilder.error_response(server, location, result.statusCode);
    // return buildCGIResponse(result, keepAlive);
}
