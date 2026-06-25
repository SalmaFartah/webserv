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

int FileHandler::handleUpload(
    const ReqContent& request,
    const locationConf& loc
) {
    if (request.method != "POST")
        return 405;
    
    if (request.body.empty())
        return 400;
    
    if (loc.body_size > 0 && request.body.size() > loc.body_size)
        return 413;
    
    if (loc.upload_store.empty())
        return 403;
    
    if (!isUploadDirValid(loc.upload_store))
        return 403;
    
    std::string filepath = loc.upload_store;
    if (filepath[filepath.size() - 1] != '/')
        filepath += '/';
    
    filepath += generateFilename();
    
    std::ofstream outFile(filepath.c_str(), std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Failed to create upload file: " << strerror(errno) << std::endl;
        return 500;
    }
    
    outFile.write(request.body.data(), request.body.size());
    if (!outFile.good()) {
        std::cerr << "Error writing to upload file: " << strerror(errno) << std::endl;
        outFile.close();
        return 500;
    }
    outFile.close();
    
    std::cout << "File uploaded successfully to: " << filepath << std::endl;
    return 201;
}