#include "../inc/CGIHandler.hpp"
#include "../inc/CGIExecutor.hpp"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>




std::string CGIHandler::getFileExtension(const std::string& filename)
{
    size_t dotPos = filename.find_last_of(".");
    if (dotPos == std::string::npos || dotPos == filename.length() - 1)
        return "";
    return filename.substr(dotPos);
}



bool CGIHandler::isCGIRequest(const std::string& requestTarget, const locationConf& loc)
{

    if (loc.cgi_extension.empty())
        return false;
    
    std::string path = requestTarget;
    size_t queryPos = path.find("?");
    if (queryPos != std::string::npos)
        path = path.substr(0, queryPos);
    
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
        
        if (it2->first == "content-type" || it2->first == "content-length" || it2->first == "host") {
            continue;
        }
        
        std::string headerKey = "HTTP_" + it2->first;
        std::transform(headerKey.begin(), headerKey.end(), headerKey.begin(), ::toupper);
        for (size_t i = 0; i < headerKey.length(); ++i) {
            if (headerKey[i] == '-')
                headerKey[i] = '_';
        }
        env[headerKey] = it2->second;
    }
    
    return env;
}


std::string CGIHandler::handleCGIRequest(
    const ReqContent& request,
    const serverConf& server
) {
    std::string path = request.request_target;
    
    std::cout << "CGIHandler: Processing " << path << std::endl;
    
    const locationConf* loc = NULL;
    size_t bestMatchLen = 0;
    
    for (size_t i = 0; i < server.locations.size(); ++i) {
        const locationConf& l = server.locations[i];
        if (path.find(l.path) == 0 && l.path.length() > bestMatchLen) {
            loc = &l;
            bestMatchLen = l.path.length();
        }
    }
    
    if (!loc) {
        return buildErrorResponse(404, "Not Found");
    }
    
    std::cout << "Location: " << loc->path << std::endl;
    std::cout << "CGI Extension: [" << loc->cgi_extension << "]" << std::endl;
    std::cout << "CGI Pass: [" << loc->cgi_pass << "]" << std::endl;
    

    if (!isCGIRequest(path, *loc)) {
        return buildErrorResponse(400, "Bad Request: Not a CGI request");
    }
  
    std::string interpreter = loc->cgi_pass;
    if (interpreter.empty()) {
        return buildErrorResponse(500, "No cgi_pass configured");
    }
    
    std::string scriptPath;
    if (!loc->root.empty()) {
        scriptPath = loc->root;
    } else {
        scriptPath = server.root;
    }
    
    std::string relativePath = path;
    if (relativePath.find(loc->path) == 0) {
        relativePath = relativePath.substr(loc->path.length());
    }
    
    if (!scriptPath.empty() && scriptPath[scriptPath.length() - 1] != '/') {
        scriptPath += '/';
    }
    
    size_t queryPos = relativePath.find('?');
    if (queryPos != std::string::npos) {
        relativePath = relativePath.substr(0, queryPos);
    }
    
    scriptPath += relativePath;
    std::cout << "Script path: " << scriptPath << std::endl;
    

    struct stat st;
    if (stat(scriptPath.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) {
        return buildErrorResponse(404, "CGI script not found");
    }
    
    std::map<std::string, std::string> envVars = buildCGIEnv(request, *loc, server, scriptPath);
    
   
    CGIExecutor executor;
    CGIExecutor::CGIResult result = executor.executeCGI(
        scriptPath,
        interpreter,
        envVars,
        request.body,  
        30
    );
    
    return buildCGIResponse(result);
}

std::string CGIHandler::buildCGIResponse(const CGIExecutor::CGIResult& result) {
    if (result.statusCode != 200) {
        return buildErrorResponse(500, result.error);
    }
    
    std::string output = result.output;
    
    if (output.find("HTTP/") == 0 || output.find("Status:") == 0) {
        return output;
    }
    
    std::string contentType = "text/html";
    std::string body = output;
    
    size_t contentTypePos = output.find("Content-Type:");
    if (contentTypePos != std::string::npos) {
        size_t endLine = output.find("\r\n", contentTypePos);
        if (endLine != std::string::npos) {
            contentType = output.substr(contentTypePos + 14, endLine - contentTypePos - 14);
            size_t start = contentType.find_first_not_of(" \t");
            if (start != std::string::npos) {
                contentType = contentType.substr(start);
                size_t end = contentType.find_last_not_of(" \t");
                if (end != std::string::npos) {
                    contentType = contentType.substr(0, end + 1);
                }
            }
            body = output.substr(endLine + 2);
        }
    }
    
    std::stringstream ss;
    ss << body.length();
    
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + ss.str() + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += body;
    
    return response;
}

std::string CGIHandler::buildErrorResponse(int code, const std::string& message) {
    std::stringstream codeStr;
    codeStr << code;
    
    std::string body = "<html><body><h1>Error " + codeStr.str() + "</h1>";
    body += "<p>" + message + "</p></body></html>";
    
    std::stringstream bodyLen;
    bodyLen << body.length();
    
    std::string response = "HTTP/1.1 " + codeStr.str() + " " + getStatusText(code) + "\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + bodyLen.str() + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += body;
    
    return response;
}

std::string CGIHandler::getStatusText(int code) {
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