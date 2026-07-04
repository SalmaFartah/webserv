#pragma once
#include <vector>
#include <cstring>
#include <signal.h> 
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
#include "../parse_config/inc/parse.hpp"
#include "../parse_config/inc/FillLocation.hpp"
#include "../parse_config/inc/FillServer.hpp"
#include "../parse_request/inc/HttpRequest.hpp"
#include "../route/RouteResp.hpp"
#define BUFFER_SZ 1000

struct myclients
{
    int         info_fd;
    size_t      ofssetResp;
    std::string clieFile;
    serverConf  *cliConf;
    std::time_t clieTime;
    HttpRequest request;
    std::string resp;
};


class loopTools
{
    private:
        RouteResp realResp;
        bool isconnected;
        std::vector<struct myclients> infoClie;
        std::map<int, struct CGIResult> cgiMap;
        std::vector<struct pollfd> vecFds;
        size_t serv_nb;
        std::map<int, serverConf*> linkServConf;
        void addNewFd(int fd, short event);
        void eraseChild(CGIResult& Childinfo, int code);
    public:
        loopTools();
        loopTools(std::vector<serverConf>& servers);
        void mainLoop();
        void newConnection(struct pollfd& server);
        bool existClient(struct pollfd& client, int clieIdx, size_t *idx);
        void close_fds();
        void closeClient(int fd, int clieIdx, size_t *i);
        bool CgiWrite(CGIResult& cgiWr, size_t &i);
        bool CgiRead(CGIResult& cgiRd, size_t &i);
        int findClient(int fd);
        int findInVec(int fd);
        void CgiTimout();
        void shutdownCGI();
        ~loopTools();
};


