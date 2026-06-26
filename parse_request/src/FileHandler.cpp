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

std::string FileHandler::handleUpload(
    const ReqContent& request,
    const locationConf& loc,
    const serverConf& server,
    bool keepAlive
) {
    error = false;
    

    HttpResponse responseBuilder;
    
  
    
    if (request.body.empty()) {
        error = true;
        return responseBuilder.error_response(
            const_cast<serverConf&>(server),
            const_cast<locationConf&>(loc),
            400
        );
    }
    
    if (loc.body_size > 0 && request.body.size() > loc.body_size) {
        error = true;
        return responseBuilder.error_response(
            const_cast<serverConf&>(server),
            const_cast<locationConf&>(loc),
            413
        );
    }
    
    if (loc.upload_store.empty()) {
        error = true;
        return responseBuilder.error_response(
            const_cast<serverConf&>(server),
            const_cast<locationConf&>(loc),
            403
        );
    }
    
    if (!isUploadDirValid(loc.upload_store)) {
        error = true;
        return responseBuilder.error_response(
            const_cast<serverConf&>(server),
            const_cast<locationConf&>(loc),
            403
        );
    }
    

    std::string filepath = loc.upload_store;
    if (filepath[filepath.size() - 1] != '/')
        filepath += '/';
    filepath += generateFilename();
    
    std::ofstream outFile(filepath.c_str(), std::ios::binary);
    if (!outFile.is_open()) {
        error = true;
        return responseBuilder.error_response(
            const_cast<serverConf&>(server),
            const_cast<locationConf&>(loc),
            500
        );
    }
    
    outFile.write(request.body.data(), request.body.size());
    if (!outFile.good()) {
        error = true;
        outFile.close();
        return responseBuilder.error_response(
            const_cast<serverConf&>(server),
            const_cast<locationConf&>(loc),
            500
        );
    }
    outFile.close();
    
    std::cout << "File uploaded successfully to: " << filepath << std::endl;
    
    return responseBuilder.build(201, 
                                  "<html><body><h1>201 Created</h1><p>File uploaded successfully</p></body></html>", 
                                  "text/html", 
                                  keepAlive);
}