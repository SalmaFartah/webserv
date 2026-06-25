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
  
    std::string handleCGIRequest(const ReqContent& request, const serverConf& server);
    

    bool isCGIRequest(const std::string& requestTarget, const locationConf& loc);
    
    std::string getFileExtension(const std::string& filename);
    
    std::map<std::string, std::string> buildCGIEnv(
        const ReqContent& request,
        const locationConf& loc,
        const serverConf& srv,
        const std::string& scriptPath
    );
    
    std::string buildCGIResponse(const CGIExecutor::CGIResult& result);
    std::string buildErrorResponse(int code, const std::string& message);
    std::string getStatusText(int code);
};

#endif




