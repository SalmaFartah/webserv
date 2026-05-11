#pragma once

#include "FillLocation.hpp"
// regex is used to check if a piece of text matches a specific format or rule.



typedef struct
{
    std::vector<std::pair<std::string, int> > listen; // default localhost:80
    std::map<int, std::string> error_page;
    size_t body_size;
    std::vector<locationConf> locations;
}   serverConf;

class FillServer
{
    private:
        void ListenHandler(std::vector<std::string>);
        bool valid_ip(std::string);
        void ErrPgHandler(std::vector<std::string>);
        void BodySzHandler(std::vector<std::string>);
        std::string Directives[3]; // will store all server directives each one in index
        void (FillServer::*caller[3])( std::vector<std::string> ); // the array that will store the pointers to functions
    public:
        FillServer();
        serverConf server;
        void fillServer(std::vector<std::pair<tokenType, std::string> >, size_t&);
        ~FillServer();
};
