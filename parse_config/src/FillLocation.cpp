#include "../inc/FillLocation.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>

FillLocation::FillLocation()
{
    location.defaultm = true;
    Directives[0] = "allowed_method";
    Directives[1] = "return";
    Directives[2] = "root";
    Directives[3] = "autoindex";
    Directives[4] = "index";
    Directives[5] = "upload_store";
    Directives[6] = "cgi_pass";
    Directives[7] = "cgi_extension";
    Directives[8] = "client_max_body_size";
    Directives[9] = "error_page";

    caller[0] = &FillLocation::methodsHandler;
    caller[1] = &FillLocation::returnHandler;
    caller[2] = &FillLocation::rootHandler;
    caller[3] = &FillLocation::autoindexHandler;
    caller[4] = &FillLocation::indexHandler;
    caller[5] = &FillLocation::uploadHandler;
    caller[6] = &FillLocation::cgiPassHandler;
    caller[7] = &FillLocation::cgiExtHandler;
    caller[8] = &FillLocation::BodySzHandler;
    caller[9] = &FillLocation::ErrPgHandler;
}

FillLocation::~FillLocation() {}


void FillLocation::methodsHandler(std::vector<std::string> values, state)
{
    if (location.defaultm)
    {
        location.methods.clear();
        location.defaultm = false;
    }
    if (!values.size())
        throw std::logic_error("Error: allowed_method: missing value.");
    std::pair<std::set<std::string>::iterator, bool> result;
    for (size_t i = 0; i < values.size(); i++)
    {
        if (values[i] == "GET")
            result = location.methods.insert("GET");
        else if (values[i] == "POST")
            result = location.methods.insert("POST");
        else if (values[i] == "DELETE")
            result = location.methods.insert("DELETE");
        else
            throw std::logic_error("Error: allowed_method: invalid method: `" + values[i] + "'");
        if (!result.second)
            throw std::logic_error("Error: allowed_method: duplicate method: `" + values[i] + "'");
    }
}

void FillLocation::returnHandler(std::vector<std::string> values, state)
{
    if (values.size() > 2)
        throw std::logic_error("Error: return: too many values.");
    if (values.size() < 2)
        throw std::logic_error("Error: return: missing value.");

    std::string status_code = values[0];
    if (!str_digit(status_code))
        throw std::logic_error("Error: return: invalid status code: `" + status_code + "'");

    char *end = NULL;
    int st_code = std::strtol(status_code.c_str(), &end, 10);
    if (status_code[0] == '0' || errno == ERANGE || (st_code != 301 && st_code != 302 \
    && st_code != 303 && st_code != 307 && st_code != 308))
        throw std::logic_error("Error: return: invalid status code: `" + status_code + "'");

    std::string url = values[1];
    if (url.find_first_of("http://") && url.find_first_of("https://") && url[0] != '/')
        throw std::logic_error("Error: return: invalid url: `" + url + "'");
    location.http_redire = std::make_pair(st_code, url);
}

void FillLocation::rootHandler(std::vector<std::string> values, state)
{
    if (values.size() > 1)
        throw std::logic_error("Error: root: too many values.");
    if (!values.size())
        throw std::logic_error("Error: root: missing value.");
    if (!valid_path(values[0]))
        throw std::logic_error("Error: root: invalid path: `" + values[0] + "'");
    location.root = values[0];
}

void FillLocation::autoindexHandler(std::vector<std::string> values, state)
{
    if (values.size() > 1)
        throw std::logic_error("Error: autoindex: too many values.");
    if (!values.size())
        throw std::logic_error("Error: autoindex: missing value.");
    if (values[0] == "on")
        location.autoindex = true;
    else if (values[0] == "off")
        location.autoindex = false;
    else
        throw std::logic_error("Error: autoindex: invalid value: `" + values[0] + "'");
    location.autoindex_set = true;
}

void FillLocation::indexHandler(std::vector<std::string> values, state)
{
	if (values.size() > 2)
		throw std::logic_error("Error: return: too many values.");
	if (values.size() < 2)
		throw std::logic_error("Error: return: missing value.");

	std::string status_code = values[0];
	if (!str_digit(status_code))
		throw std::logic_error("Error: return: invalid status code: `" + status_code + "'");

	char *end = NULL;
	int st_code = std::strtol(status_code.c_str(), &end, 10);
	if (status_code[0] == '0' || errno == ERANGE || (st_code != 301 && st_code != 302))
		throw std::logic_error("Error: return: invalid status code: `" + status_code + "'");

	std::string url = values[1];
	if (url.find("http://") && url.find("https://") && url[0] != '/')
		throw std::logic_error("Error: return: invalid url: `" + url + "'");
	location.http_redire = std::make_pair(st_code, url);
}

void FillLocation::uploadHandler(std::vector<std::string> values, state)
{
    if (values.size() > 1)
        throw std::logic_error("Error: upload_store: too many values.");
    if (!values.size())
        throw std::logic_error("Error: upload_store: missing value.");
    if (!valid_path(values[0]))
        throw std::logic_error("Error: upload_store: invalid path: `" + values[0] + "'");
    location.upload_store = values[0];
}

void FillLocation::cgiPassHandler(std::vector<std::string> values, state)
{
    if (values.size() > 1)
        throw std::logic_error("Error: cgi_pass: too many values.");
    if (!values.size())
        throw std::logic_error("Error: cgi_pass: missing value.");
    if (!valid_path(values[0]))
        throw std::logic_error("Error: cgi_pass: invalid path: `" + values[0] + "'");
    location.cgi_pass = values[0];
}

void FillLocation::cgiExtHandler(std::vector<std::string> values, state)
{
	if (values.size() > 1)
		throw std::logic_error("Error: cgi_extension: too many values.");
	if (!values.size())
		throw std::logic_error("Error: cgi_extension: missing value.");
	std::string ext = values[0];
	if (ext.find(".") || ext.size() == 1 || std::count(ext.begin() + 1, ext.end(), '.'))
		throw std::logic_error("Error: cgi_extension: invalid extension: `" + ext + "'");
	location.cgi_extension = ext;
}

void FillLocation::BodySzHandler(std::vector<std::string> values, state)
{
    if (values.size() > 1)
        throw std::logic_error("Error: client_max_body_size: too many values.");
    if (!values.size())
        throw std::logic_error("Error: client_max_body_size: missing value.");
    
    char *end = NULL;
    size_t size = std::strtoul(values[0].c_str(), &end, 10);
    if (*end != '\0')
        throw std::logic_error("Error: client_max_body_size: invalid number: `" + values[0] + "'");
    location.body_size = size;
    location.body_size_set = true;
}

void FillLocation::ErrPgHandler(std::vector<std::string> values, state)
{
    if (values.size() != 2)
        throw std::logic_error("Error: error_page: expected 2 values (status_code and url)");
    
    std::string code = values[0];
    if (!str_digit(code))
        throw std::logic_error("Error: error_page: invalid status code: `" + code + "'");
    
    location.error_page[std::atoi(code.c_str())] = values[1];
}



bool FillLocation::directive(std::string str)
{
    if (str == "location")
        return true;
    for (size_t i = 0; i < 10; i++)
    {
        if (str == Directives[i])
            return true;
    }
    return false;
}


void FillLocation::LocationFiller(std::vector<std::pair<tokenType, std::string> > tokens, size_t& pos)
{
    if (pos == tokens.size())
        return;

    std::vector<std::string> values;
    // Renommer la variable pour ne pas cacher la méthode
    std::string dirName = tokens[pos].second;

    pos++;
    
    // Appel correct de la méthode directive()
    while (pos < tokens.size() && tokens[pos].first == WORD && !this->directive(tokens[pos].second))
    {
        values.push_back(tokens[pos].second);
        pos++;
    }

    if (pos >= tokens.size() || tokens[pos].first != SEMI_COL)
        throw std::logic_error("Error: invalid syntax: expected ';' after directive value.");
    pos++;

    for (size_t i = 0; i < 10; i++)
    {
        if (dirName == Directives[i])
        {
            (this->*caller[i])(values, LOCATION);
            return;
        }
    }
    throw std::logic_error("Error: undefined directive: `" + dirName + "'");
}