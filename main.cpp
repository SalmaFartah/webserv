#include "include/FillServer.hpp"

int main()
{
	std::vector<std::pair<tokenType, std::string> > tokens;

	tokens.push_back(std::make_pair(WORD, "listen"));
	tokens.push_back(std::make_pair(WORD, "localhost:45"));
	tokens.push_back(std::make_pair(SEMI_COL, ";"));
	tokens.push_back(std::make_pair(WORD, "listen"));
	tokens.push_back(std::make_pair(WORD, "192.0.00000.1:80"));
	tokens.push_back(std::make_pair(SEMI_COL, ";"));
	tokens.push_back(std::make_pair(WORD, "listen"));
	tokens.push_back(std::make_pair(WORD, "12.3.5.2:443"));
	tokens.push_back(std::make_pair(SEMI_COL, ";"));

	FillServer fil;
	for (size_t pos = 0; pos < tokens.size(); )
	{
		try
		{
			fil.fillServer(tokens, pos);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			return 0;
		}
	}
	for (size_t i = 0; i < fil.server.listen.size(); i++)
		std::cout << "ip: " << fil.server.listen[i].first << " port: " << fil.server.listen[i].second << std::endl;
		
	// std::cout << fil.server.body_size << std::endl;
	// std::map<int, std::string>::iterator it;
	// for (it = fil.server.error_page.begin(); it != fil.server.error_page.end(); it++)
	// {
	// 	std::cout << "code: " << it->first << " path: " << it->second << std::endl;
	// }
	
	// std::cout << "ip: " << fil.server.listen[0].first << "\nport: " << fil.server.listen[0].second << "\npos: " << pos << std::endl;
}