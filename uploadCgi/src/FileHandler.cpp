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

    std::string filepath = loc.upload_store;
    if (filepath[filepath.size() - 1] != '/')
        filepath += '/';
    filepath += generateFilename();
    // file is created

    std::ofstream outFile(filepath.c_str(), std::ios::binary);
    /* check it later*/
    if (!outFile.is_open())
    {
        error = true;
        return responseBuilder.error_response(server, loc, 500);
    }
    /**/
    outFile << request.body;
    
    std::cout << "File uploaded successfully to: " << filepath << std::endl;
    return responseBuilder.build(201, "<html><body><h1>201 Created</h1><p>File uploaded successfully</p></body></html>", "text/html", keepAlive);
}