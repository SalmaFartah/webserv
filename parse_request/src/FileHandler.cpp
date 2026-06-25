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


bool FileHandler::isUploadDirValid(const std::string& uploadDir)
{
    DIR* dir = opendir(uploadDir.c_str());
    if (!dir)
    {
        if (mkdir(uploadDir.c_str(), 0755) == -1)
        {
            std::cerr << "Error creating upload directory: " << strerror(errno) << std::endl;
            return false;
        }
    }
    else
        closedir(dir);
    
    if (access(uploadDir.c_str(), W_OK) == -1)
    {
        std::cerr << "Upload directory is not writable: " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

std::string FileHandler::generateFilename()
{
    static unsigned int counter = 0;
    std::stringstream ss;
    ss << "upload_" << time(NULL) << "_" << (counter++);
    return ss.str();
}

int FileHandler::handleUpload(const ReqContent& request, const locationConf& loc)
{
    if (request.method != "POST")
        return 405;
    
    if (request.body.empty())
        return 400;
    
    
    size_t bodySize = request.body.size();
    if (loc.body_size > 0 && bodySize > loc.body_size)
        return 413;
    
    if (loc.upload_store.empty())
        return 403;
    
    if (!isUploadDirValid(loc.upload_store))
        return 403;
    
    std::string filename = generateFilename();
    std::string filepath = loc.upload_store;
    
    if (filepath[filepath.length() - 1] != '/')
        filepath += "/";
    filepath += filename;
    
    std::ofstream outFile(filepath.c_str(), std::ios::binary);
    if (!outFile.is_open())
    {
        std::cerr << "Failed to create upload file: " << strerror(errno) << std::endl;
        return 500;
    }
    
    outFile.write(request.body.c_str(), request.body.length());
    if (!outFile.good())
    {
        std::cerr << "Error writing to upload file: " << strerror(errno) << std::endl;
        outFile.close();
        return 500;
    }
    
    outFile.close();
    
    std::cout << "File uploaded successfully to: " << filepath << std::endl;
    return 201;
}