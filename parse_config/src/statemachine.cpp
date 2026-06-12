#include "../inc/FillServer.hpp"
#include "../inc/FillLocation.hpp"
#include "../inc/parse.hpp"
#include <map>
#include <vector>
#include <string>
#include <stdexcept>

std::vector<serverConf> parseConfig(std::vector<std::pair<tokenType, std::string> > tokens)
{
	std::vector<serverConf> servers;
	size_t pos = 0;

	enum State
	{
		GLOBAL,
		IN_SERVER,
		IN_LOCATION
	};

	State state = GLOBAL;

	serverConf currentServer;
	locationConf currentLocation;
	FillServer filler;
	FillLocation fill;

	// required directives tracker
	std::map<std::string, bool> required;

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
					throw std::runtime_error("Expected '{' after server");

				pos++;

				currentServer = serverConf();

				required.clear();

				// required["location"] = false;
				// required["listen"] = false;
				// required["host"] = false;
				required["root"] = false;

				state = IN_SERVER;
				break;

			case IN_SERVER:

				// end of server block
				if (type == CLOSED_BC)
				{
					// validate required directives
					for (std::map<std::string, bool>::iterator it = required.begin();
						 it != required.end(); ++it)
					{
						if (!it->second)
						{
							throw std::runtime_error(
								"Missing required directive: " + it->first);
						}
					}

					for (size_t i = 0; i < currentServer.locations.size(); i++)
					{
						locationConf &loc = currentServer.locations[i];

						if (loc.root.empty() && !currentServer.root.empty())
							loc.root = currentServer.root;
						if (loc.index.empty() && !currentServer.index.empty())
							loc.index = currentServer.index;
						if (!loc.autoindex_set)
							loc.autoindex = currentServer.autoindex;
						if (!loc.body_size_set)
							loc.body_size = currentServer.body_size;
					}

					servers.push_back(currentServer);

					state = GLOBAL;
					pos++;
				}

				// location block
				else if (type == WORD && value == "location")
				{
					required["location"] = true;

					pos++;

					if (pos >= tokens.size() || tokens[pos].first != WORD)
						throw std::runtime_error("Expected location path");

					std::string path = tokens[pos].second;

					if (path.empty() || path[0] != '/')
						throw std::runtime_error("Location path must start with '/'");

					if (path.find("//") != std::string::npos)
						throw std::runtime_error("Location path cannot contain '//'");

					for (size_t i = 0; i < currentServer.locations.size(); i++)
					{
						if (currentServer.locations[i].path == path)
							throw std::runtime_error("Duplicate location path: " + path);
					}

					pos++;

					if (pos >= tokens.size() || tokens[pos].first != OPEND_BC)
						throw std::runtime_error("Expected '{' after location path");

					pos++;

					currentLocation = locationConf();

					currentLocation.path = path;

					// inherit server directives
					currentLocation.root = currentServer.root;
					currentLocation.autoindex = currentServer.autoindex;
					currentLocation.body_size = currentServer.body_size;

					state = IN_LOCATION;
				}

				// server directives
				else
				{
					// mark directive as found if required
					if (required.find(value) != required.end())
						required[value] = true;

					filler.server = currentServer;

					filler.fillServer(tokens, pos);

					currentServer = filler.server;
				}

				break;

			case IN_LOCATION:

				// end of location block
				if (type == CLOSED_BC)
				{
					currentServer.locations.push_back(currentLocation);
					state = IN_SERVER;
					pos++;
				}

				// location directives
				else if (type == WORD)
				{
					fill.location = currentLocation;

    				fill.LocationFiller(tokens, pos);

					currentLocation = fill.location;
				}

				else
				{
					throw std::runtime_error("Unexpected token in location block");
				}

				break;
		}
	}

	if (state != GLOBAL)
		throw std::runtime_error("Unclosed block at end of file");
	checkPortConflict(servers);
	return servers;
}