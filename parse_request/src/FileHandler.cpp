#include "../inc/FileHandler.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>


bool FileHandler::isUploadDirValid(const std::string& uploadDir)
{
    struct stat st;
    if (stat(uploadDir.c_str(), &st) != 0) {
        if (mkdir(uploadDir.c_str(), 0755) == -1) {
            std::cerr << "Error creating upload directory: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }
    return S_ISDIR(st.st_mode) && (access(uploadDir.c_str(), W_OK) == 0);
}



std::string FileHandler::generateFilename()
{
    static unsigned int counter = 0;
    std::stringstream ss;
    ss << "upload_" << time(NULL) << "_" << (counter++);
    return ss.str();
}


std::string FileHandler::buildErrorResponse(int code, const serverConf& server, const locationConf& location, bool keepAlive)
{
    
    HttpResponse response;
    std::string result = response.error_response(
        const_cast<serverConf&>(server), 
        const_cast<locationConf&>(location), 
        code
    );
    
    if (!keepAlive) {
        size_t pos = result.find("Connection: keep-alive");
        if (pos != std::string::npos) {
            result.replace(pos, 22, "Connection: close");
        }
    } else {
        size_t pos = result.find("Connection:");
        if (pos == std::string::npos) {
            size_t end = result.find("\r\n\r\n");
            if (end != std::string::npos) {
                result.insert(end, "Connection: keep-alive\r\n");
            }
        }
    }
    
    return result;
}



std::string FileHandler::buildSuccessResponse(int code, const std::string& message, bool keepAlive)
{
    std::stringstream codeStr;
    codeStr << code;
    
    std::string response = "HTTP/1.1 " + codeStr.str();
    if (code == 201) response += " Created";
    else if (code == 200) response += " OK";
    response += "\r\n";
    response += "Content-Type: text/html\r\n";
    

    if (keepAlive) {
        response += "Connection: keep-alive\r\n";
    } else {
        response += "Connection: close\r\n";
    }
    
    std::string body = "<html><body><h1>" + codeStr.str();
    if (code == 201) body += " Created";
    body += "</h1><p>" + message + "</p></body></html>";
    
    std::stringstream lengthStr;
    lengthStr << body.size();
    response += "Content-Length: " + lengthStr.str() + "\r\n";
    response += "\r\n";
    response += body;
    
    return response;
}


std::string FileHandler::handleUpload(
    const ReqContent& request,
    const locationConf& loc,
    const serverConf& server,
    const locationConf& location,
    bool keepAlive
) {
    error = false;
    
  
    if (request.body.empty()) {
        error = true;
        return buildErrorResponse(400, server, location, keepAlive);
    }
    
    if (loc.body_size > 0 && request.body.size() > loc.body_size) {
        error = true;
        return buildErrorResponse(413, server, location, keepAlive);
    }
    
    if (loc.upload_store.empty()) {
        error = true;
        return buildErrorResponse(403, server, location, keepAlive);
    }
    
    if (!isUploadDirValid(loc.upload_store)) {
        error = true;
        return buildErrorResponse(403, server, location, keepAlive);
    }
    
    std::string filepath = loc.upload_store;
    if (filepath[filepath.size() - 1] != '/')
        filepath += '/';
    filepath += generateFilename();
    
    std::ofstream outFile(filepath.c_str(), std::ios::binary);
    if (!outFile.is_open()) {
        error = true;
        return buildErrorResponse(500, server, location, keepAlive);
    }
    
    outFile.write(request.body.data(), request.body.size());
    if (!outFile.good()) {
        error = true;
        outFile.close();
        return buildErrorResponse(500, server, location, keepAlive);
    }
    outFile.close();
    
    std::cout << "File uploaded successfully to: " << filepath << std::endl;
    
    return buildSuccessResponse(201, "File uploaded successfully", keepAlive);
}