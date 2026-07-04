#include "../inc/FileHandler.hpp"

bool FileHandler::isUploadDirValid(const std::string& uploadDir)
{
    struct stat st;
    if (stat(uploadDir.c_str(), &st) != 0) 
    {
        if (mkdir(uploadDir.c_str(), 0755) == -1)
        {
            std::cerr << "Error creating upload directory: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }
    if (S_ISDIR(st.st_mode) && (access(uploadDir.c_str(), W_OK) == 0))
        return true;
    return false;
}

std::string FileHandler::generateFilename()
{
    static unsigned int counter = 0;
    std::stringstream ss;
    ss << "upload_" << time(NULL) << "_" << (counter++);
    return ss.str();
}

std::string FileHandler::handleUpload(ReqContent& request, serverConf& server, locationConf& loc, bool keepAlive)
{
    std::map<std::string, std::string> uploadMap;
    std::map<std::string, std::string>::iterator it;
    error = false;
    HttpResponse responseBuilder;
    
    if (request.body.empty()) 
    {
        error = true;
        return responseBuilder.error_response(server, loc, 400);
    }
    
  
    if (!isUploadDirValid(loc.upload_store)) 
    {
        error = true;
        return responseBuilder.error_response(server, loc, 403);
    }
    // folder is created or exist

    std::string direPath = loc.upload_store;
    std::ofstream outFile;

    if (direPath[direPath.size() - 1] != '/')
        direPath += '/';

    for (it = uploadMap.begin(); it != uploadMap.end(); it++)
    {
        direPath += it->first.c_str();
        outFile.open(direPath, std::ios::binary | std::ios::trunc);
        if (!outFile.is_open())
        {
            error = true;
            return responseBuilder.error_response(server, loc, 500);
        }
        outFile << it->second;
        std::cout << "File uploaded successfully to: " << direPath << std::endl;
    }
    if (!uploadMap.size())
    {
        direPath += generateFilename();
        outFile.open(direPath, std::ios::binary | std::ios::trunc);
        if (!outFile.is_open())
        {
            error = true;
            return responseBuilder.error_response(server, loc, 500);
        }
        outFile << request.body;
        std::cout << "File uploaded successfully to: " << direPath << std::endl;
    }

    return responseBuilder.build(201, "<html><body><h1>201 Created</h1><p>File uploaded successfully</p></body></html>", "text/html", keepAlive);
}