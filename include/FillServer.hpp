#pragma once
#include "Fill.hpp"

class FillServer : public Fill
{
    private:
        void ListenHandler(std::vector<std::string>);
        bool valid_ip(std::string);
        std::string resolveHost();
        std::string Directives[3]; // will store all server directives each one in index
        void (FillServer::*caller[3])( std::vector<std::string> ); // the array that will store the pointers to functions
    public:
        FillServer();
        void fillServer(std::vector<std::pair<tokenType, std::string> >, size_t&);
        ~FillServer();
};
