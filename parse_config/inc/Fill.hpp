#pragma once
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <set>
#include <netdb.h>
#include <arpa/inet.h>
#include <algorithm>
#include "parse.hpp"
#define MAX_CGI_PROCESSES 10

enum state { LOCATION, SERVER };
typedef struct CGIResult
{
    int         statusCode;
    pid_t       pidChild;
    size_t      clie_fd;    // ← the client waiting for response
    int         stdinPipe;
    int         stdoutPipe;
    std::string output;       // accumulate response
    std::string body;         // body to write to script
    size_t      ofssetCgi; // how much of body sent so far
    std::time_t start_time;
    CGIResult();
} CGIResult;

struct HasPort
{
    int port;
    HasPort(int p) : port(p) {}
    bool operator()(const std::pair<std::string, int>& p) const
    {
        return p.second == port;
    }
};

typedef struct
{
	std::string method;
	std::string request_target;
	std::string query;
	std::string httpVersion;
	std::map<std::string, std::string> headers;
	std::string body;
    bool connection;
    std::map<std::string, std::string> uploads;
} ReqContent;

typedef struct locationConf
{
    std::string path; // location /path/
    std::set<std::string> methods;// POST/GET/DELETE
    std::pair<int, std::string> http_redire; // return direct
    std::string root; // path where exist ure site's files
    bool autoindex; // on/off
    bool autoindex_set;
    std::vector<std::string> index; // index index.php index.html;
    std::string upload_store; // path where the client post smth
	std::map<int, std::string> error_page;
    size_t body_size;
    bool body_size_set;
    std::string cgi_extension; // script extension: .php/.py/.pl
    std::string cgi_pass; // the executer that will run the script
    bool defaultm;
    locationConf()
    {
        methods.insert("GET");
        methods.insert("POST");
        methods.insert("DELETE");
        autoindex = false;
        autoindex_set = false;
        body_size = 0;
        body_size_set = false;
        defaultm = true;
    };
}   locationConf;

typedef struct serverConf
{
    std::vector<std::pair<std::string, int> > listen; // default localhost:80
    std::map<int, std::string> error_page;
    size_t body_size;
	std::string root; // path where exist ure site's files
    bool autoindex; // on/off
    std::vector<std::string> index; // index index.php index.html;
    std::vector<locationConf> locations;
    bool defaults;
    bool defaulti;
    serverConf()
    {
        defaults = true;
        defaulti = true;
        body_size = 1048576;
        listen.push_back(std::make_pair("0.0.0.0", 80));
        index.push_back("index.html");
        autoindex = false;
    };
}   serverConf;

class Fill
{
	protected:
		bool str_digit(std::string str);
		bool valid_path(std::string path);
		void rootHandler( std::vector<std::string>, state);
		void autoindexHandler( std::vector<std::string>, state);
		void indexHandler( std::vector<std::string>, state);
		void ErrPgHandler(std::vector<std::string>, state);
        void BodySzHandler(std::vector<std::string>, state);
		bool valid_suffix(char c);
	public:
		Fill();
	    serverConf server;
		locationConf location;
		virtual ~Fill();
};

void print_config(std::vector<serverConf> conf);
void checkPortConflict(std::vector<serverConf>);
std::string to_string(int val);