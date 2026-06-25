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
    // ✅ FONCTION PRINCIPALE
    std::string handleCGIRequest(const ReqContent& request, const serverConf& server);
    
    // ✅ DÉTECTION CGI
    bool isCGIRequest(const std::string& requestTarget, const locationConf& loc);
    
    // ✅ UTILITAIRES
    std::string getFileExtension(const std::string& filename);
    
    // ✅ VARIABLES D'ENVIRONNEMENT
    std::map<std::string, std::string> buildCGIEnv(
        const ReqContent& request,
        const locationConf& loc,
        const serverConf& srv,
        const std::string& scriptPath
    );
    
    // ✅ CONSTRUCTION DES RÉPONSES
    std::string buildCGIResponse(const CGIExecutor::CGIResult& result);
    std::string buildErrorResponse(int code, const std::string& message);
    std::string getStatusText(int code);
};

#endif


