#pragma once
#include <vector>
#include <iostream>
#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <stdbool.h>

#define BUFFER_SZ 200

struct myclients
{
    std::string clieFile;
};

class loopTools
{
    private:
        std::vector<struct myclients> infoClie;
        int servsock;
        std::vector<struct pollfd> vecFds;
        bool ctlen;
    public:
        loopTools();
        // void inite_server();
        std::vector<struct pollfd> getFds() const;
        void mainLoop();
        void newConnection();
        void existClient(int i);
        ~loopTools();
};


