#include "../inc/CGIHandler.hpp"
#include "../inc/CGIExecutor.hpp"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <cctype>


std::string CGIHandler::getFileExtension(const std::string& filename)
{
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos || dotPos == filename.length() - 1)
        return "";
    return filename.substr(dotPos);
}


bool CGIHandler::isCGIRequest(const std::string& requestTarget, const locationConf& loc)
{
    if (loc.cgi_extension.empty())
        return false;
    
    size_t queryPos = requestTarget.find('?');
    std::string path = (queryPos != std::string::npos) 
                        ? requestTarget.substr(0, queryPos) 
                        : requestTarget;
    
    std::string ext = getFileExtension(path);
    if (ext.empty())
        return false;
    
    std::string extLower = ext;
    std::string configLower = loc.cgi_extension;
    std::transform(extLower.begin(), extLower.end(), extLower.begin(), ::tolower);
    std::transform(configLower.begin(), configLower.end(), configLower.begin(), ::tolower);
    
    return extLower == configLower;
}



std::map<std::string, std::string> CGIHandler::buildCGIEnv(
    const ReqContent& request,
    const locationConf& loc,
    const serverConf& srv,
    const std::string& scriptPath
) {
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
    if (it != request.headers.end() && !it->second.empty()) {
        env["CONTENT_TYPE"] = it->second;
    }
    
    if (!srv.listen.empty()) {
        env["SERVER_NAME"] = srv.listen[0].first;
        std::stringstream portStr;
        portStr << srv.listen[0].second;
        env["SERVER_PORT"] = portStr.str();
    }
    
    it = request.headers.find("host");
    if (it != request.headers.end() && !it->second.empty()) {
        env["HTTP_HOST"] = it->second;
    }
    
    for (std::map<std::string, std::string>::const_iterator it2 = request.headers.begin();
         it2 != request.headers.end(); ++it2) {
        
        const std::string& key = it2->first;
        if (key == "content-type" || key == "content-length" || key == "host")
            continue;
        
        std::string envKey = "HTTP_";
        for (size_t i = 0; i < key.size(); ++i) {
            char c = key[i];
            envKey.push_back(c == '-' ? '_' : std::toupper(c));
        }
        env[envKey] = it2->second;
    }
    
    return env;
}


std::string CGIHandler::buildErrorResponse(int code, const std::string& message, bool keepAlive)
{
    std::stringstream codeStr;
    codeStr << code;
    
    std::string body = "<html><body><h1>Error " + codeStr.str() + "</h1>";
    body += "<p>" + message + "</p></body></html>";
    
    std::stringstream bodyLen;
    bodyLen << body.size();
    
    std::string response = "HTTP/1.1 " + codeStr.str() + " " + getStatusText(code) + "\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + bodyLen.str() + "\r\n";
    
    if (keepAlive) {
        response += "Connection: keep-alive\r\n";
    } else {
        response += "Connection: close\r\n";
    }
    
    response += "\r\n";
    response += body;
    
    return response;
}

std::string CGIHandler::buildCGIResponse(const CGIExecutor::CGIResult& result, bool keepAlive)
{
    if (result.statusCode != 200) {
        error = true;
        return buildErrorResponse(500, result.error, keepAlive);
    }
    
    const std::string& output = result.output;
    
    if (output.find("HTTP/") == 0 || output.find("Status:") == 0) {
        std::string response = output;
        if (!keepAlive) {
            size_t pos = response.find("Connection: keep-alive");
            if (pos != std::string::npos) {
                response.replace(pos, 22, "Connection: close");
            }
        }
        return response;
    }
    
    std::string contentType = "text/html";
    std::string body = output;
    
    size_t pos = output.find("Content-Type:");
    if (pos != std::string::npos) {
        size_t endLine = output.find("\r\n", pos);
        if (endLine != std::string::npos) {
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
    
    std::stringstream ss;
    ss << body.size();
    
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + ss.str() + "\r\n";
    
    if (keepAlive) {
        response += "Connection: keep-alive\r\n";
    } else {
        response += "Connection: close\r\n";
    }
    
    response += "\r\n";
    response += body;
    
    return response;
}
std::string CGIHandler::handleCGIRequest(
    const ReqContent& request,
    const serverConf& server,
    const locationConf& location,
    bool keepAlive
) {
    error = false;
    
    const std::string& path = request.request_target;
    
    const locationConf* loc = &location;
    

    if (!isCGIRequest(path, *loc)) {
        error = true;
        return buildErrorResponse(400, "Bad Request: Not a CGI request", keepAlive);
    }
    
    if (loc->cgi_pass.empty()) {
        error = true;
        return buildErrorResponse(500, "No cgi_pass configured", keepAlive);
    }
    
    std::string scriptPath = loc->root.empty() ? server.root : loc->root;
    if (scriptPath[scriptPath.size() - 1] != '/')
        scriptPath += '/';
    
    std::string relativePath = path.substr(loc->path.length());
    size_t queryPos = relativePath.find('?');
    if (queryPos != std::string::npos)
        relativePath = relativePath.substr(0, queryPos);
    scriptPath += relativePath;
    
    struct stat st;
    if (stat(scriptPath.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) {
        error = true;
        return buildErrorResponse(404, "CGI script not found", keepAlive);
    }
    
    std::map<std::string, std::string> envVars = buildCGIEnv(request, *loc, server, scriptPath);
    
    CGIExecutor executor;
    CGIExecutor::CGIResult result = executor.executeCGI(
        scriptPath, 
        loc->cgi_pass, 
        envVars, 
        request.body, 
        30
    );
    
    return buildCGIResponse(result, keepAlive);
}


std::string CGIHandler::getStatusText(int code)
{
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 206: return "Partial Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        case 416: return "Range Not Satisfiable";
        case 500: return "Internal Server Error";
        case 504: return "Gateway Timeout";
        default: return "Unknown";
    }
}