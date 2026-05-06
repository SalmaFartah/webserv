#include "../include/checkDirect.hpp"

FillServer::FillServer()
{
	Directives[0] = "listen";
	Directives[1] = "server_name";
	Directives[2] = "error_page";
	Directives[3] = "client_max_body_size";

	caller[0] = &FillServer::ListenHandler;
	caller[1] = &FillServer::ServerNmHandler;
	caller[2] = &FillServer::ErrPgHandler;
	caller[3] = &FillServer::BodySzHandler;
}

bool FillServer::str_digit(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

void FillServer::ListenHandler(std::vector<std::string> values)
{
	if (values.size() != 1)
		throw std::logic_error("error:");
	std::string vl = values[0];
	int pos = vl.find(":");
	if (pos == vl.npos)
	{
		// no ':' found jst ip or jst port
		int point;
		int port;
		if ((point = vl.find(".")) != vl.npos && point != 0 && point != vl.size())
		{
			std::stringstream ip(vl);
			std::string segment;
			int cnt;
			for (cnt = 0; std::getline(ip, segment, '.'); cnt++)
			{
				if (!str_digit(segment) || atoi(segment.c_str()) < 0 || atoi(segment.c_str()) > 255)
					break;
			}
			if (cnt != 4)
				throw std::logic_error("error:");
			server.listen.push_back(std::make_pair(vl, 80));
		}
		else if (str_digit(vl) && (port = atoi(vl.c_str())) >= 1 && port <= 65535)
			server.listen.push_back(std::make_pair("0.0.0.0", port));
		else if (vl == "localhost")
		{
			
		}
		
		else
			throw std::logic_error("listen: ");
	}
	else if (pos != 0 && pos != vl.size())
	{
		// ip:port
	}
	else
	{
		// error
	}
}

void FillServer::fillServer(std::vector<std::pair<tokenType, std::string>> tokens, int& pos)
{
	std::vector<std::string> values;
	std::string dierective = tokens[pos].second;

	size_t u = pos + 1;
	for (; u < tokens.size() && tokens[u].first != SEMI_COL ; u++)
		values.push_back(tokens[u].second);
	pos = u;
	if (tokens[pos].first != SEMI_COL)
		throw std::logic_error("error:");
	for (size_t i = 0; i < 4; i++)
	{
		if (dierective == Directives[i])
			(this->*caller[i])(values);
	}
	
}

FillServer::~FillServer()
{
}

