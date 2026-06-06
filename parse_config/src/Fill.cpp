#include "../inc/Fill.hpp"

bool Fill::str_digit(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

void print_config(std::vector<serverConf> conf)
{
	for (size_t i = 0; i < conf.size(); i++)
	{
		std::cout << "-----------server (" << i + 1 << ")--------" << std::endl;
		std::cout << "[LISTEN DIRECTIVE]" << std::endl;

		for (size_t j = 0; j < conf[i].listen.size(); j++)
		{
			std::cout << "ip: " << conf[i].listen[j].first << " port: " << conf[i].listen[j].second << std::endl;
		}
		std::cout << "[ERROR PAGES]" << std::endl;
		std::map<int, std::string>::iterator it;
		for (it = conf[i].error_page.begin(); it != conf[i].error_page.end(); it++)
		{
			std::cout << "code: " << it->first << " path: " << it->second << std::endl;
		}
		std::cout << "[MAX BODY SIZE]" << std::endl;
		std::cout << "body size: " << conf[i].body_size << std::endl;
		std::cout << "[ROOT DIRECTIVE]" << std::endl;
		std::cout << "root: " << conf[i].root << std::endl;
		std::cout << "[AUTOINDEX DIRECTIVE]" << std::endl;
		std::cout << "autoindex: " << conf[i].autoindex << std::endl;
		std::cout << "[INDEX DIRECTIVE]" << std::endl;
		for (size_t j = 0; j < conf[i].index.size(); j++)
		{
			std::cout << conf[i].index[j] << " ";
		}
		std::cout << "\n[LOCATION BLOCK]" << std::endl;
		for (size_t n = 0; n < conf[i].locations.size(); n++)
		{
			std::cout << "location path: " << conf[i].locations[n].path;
			std::cout << "[ALLOWED_METHOD DIRECTIVE]" << std::endl;
			std::set<std::string>::iterator it;
			for (it = conf[i].locations[n].methods.begin(); it != conf[i].locations[n].methods.end(); it++)
			{
				std::cout << *it << " ";
			}
			std::cout << "\n[RETURN DIRECTIVE]" << std::endl;
			std::cout << "status code: " << conf[i].locations[n].http_redire.first << " url: " << conf[i].locations[n].http_redire.second << std::endl;
			std::cout << "[ROOT LOCATION]" << std::endl;
			std::cout << "root: " << conf[i].locations[n].root << std::endl;
			std::cout << "[AUTOINDEX DIRECTIVE]" << std::endl;
			std::cout << "autoindex: " << conf[i].locations[n].autoindex << std::endl;
			std::cout << "[INDEX DIRECTIVE]" << std::endl;
			for (size_t j = 0; j < conf[i].locations[n].index.size(); j++)
			{
				std::cout << conf[i].locations[n].index[j];
			}
			std::cout << "\n[UPLOAD_STORE DIRECTIVE]" << std::endl;
			std::cout << "path: " << conf[i].locations[n].upload_store << std::endl;
			std::map<int, std::string>::iterator iter;
			for (iter = conf[i].locations[n].error_page.begin(); iter != conf[i].locations[n].error_page.end(); it++)
			{
				std::cout << "code: " << iter->first << " path: " << iter->second << std::endl;
			}
			std::cout << "[MAX BODY SIZE]" << std::endl;
			std::cout << "body size: " << conf[i].locations[n].body_size << std::endl;
			std::cout << "[CGI_EXTENSION]" << std::endl;
			std::cout << conf[i].locations[n].cgi_extension << std::endl;
			std::cout << "[CGI_PASS]" << std::endl;
			std::cout << conf[i].locations[n].cgi_pass << std::endl;
		}
		
	}
	
}

bool Fill::valid_path(std::string path)
{
	if (path.find("/") != 0 || path.find("//") != path.npos)
		return false;
	return true;
}

void Fill::rootHandler( std::vector<std::string> values, state type)
{
	if (values.size() > 1)
		throw std::logic_error("Error: root: too many values.");
	if (!values.size())
		throw std::logic_error("Error: root: missing value.");
	std::string path = values[0];
	if (!valid_path(path))
		throw std::logic_error("Error: root: invalid path: `" + path + "'");
	if (type == LOCATION)
		location.root = path;
	else
		server.root = path;
}

void Fill::autoindexHandler( std::vector<std::string> values, state type)
{
	if (values.size() > 1)
		throw std::logic_error("Error: autoindex: too many values.");
	if (!values.size())
		throw std::logic_error("Error: autoindex: missing value.");
	if (values[0] == "on")
	{
		if (type == LOCATION)
			location.autoindex = true;
		else
			server.autoindex = true;
	}
	else if (values[0] == "off")
	{
		if (type == LOCATION)
			location.autoindex = false;
		else
			server.autoindex = false;
	}
	else
		throw std::logic_error("Error: autoindex: invalid value: `" + values[0] + "'");
}

void Fill::indexHandler( std::vector<std::string> values, state type)
{
	if (server.defaulti)
	{
		server.index.clear();
		server.defaulti = false;
	}
	std::string filename;
	if (!values.size())
		throw std::logic_error("Error: index: missing value.");
	for (size_t i = 0; i < values.size(); i++)
	{
		if (values[i][0] == '/')
			throw std::logic_error("Error: index: absolute path not accepted: `" + values[i] + "'");
		if (type == LOCATION)
			location.index.push_back(values[i]);
		else
			server.index.push_back(values[i]);
	}
}

bool Fill::valid_suffix(char c)
{
	if (!c || c == 'k' || c == 'K' || c == 'g' || c == 'G' || c == 'm' || c == 'M')
		return true;
	return false;	
}

void Fill::BodySzHandler(std::vector<std::string> values, state type)
{
	if (values.size() > 1)
		throw std::logic_error("Error: client_max_body_size: too many values.");
	if (values.size() < 1)
		throw std::logic_error("Error: client_max_body_size: missing value.");

	std::string val = values[0];
	char *end = NULL;
	size_t value = std::strtoull(val.c_str(), &end, 10);

	if (val[0] == '-' || errno == ERANGE || !valid_suffix(*end) \
	|| (!value && end == val.c_str()) || end[1] \
	|| (value && val[0] == '0') || (value == 0 && val.size() > 1))
		throw std::logic_error("Error: client_max_body_size: invalid value: `" + val + "'");
	size_t max = std::numeric_limits<size_t>::max();
	switch (*end)
	{
		case 'k':
		case 'K':
			if (value > max / 1024)
				throw std::logic_error("Error: client_max_body_size: too a large value.");
			value *= 1024;
			break;
		case 'm':
		case 'M':
			if (value > max / std::pow(1024, 2))
				throw std::logic_error("Error: client_max_body_size: too a large value.");
			value *= std::pow(1024, 2);
			break;
		case 'g':
		case 'G':
			if (value > max / std::pow(1024, 3))
				throw std::logic_error("Error: client_max_body_size: too a large value.");
			value *= std::pow(1024, 3);
		default:
			break;
	}
	if (type == SERVER)
		server.body_size = value;
	else
		location.body_size = value;	
}

void Fill::ErrPgHandler(std::vector<std::string> values, state type)
{
	if (!values.size())
		throw std::logic_error("Error: error_page: missing error code");
	
	std::string path = values.back();
	int err_code;
	size_t i = 0;
	for (; i < values.size() - 1; i++)
	{
		err_code = atoi(values[i].c_str());
		if (values[i].size() != 3 || values[i][0] == '0' || !str_digit(values[i]) || (err_code != 400 \
		&& err_code != 403 && err_code != 404 && err_code != 405 \
		&& err_code != 413 && err_code != 500 && err_code != 501))
			throw std::logic_error("Error: error_page: invalid error code: `" + values[i] + "'");

		if (type == SERVER)
			server.error_page[err_code] = path;
		else
			location.error_page[err_code] = path;
	}
	if (str_digit(path))
		throw std::logic_error("Error: error_page: missing file path");
	else if (!i)
		throw std::logic_error("Error: error_page: missing error code");
	if (!valid_path(path))
		throw std::logic_error("Error: error_page: invalid path");
}

Fill::Fill(){}

Fill::~Fill(){}