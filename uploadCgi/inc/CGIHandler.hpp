#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "../../parse_config/inc/Fill.hpp"
#include "../../build_response/HttpResponse.hpp"
#include "CGIExecutor.hpp"
#include <map>
#include <string>
#include <vector>
#include "../inc/CGIExecutor.hpp"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <cctype>

class CGIHandler 
{
public:
    bool error;
    
    CGIHandler() : error(false) {}
    HttpResponse responseBuilder;;
    CGIExecutor executor;
    std::string handleCGIRequest(ReqContent& request, serverConf& server, locationConf& location, bool keepAlive);
    

    std::map<std::string, std::string> buildCGIEnv(ReqContent& request, locationConf& loc, serverConf& srv, std::string& scriptPath);
    

    std::string buildCGIResponse(const CGIExecutor::CGIResult& result, bool keepAlive);
};


#endif