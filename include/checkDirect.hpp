#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
// regex is used to check if a piece of text matches a specific format or rule.
enum tokenType
{
    WORD,
    OPEND_BC,
    CLOSED_BC,
    SEMI_COL
};

typedef struct
{
    std::string path; // location /path/
    std::vector<std::string> methods;// POST/GET/DELETE
    std::pair<int, std::string> http_redire; // return direct
    std::string root; // path where exist ure site's files
    bool autoindex; // on/off
    std::vector<std::string> index; // index index.php index.html;
    std::string upload_store; // path where the client post smth
    std::string cgi_extension; // script extension: .php/.py/.pl
    std::string cgi_pass; // the executer that will run the script
}   locationConf;

typedef struct
{
    std::string serverName;
    std::vector<std::pair<std::string, int>> listen; // default localhost:80
    std::string error_page;
    size_t max_body_sz;
    std::vector<locationConf> locations;
}   serverConf;


class FillServer
{
    private:
        void ListenHandler(std::vector<std::string>);
        bool str_digit(std::string str);
        void ServerNmHandler(std::vector<std::string>);
        void ErrPgHandler(std::vector<std::string>);
        void BodySzHandler(std::vector<std::string>);
        std::string Directives[4]; // will store all server directives each one in index
        void (FillServer::*caller[4])( std::vector<std::string> ); // the array that will store the pointers to functions
    public:
        FillServer();
        serverConf server;
        void fillServer(std::vector<std::pair<tokenType, std::string>>, int&);
        ~FillServer();
};
