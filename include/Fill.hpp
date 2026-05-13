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
#include "../tokenz/parse.hpp"

enum state { LOCATION, SERVER };

typedef struct
{
    std::string path; // location /path/
    std::set<std::string> methods;// POST/GET/DELETE
    std::pair<int, std::string> http_redire; // return direct
    std::string root; // path where exist ure site's files
    bool autoindex; // on/off
    std::vector<std::string> index; // index index.php index.html;
    std::string upload_store; // path where the client post smth
	std::map<int, std::string> error_page;
    size_t body_size;
    std::string cgi_extension; // script extension: .php/.py/.pl
    std::string cgi_pass; // the executer that will run the script
}   locationConf;

typedef struct serverConf
{
    std::vector<std::pair<std::string, int> > listen; // default localhost:80
    std::map<int, std::string> error_page;
    size_t body_size;
	std::string root; // path where exist ure site's files
    bool autoindex; // on/off
    std::vector<std::string> index; // index index.php index.html;
    serverConf()
    {
        listen.push_bach(std::make_pair("0.0.0.0", 80));
    };
    std::vector<locationConf> locations;
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
		~Fill();
};

