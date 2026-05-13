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
#include "Fill.hpp"

typedef struct
{
    std::vector<std::pair<std::string, int> > listen; // default localhost:80
    std::map<int, std::string> error_page;
    size_t body_size;
    std::vector<locationConf> locations;
}   serverConf;

typedef struct
{
    std::string path; // location /path/
    std::set<std::string> methods;// POST/GET/DELETE
    std::pair<int, std::string> http_redire; // return direct
    std::string root; // path where exist ure site's files
    bool autoindex; // on/off
    std::vector<std::string> index; // index index.php index.html;
    std::string upload_store; // path where the client post smth
    std::string cgi_extension; // script extension: .php/.py/.pl
    std::string cgi_pass; // the executer that will run the script
}   locationConf;

class Fill
{
	private:
		bool str_digit(std::string str);
		bool valid_path(std::string path);
		void rootHandler( std::vector<std::string> );
		void autoindexHandler( std::vector<std::string> );
		void indexHandler( std::vector<std::string> );
		void ErrPgHandler(std::vector<std::string>);
        void BodySzHandler(std::vector<std::string>);
	public:
		Fill();
	    serverConf server;
		locationConf location;
		~Fill();
};

Fill::Fill()
{
}

Fill::~Fill()
{
}
