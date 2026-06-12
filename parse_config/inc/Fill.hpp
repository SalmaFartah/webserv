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

enum state { LOCATION, SERVER };

struct HasPort
{
    int port;
    HasPort(int p) : port(p) {}
    bool operator()(const std::pair<std::string, int>& p) const
    {
        return p.second == port;
    }
};

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
    locationConf() // CHECK THIS LATER
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
        error_page[400] = "<html><body><h1>400 Bad Request</h1></body></html>";
        error_page[403] = "<html><body><h1>403 Forbidden</h1></body></html>";
        error_page[404] = "<html><body><h1>404 Not Found</h1></body></html>";
        error_page[405] = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        error_page[413] = "<html><body><h1>413 Content Too Large</h1></body></html>";
        error_page[500] = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        error_page[501] = "<html><body><h1>501 Not Implemented</h1></body></html>";
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