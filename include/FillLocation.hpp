#pragma once

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <set>
#include "../tokenz/parse.hpp"

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

bool str_digit(std::string str);
bool valid_path(std::string path);

class FillLocation
{
    private:
        std::string Directives[8]; // will store all server directives each one in index
        void (FillLocation::*caller[8])( std::vector<std::string> ); // the array that will store the pointers to functions
    
        void methodsHandler( std::vector<std::string> );
        void returnHandler( std::vector<std::string> );
        void rootHandler( std::vector<std::string> );
        void autoindexHandler( std::vector<std::string> );
        void indexHandler( std::vector<std::string> );
        void uploadHandler( std::vector<std::string> );
        void cgiPassHandler( std::vector<std::string> );
        void cgiExtHandler( std::vector<std::string> );

    public:
        FillLocation();
        void LocationFiller(std::vector<std::pair<tokenType, std::string> >, size_t&);
        locationConf location;
        ~FillLocation();
};

