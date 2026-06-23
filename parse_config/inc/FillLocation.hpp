

#ifndef FILLLOCATION_HPP
#define FILLLOCATION_HPP

#include "Fill.hpp"
#include <vector>
#include <string>
#include <map>

class FillLocation : public Fill
{
public:
    locationConf location;

    FillLocation();
    ~FillLocation();

   
    
    void methodsHandler(std::vector<std::string> values, state s);
    void returnHandler(std::vector<std::string> values, state s);
    void rootHandler(std::vector<std::string> values, state s);
    void autoindexHandler(std::vector<std::string> values, state s);
    void indexHandler(std::vector<std::string> values, state s);
    void uploadHandler(std::vector<std::string> values, state s);
    void cgiPassHandler(std::vector<std::string> values, state s);
    void cgiExtHandler(std::vector<std::string> values, state s);
    void BodySzHandler(std::vector<std::string> values, state s);
    void ErrPgHandler(std::vector<std::string> values, state s);
    

    
    void LocationFiller(std::vector<std::pair<tokenType, std::string> > tokens, size_t& pos);
    bool directive(std::string str);

private:
    std::string Directives[10];
    void (FillLocation::*caller[10])(std::vector<std::string>, state);
};

#endif