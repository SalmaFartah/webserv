#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "../inc/HttpRequest.hpp"
#include "../../parse_config/inc/Fill.hpp"
#include "CGIExecutor.hpp"
#include <map>
#include <string>
#include <vector>



class CGIHandler {
public:
    bool error;
    
    CGIHandler() : error(false) {}
    
    std::string handleCGIRequest(
        const ReqContent& request,
        const serverConf& server,
        const locationConf& location,
        bool keepAlive
    );
    
    bool isCGIRequest(const std::string& requestTarget, const locationConf& loc);
    std::string getFileExtension(const std::string& filename);
    
    std::map<std::string, std::string> buildCGIEnv(
        const ReqContent& request,
        const locationConf& loc,
        const serverConf& srv,
        const std::string& scriptPath
    );
    
    std::string buildCGIResponse(const CGIExecutor::CGIResult& result, bool keepAlive);
    std::string buildErrorResponse(int code, const std::string& message, bool keepAlive);
    std::string getStatusText(int code);
};
#endif




