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
    
    std::string getFileExtension(const std::string& filename);
    bool isCGIRequest(const std::string& requestTarget, const locationConf& loc);

    std::map<std::string, std::string> buildCGIEnv(
        const ReqContent& request,
        const locationConf& loc,
        const serverConf& srv,
        const std::string& scriptPath
    );
    

    bool isValidExtension(const std::string& ext);
    const locationConf* findMatchingLocation(const std::string& path, const serverConf& server);
    std::string getScriptPath(const std::string& requestTarget, const locationConf& loc, const serverConf& server);
    bool scriptExists(const std::string& scriptPath);
    
    std::string getInterpreter(const std::string& scriptPath, const locationConf& loc);
    
    std::string buildCGIResponse(const CGIExecutor::CGIResult& result);
    std::string buildErrorResponse(int code, const std::string& message);
    std::string getStatusText(int code);
};

#endif
