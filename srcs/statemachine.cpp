#include "../include/FillServer.hpp" 
#include <vector>
#include <string>
#include <stdexcept>


std::vector<serverConf> parseConfig(std::vector<std::pair<tokenType, std::string>> tokens)
{
	std::vector<serverConf> servers;
	size_t pos = 0;
	enum { GLOBAL, IN_SERVER, IN_LOCATION } state = GLOBAL;
	
	serverConf currentServer;
	locationConf currentLocation;
	FillServer filler;

	while (pos < tokens.size())
	{
		tokenType type = tokens[pos].first;
		std::string value = tokens[pos].second;

		switch (state)
		{
			case GLOBAL:
				if (type != WORD || value != "server")
					throw std::runtime_error("Expected 'server' at top level");
				pos++;
				if (pos >= tokens.size() || tokens[pos].first != OPEND_BC)
					throw std::runtime_error("Expected '{' after 'server'");
				pos++;
				currentServer = serverConf(); // default
				state = IN_SERVER;
				break;

			case IN_SERVER:
				if (type == CLOSED_BC) // if }
				{
					servers.push_back(currentServer);
					state = GLOBAL;
					pos++;
				}
				else if (type == WORD && value == "location")
				{
					// path must be non duplicated, starting with / and not contain //
					pos++; // after "location"
					if (pos >= tokens.size() || tokens[pos].first != WORD)
						throw std::runtime_error("Expected location path");
					std::string path = tokens[pos].second;
					pos++; // after path
					if (pos >= tokens.size() || tokens[pos].first != OPEND_BC)
						throw std::runtime_error("Expected '{' after location path");
					pos++; // after {
					currentLocation = locationConf();
					currentLocation.path = path;
					state = IN_LOCATION;
				}
				else // if a directive
				{
					filler.server = currentServer;
					filler.fillServer(tokens, pos);
					currentServer = filler.server;
				}
				break;

			case IN_LOCATION: // after {
				if (type == CLOSED_BC) // if }
				{
					currentServer.locations.push_back(currentLocation);
					state = IN_SERVER;
					pos++;
				}
				else if (type == WORD)
				{
					// sould call the fillLocation form taskC.					
				}
				else
					throw std::runtime_error("Unexpected token in location block"); // can be {
				break;
		}
	}

	if (state != GLOBAL)
		throw std::runtime_error("Unclosed block at end of file");
	return servers;
}