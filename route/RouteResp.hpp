#pragma once
#include "../parse_config/inc/Fill.hpp"

class RouteResp
{
    private:
        std::string winnerPath;
        size_t      winnerIdx;
    public:
        RouteResp();
        ~RouteResp();
        void locationMatcha(serverConf *conf, ReqContent reqCon);
        void routeCheck(serverConf *conf, ReqContent reqCon);
        // void checkMethods()
};


