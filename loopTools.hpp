#pragma once
#include <vector>
#include "tokenz/parse.hpp"
#include <iostream>
#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>


// struct myclients
// {
//     std::vector<std::string> clieFiles;
//     std::string tmpRead;
// };

class loopTools
{
    private:
        // myclients allCli;
        int servsock;
        std::vector<struct pollfd> vecFds;
    public:
        loopTools();
        // void inite_server();
        std::vector<struct pollfd> getFds() const;
        void mainLoop();
        void newConnection();
        void existClient(int i);
        ~loopTools();
};


