#include "../inc/FillServer.hpp"
#include <cstring>

FillServer::FillServer()
{
	server.defaults = true;
	server.defaulti = true;
	Directives[0] = "listen";
	Directives[1] = "error_page";
	Directives[2] = "client_max_body_size";
	Directives[3] = "index";
	Directives[4] = "autoindex";
	Directives[5] = "root";

	caller[0] = &FillServer::ListenHandler;
	caller[1] = &FillServer::ErrPgHandler;
	caller[2] = &FillServer::BodySzHandler;
	caller[3] = &FillServer::indexHandler;
	caller[4] = &FillServer::autoindexHandler;
	caller[5] = &FillServer::rootHandler;
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
		int seg = atoi(segment.c_str());
		if (segment.size() > 3 || !str_digit(segment) || seg < 0 || seg > 255 \
		|| (seg != 0 && segment[0] == '0') || (seg == 0 && segment.size() > 1)) // leading 0
			break;
	}
	if (cnt != 4)
		return false;
	return true;
}

std::string FillServer::resolveHost()
{
    struct addrinfo hints;
	struct addrinfo *res;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo("localhost", NULL, &hints, &res))
		throw std::logic_error("");

    char ip[INET_ADDRSTRLEN];
    struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;// type of ai_addr is sockaddr
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

    freeaddrinfo(res);
    return std::string(ip);
}

void FillServer::ListenHandler(std::vector<std::string> values, state)
{
	if (server.defaults)
	{
		server.listen.clear();
		server.defaults = false; 
	}
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
		if (vl == "localhost")
			vl = resolveHost();
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

		if (ip == "localhost")
			ip = resolveHost();
		if (!valid_ip(ip))
			throw std::logic_error("Error: listen: invalid IP address: `" + ip + "'");
		if (!str_digit(portstr))
			throw std::logic_error("Error: listen: invalid port.");
		port = atoi(portstr.c_str());
		if (port < 1 && port > 65535)
			throw std::logic_error("Error: listen: port out of range.");

		server.listen.push_back(std::make_pair(ip, port));
	}		
}

bool FillServer::directive(std::string str)
{
	if (str == "location")
		return true;
	for (size_t i = 0; i < 6; i++)
	{
		if (str == Directives[i])
			return true;
	}
	return false;
}

void FillServer::fillServer(std::vector<std::pair<tokenType, std::string> > tokens, size_t& pos)
{
	if (pos == tokens.size())
		return ;

	std::vector<std::string> values;
	std::string dierective = tokens[pos].second;

	pos++;
	for (; pos < tokens.size() && tokens[pos].first == WORD && !directive(tokens[pos].second) ; pos++)
		values.push_back(tokens[pos].second);
	if (tokens[pos].first != SEMI_COL)
		throw std::logic_error("Error: invalid syntax: expected ';' after directive value. ");
	pos++;
	for (size_t i = 0; i < 6; i++)
	{
		if (dierective == Directives[i])
		{
			(this->*caller[i])(values, SERVER);
			return ;
		}
	}
	throw std::logic_error("Error: undefined directive: '" + dierective + "'");
}

FillServer::~FillServer(){}
