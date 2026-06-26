#pragma once
#include <string>
#include <map>
#include "../inc/HttpRequest.hpp"
#include "../../parse_config/inc/Fill.hpp"
#include "../../build_response/HttpResponse.hpp"


class FileHandler {
public:
    bool error;
    
    FileHandler() : error(false) {}
    
    std::string handleUpload(
        const ReqContent& request,
        const locationConf& loc,
        const serverConf& server,
        bool keepAlive
    );
    
    bool isUploadDirValid(const std::string& uploadDir);
    std::string generateFilename();

private:

    std::string buildErrorResponse(int code, const serverConf& server, const locationConf& location, bool keepAlive);
    std::string buildSuccessResponse(int code, const std::string& message, bool keepAlive);
};
