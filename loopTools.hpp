#pragma once
#include <vector>
#include <map>
#include <ctime>
#include <iostream>
#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <stdbool.h>
#include "parse_config/inc/parse.hpp"
#include "parse_config/inc/FillLocation.hpp"
#include "parse_config/inc/FillServer.hpp"
#include "parse_request/inc/HttpRequest.hpp"

#define BUFFER_SZ 200

struct myclients
{
    std::string clieFile;
    serverConf  *cliConf;
    std::time_t clieTime;
    HttpRequest request;
};


class loopTools
{
    private:
        std::vector<struct myclients> infoClie;
        std::vector<struct pollfd> vecFds;
        size_t serv_nb;
        std::map<int, serverConf*> linkServConf;
    public:
        loopTools();
        loopTools(std::vector<serverConf> servers);
        void mainLoop();
        void newConnection(struct pollfd& server);
        void existClient(struct pollfd& client, int clieIdx);
        void close_fds();
        void incompleteCase();
        ~loopTools();
};


