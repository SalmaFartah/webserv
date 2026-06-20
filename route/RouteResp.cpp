#include "RouteResp.hpp"

RouteResp::RouteResp() : winnerIdx(0) {}

RouteResp::~RouteResp(){}

void RouteResp::locationMatcha(serverConf *conf, ReqContent cont)
{
    for (size_t i = 0; i < conf->locations.size(); i++)
    {
        if (cont.request_target.find(conf->locations[i].path) == 0 && conf->locations[i].path.size() > winnerPath.size())
        {
            winnerPath = conf->locations[i].path;
            winnerIdx = i;
        }
    }
    if (winnerPath.empty())
        std::cout << "location error i must call response 404: " << winnerPath << "\n";
    else
        std::cout << "test matcha location: " << winnerPath << "\n";
    
}

void RouteResp::routeCheck(serverConf *conf, ReqContent cont)
{
    
    locationMatcha(conf, cont);

    /* CHECK METHODS*/
    if (!conf->locations[winnerIdx].methods.count(cont.method))
        std::cout << "method error i must call response 405 Method Not Allowed: " << winnerIdx << "\n";
    else
        std::cout << "method found it is: " << cont.method << "\n";
    /* BODY SIZE */
    // if (cont.)
}