#pragma once
#include "Fill.hpp"



class FillLocation : public Fill
{
    private:
        std::string Directives[8]; // will store all server directives each one in index
        void (FillLocation::*caller[8])( std::vector<std::string> ); // the array that will store the pointers to functions
        void methodsHandler( std::vector<std::string> );
        void returnHandler( std::vector<std::string> );
        void uploadHandler( std::vector<std::string> );
        void cgiPassHandler( std::vector<std::string> );
        void cgiExtHandler( std::vector<std::string> );

    public:
        FillLocation();
        void LocationFiller(std::vector<std::pair<tokenType, std::string> >, size_t&);
        ~FillLocation();
};
