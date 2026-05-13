#pragma once
#include "Fill.hpp"

class FillServer : public Fill
{
    private:
        std::string Directives[6]; // will store all server directives each one in index
        void (FillServer::*caller[6])( std::vector<std::string>, state); // the array that will store the pointers to functions

        void ListenHandler(std::vector<std::string>, state);
        bool valid_ip(std::string);
        std::string resolveHost();
    public:
        FillServer();
        void fillServer(std::vector<std::pair<tokenType, std::string> >, size_t&);
        ~FillServer();
};
