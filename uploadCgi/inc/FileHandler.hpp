#pragma once
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include "../../build_response/HttpResponse.hpp"
#include "../../parse_config/inc/Fill.hpp"


class FileHandler
{
    public:
        bool error;
        FileHandler();
        
        std::string handleUpload(ReqContent& request, serverConf& server, locationConf& loc, bool keepAlive, HttpResponse &responseBuilder);
        
        bool isUploadDirValid(const std::string& uploadDir);
        std::string generateFilename();

};
