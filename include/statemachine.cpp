#include "FillServer.hpp" 
#include <vector>
#include <string>
#include <stdexcept>


std::vector<serverConf> parseConfig(std::vector<std::pair<tokenType, std::string>> tokens) {
    std::vector<serverConf> servers;
    size_t pos = 0;
    enum { GLOBAL, IN_SERVER, IN_LOCATION } state = GLOBAL;
    
    serverConf currentServer;
    locationConf currentLocation;
    FillServer filler;

    while (pos < tokens.size()) {
        tokenType type = tokens[pos].first;
        std::string value = tokens[pos].second;

        switch (state) {
            case GLOBAL:
                if (type != WORD || value != "server")
                    throw std::runtime_error("Expected 'server' at top level");
                pos++;
                if (pos >= tokens.size() || tokens[pos].first != LBRACE)
                    throw std::runtime_error("Expected '{' after 'server'");
                pos++;
                currentServer = serverConf();
                state = IN_SERVER;
                break;

            case IN_SERVER:
                if (type == RBRACE) {
                    servers.push_back(currentServer);
                    state = GLOBAL;
                    pos++;
                }
                else if (type == WORD && value == "location") {
                    pos++;
                    if (pos >= tokens.size() || tokens[pos].first != WORD)
                        throw std::runtime_error("Expected location path");
                    std::string path = tokens[pos].second;
                    pos++;
                    if (pos >= tokens.size() || tokens[pos].first != LBRACE)
                        throw std::runtime_error("Expected '{' after location path");
                    pos++;
                    currentLocation = locationConf();
                    currentLocation.path = path;
                    state = IN_LOCATION;
                }
                else {
                    filler.server = currentServer;
                    filler.fillServer(tokens, pos);
                    currentServer = filler.server;
                }
                break;

            case IN_LOCATION:
                if (type == RBRACE) {
                    currentServer.locations.push_back(currentLocation);
                    state = IN_SERVER;
                    pos++;
                }
                else {
                    if (type != WORD) throw std::runtime_error("Unexpected token in location block");
                    pos++;
                    if (value == "root") {
                        if (pos >= tokens.size() || tokens[pos].first != WORD)
                            throw std::runtime_error("Expected root path");
                        currentLocation.root = tokens[pos].second;
                        pos++;
                    }
                    else if (value == "autoindex") {
                        if (pos >= tokens.size() || tokens[pos].first != WORD)
                            throw std::runtime_error("Expected 'on' or 'off'");
                        currentLocation.autoindex = (tokens[pos].second == "on");
                        pos++;
                    }
                    else if (value == "index") {
                        while (pos < tokens.size() && tokens[pos].first == WORD) {
                            currentLocation.index.push_back(tokens[pos].second);
                            pos++;
                        }
                    }
                    else if (value == "methods") {
                        while (pos < tokens.size() && tokens[pos].first == WORD) {
                            currentLocation.methods.push_back(tokens[pos].second);
                            pos++;
                        }
                    }
                    else if (value == "return") {
                        if (pos >= tokens.size() || tokens[pos].first != WORD)
                            throw std::runtime_error("Expected HTTP code");
                        int code = std::atoi(tokens[pos].second.c_str());
                        pos++;
                        if (pos >= tokens.size() || tokens[pos].first != WORD)
                            throw std::runtime_error("Expected redirect URL");
                        std::string url = tokens[pos].second;
                        pos++;
                        currentLocation.http_redire = std::make_pair(code, url);
                    }
                    else if (value == "upload_store") {
                        if (pos >= tokens.size() || tokens[pos].first != WORD)
                            throw std::runtime_error("Expected upload path");
                        currentLocation.upload_store = tokens[pos].second;
                        pos++;
                    }
                    else {
                        throw std::runtime_error("Unknown location directive: " + value);
                    }
                    if (pos >= tokens.size() || tokens[pos].first != SEMI_COL)
                        throw std::runtime_error("Expected ';'");
                    pos++;
                }
                break;
        }
    }

    if (state != GLOBAL)
        throw std::runtime_error("Unclosed block at end of file");
    return servers;
}