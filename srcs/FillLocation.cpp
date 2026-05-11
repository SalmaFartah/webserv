#include "../include/FillLocation.hpp"

FillLocation::FillLocation()
{
	Directives[0] = "allowed_method";
	Directives[1] = "return";
	Directives[2] = "root";
	Directives[3] = "autoindex";
	Directives[4] = "index";
	Directives[5] = "upload_store";
	Directives[6] = "cgi_pass";
	Directives[7] = "cgi_extension";

	caller[0] = &FillLocation::methodsHandler;
	caller[1] = &FillLocation::returnHandler;
	caller[2] = &FillLocation::rootHandler;
	caller[3] = &FillLocation::autoindexHandler;
	caller[4] = &FillLocation::indexHandler;
	caller[5] = &FillLocation::uploadHandler;
	caller[6] = &FillLocation::cgiPassHandler;
	caller[7] = &FillLocation::cgiExtHandler;
}

void FillLocation::methodsHandler( std::vector<std::string> values)
{
	if (!values.size())
		throw std::logic_error("Error: allowed_methods: missing value.");
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
			throw std::logic_error("Error: allowed_methods: invalid method: `" + values[i] + "'");
		if (!result.second)
			throw std::logic_error("Error: allowed_methods: duplicate method.");
	}
}

bool str_digit(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

void FillLocation::returnHandler( std::vector<std::string> values )
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
	if (errno == ERANGE || (st_code != 301 && st_code != 302 \
	&& st_code != 303 && st_code != 307 && st_code != 308))
		throw std::logic_error("Error: return: invalid status code: `" + status_code + "'");

	std::string url = values[1];
	if (url.find_first_of("http://") && url.find_first_of("https://") && url[0] != '/')
		throw std::logic_error("Error: return: invalid url: `" + url + "'");
	location.http_redire = std::make_pair(st_code, url);
}

bool valid_path(std::string path)
{
	for (size_t i = 0; i < path.size(); i++)
	{
		if ((!isdigit(path[i]) && !isalpha(path[i]) \
		&& path[i] != '/' && path[i] != '.' \
		&& path[i] != '_' && path[i] != '-' ) \
		|| (i == 0 && path[i] != '/'))
			return false;
	}
	return true;
}

void FillLocation::rootHandler( std::vector<std::string> values)
{
	if (values.size() > 1)
		throw std::logic_error("Error: root: too many values.");
	if (!values.size())
		throw std::logic_error("Error: root: missing value.");
	std::string path = values[0];
	if (!valid_path(path))
		throw std::logic_error("Error: root: invalid path: `" + path + "'");
	location.root = path;
}

void FillLocation::autoindexHandler( std::vector<std::string> values)
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
}

void FillLocation::indexHandler( std::vector<std::string> values)
{
	if (!values.size())
		throw std::logic_error("Error: autoindex: too many values.");
	
}

void FillLocation::LocationFiller(std::vector<std::pair<tokenType, std::string> > tokens, size_t& pos)
{
	if (pos == tokens.size())
		return ;

	std::vector<std::string> values;
	std::string dierective = tokens[pos].second;

	pos++;
	for (; pos < tokens.size() && tokens[pos].first != SEMI_COL ; pos++)
		values.push_back(tokens[pos].second);

	if (tokens[pos].first != SEMI_COL)
		throw std::logic_error("Error: invalid syntax: expected ';' after directive value.");
	pos++;
	for (size_t i = 0; i < 8; i++)
	{
		if (dierective == Directives[i])
		{
			(this->*caller[i])(values);
			return ;
		}
	}
	throw std::logic_error("Error: undefined directive: '" + dierective + "'");
}
