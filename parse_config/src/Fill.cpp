#include "../include/Fill.hpp"

bool Fill::str_digit(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
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