#pragma once
#include "Fill.hpp"

class FillLocation : public Fill
{
    private:
        std::string Directives[10]; // will store all server directives each one in index
        void (FillLocation::*caller[10])( std::vector<std::string>, state); // the array that will store the pointers to functions

        bool directive(std::string);
        void methodsHandler( std::vector<std::string>, state);
        void returnHandler( std::vector<std::string>, state);
        void uploadHandler( std::vector<std::string>, state);
        void cgiPassHandler( std::vector<std::string>, state);
        void cgiExtHandler( std::vector<std::string>, state);
    public:
        FillLocation();
        void LocationFiller(std::vector<std::pair<tokenType, std::string> >, size_t&);
        ~FillLocation();
};
