#pragma once
#include "../parse_config/inc/Fill.hpp"
#include "../build_response/HttpResponse.hpp"

#include <sys/stat.h>

class RouteResp
{
    private:
        std::string winnerPath;
        size_t      winnerIdx;
        std::string finalPath;
        std::string response;
        HttpResponse respObj;
    public:
        RouteResp();
        ~RouteResp();
        int locationMatcha(serverConf *conf, ReqContent& cont, int code);
        int routeCheck(serverConf *conf, ReqContent& cont, int code);
        // void checkMethods()
        const std::string& getResponse() const;
        int checkDire(std::string &fullPath, serverConf *conf, locationConf& location, struct stat *st);
        int transLower(std::string& strPath, std::string& strExt, size_t posDot);
};


