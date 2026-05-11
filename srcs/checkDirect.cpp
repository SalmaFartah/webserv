#include "../include/checkDirect.hpp"

FillServer::FillServer()
{
	Directives[0] = "listen";
	Directives[1] = "error_page";
	Directives[2] = "client_max_body_size";

	caller[0] = &FillServer::ListenHandler;
	caller[1] = &FillServer::ErrPgHandler;
	caller[2] = &FillServer::BodySzHandler;
}

bool FillServer::valid_ip(std::string vl)
{
	size_t point;
	if ((point = vl.find(".")) == vl.npos || point == 0 || point == vl.size())
		return false;	
	std::stringstream ip(vl);
	std::string segment;
	int cnt;
	for (cnt = 0; std::getline(ip, segment, '.'); cnt++)
	{
		if (!str_digit(segment) || atoi(segment.c_str()) < 0 || atoi(segment.c_str()) > 255)
			break;
	}
	if (cnt != 4)
		return false;
	return true;
}

void FillServer::ListenHandler(std::vector<std::string> values)
{
	if (values.size() > 1)
		throw std::logic_error("Error: listen: too many values.");
	if (values.size() < 1)
		throw std::logic_error("Error: listen: missing value.");
	std::string vl = values[0];
	int port;
	size_t posColon = vl.find(":");
	if (posColon == vl.size() - 1)
		throw std::logic_error("Error: listen: expected `PORT' after `:'");
	if (posColon == 0)
		throw std::logic_error("Error: listen: missing `IP' address before `:'");
	if (posColon == vl.npos) // makinsh colon so ya ima ip bohdha wla port bohdo
	{
		// no ':' found jst ip or jst port
		if (valid_ip(vl)) // if it is jst ip 127.0.0.1
			server.listen.push_back(std::make_pair(vl, 80));
		else if (str_digit(vl) && (port = atoi(vl.c_str())) >= 1 && port <= 65535) // if it is port 8080
			server.listen.push_back(std::make_pair("0.0.0.0", port));
		else
			throw std::logic_error("Error: listen: invalid address format.");
	}
	else if (posColon != 0 && posColon != vl.size() - 1)
	{

		// std::cout << "pos: " << posColon << " size: " << vl.size() << "\n";
		std::string ip = vl.substr(0, posColon);
		std::string portstr = vl.substr(posColon + 1, vl.size() - posColon);

		if (!valid_ip(ip))
			throw std::logic_error("Error: listen: invalid IP address.");
		if (!str_digit(portstr))
			throw std::logic_error("Error: listen: invalid port.");
		port = atoi(portstr.c_str());
		if (port < 1 && port > 65535)
			throw std::logic_error("Error: listen: port out of range.");

		server.listen.push_back(std::make_pair(ip, port));
	}		
}

void FillServer::ErrPgHandler(std::vector<std::string> values)
{
	if (!values.size())
		throw std::logic_error("Error: error_page: missing error code");
	
	std::string path = values.back();
	int err_code;
	size_t i = 0;
	for (; i < values.size() - 1; i++)
	{
		err_code = atoi(values[i].c_str());
		if (!str_digit(values[i]) || (err_code != 400 \
		&& err_code != 403 && err_code != 404 && err_code != 405 \
		&& err_code != 413 && err_code != 500 && err_code != 501))
		{
			server.error_page.clear();
			throw std::logic_error("Error: error_page: invalid error code: `" + values[i] + "'");
		}
		server.error_page[err_code] = path;
	}
	if (str_digit(path))
		throw std::logic_error("Error: error_page: missing file path");
	else if (!i)
		throw std::logic_error("Error: error_page: missing error code");
	if (!valid_path(path))
		throw std::logic_error("Error: error_page: invalid path");
}
bool valid_suffix(char c)
{
	if (!c || c == 'k' || c == 'K' || c == 'g' || c == 'G' || c == 'm' || c == 'M')
		return true;
	return false;	
}

void FillServer::BodySzHandler(std::vector<std::string> values)
{
	if (values.size() > 1)
		throw std::logic_error("Error: client_max_body_size: too many values.");
	if (values.size() < 1)
		throw std::logic_error("Error: client_max_body_size: missing value.");

	std::string val = values[0];
	char *end = NULL;
	server.body_size = std::strtoull(val.c_str(), &end, 10);

	if (val[0] == '-' || errno == ERANGE || !valid_suffix(*end) \
	|| (!server.body_size && end == val.c_str()) || end[1])
		throw std::logic_error("Error: client_max_body_size: invalid value: `" + val + "'");
	size_t max = std::numeric_limits<size_t>::max();
	switch (*end)
	{
		case 'k':
		case 'K':
			if (server.body_size > max / 1024)
				throw std::logic_error("Error: client_max_body_size: too a large value.");
			server.body_size *= 1024;
			break;
		case 'm':
		case 'M':
			if (server.body_size > max / std::pow(1024, 2))
				throw std::logic_error("Error: client_max_body_size: too a large value.");
			server.body_size *= std::pow(1024, 2);
			break;
		case 'g':
		case 'G':
			if (server.body_size > max / std::pow(1024, 3))
				throw std::logic_error("Error: client_max_body_size: too a large value.");
			server.body_size *= std::pow(1024, 3);
		default:
			break;
	}
}

void FillServer::fillServer(std::vector<std::pair<tokenType, std::string> > tokens, size_t& pos)
{
	if (pos == tokens.size())
		return ;

	std::vector<std::string> values;
	std::string dierective = tokens[pos].second;

	pos++;
	for (; pos < tokens.size() && tokens[pos].first != SEMI_COL ; pos++)
		values.push_back(tokens[pos].second);
	if (tokens[pos].first != SEMI_COL)
		throw std::logic_error("Error: invalid syntax: expected ';' after directive value. ");
	pos++;
	for (size_t i = 0; i < 4; i++)
	{
		if (dierective == Directives[i])
		{
			(this->*caller[i])(values);
			return ;
		}
	}
	throw std::logic_error("Error: undefined directive: '" + dierective + "'");
}

FillServer::~FillServer(){}
