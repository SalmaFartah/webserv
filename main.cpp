#include "include/FillServer.hpp"
#include "include/FillLocation.hpp"

int main()
{
	std::vector<std::pair<tokenType, std::string> > locationTokens;

	// tokens.push_back(std::make_pair(WORD, "listen"));
	// tokens.push_back(std::make_pair(WORD, "localhost:45"));
	// tokens.push_back(std::make_pair(SEMI_COL, ";"));
	// tokens.push_back(std::make_pair(WORD, "listen"));
	// tokens.push_back(std::make_pair(WORD, "192.0.232.1:80"));
	// tokens.push_back(std::make_pair(SEMI_COL, ";"));
	// tokens.push_back(std::make_pair(WORD, "listen"));
	// tokens.push_back(std::make_pair(WORD, "12.3.5.2:443"));
	// tokens.push_back(std::make_pair(SEMI_COL, ";"));
	locationTokens.push_back(std::make_pair(WORD, "allowed_method"));
	locationTokens.push_back(std::make_pair(WORD, "GET"));
	locationTokens.push_back(std::make_pair(WORD, "POST"));
	locationTokens.push_back(std::make_pair(SEMI_COL, ";"));

	locationTokens.push_back(std::make_pair(WORD, "return"));
	locationTokens.push_back(std::make_pair(WORD, "302"));
	locationTokens.push_back(std::make_pair(WORD, "https://www.google.com"));
	locationTokens.push_back(std::make_pair(SEMI_COL, ";"));

	locationTokens.push_back(std::make_pair(WORD, "root"));
	locationTokens.push_back(std::make_pair(WORD, "/path/s"));
	locationTokens.push_back(std::make_pair(SEMI_COL, ";"));

	locationTokens.push_back(std::make_pair(WORD, "autoindex"));
	locationTokens.push_back(std::make_pair(WORD, "on"));
	locationTokens.push_back(std::make_pair(SEMI_COL, ";"));

	locationTokens.push_back(std::make_pair(WORD, "index"));
	locationTokens.push_back(std::make_pair(WORD, ".12"));
	locationTokens.push_back(std::make_pair(SEMI_COL, ";"));

	locationTokens.push_back(std::make_pair(WORD, "upload_store"));
	locationTokens.push_back(std::make_pair(WORD, "/path/"));
	locationTokens.push_back(std::make_pair(SEMI_COL, ";"));

	locationTokens.push_back(std::make_pair(WORD, "cgi_pass"));
	locationTokens.push_back(std::make_pair(WORD, "/path/"));
	locationTokens.push_back(std::make_pair(SEMI_COL, ";"));

	locationTokens.push_back(std::make_pair(WORD, "cgi_extension"));
	locationTokens.push_back(std::make_pair(WORD, ".12"));
	locationTokens.push_back(std::make_pair(SEMI_COL, ";"));

	locationTokens.push_back(std::make_pair(WORD, "client_max_body_size"));
	locationTokens.push_back(std::make_pair(WORD, "0"));
	locationTokens.push_back(std::make_pair(SEMI_COL, ";"));

	locationTokens.push_back(std::make_pair(WORD, "error_page"));
	locationTokens.push_back(std::make_pair(WORD, "404"));
	locationTokens.push_back(std::make_pair(WORD, "/pay"));
	locationTokens.push_back(std::make_pair(SEMI_COL, ";"));

	FillLocation loc;
	for (size_t pos = 0; pos < locationTokens.size(); )
	{
		try
		{
			loc.LocationFiller(locationTokens, pos);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			return 0;
		}
	}


	// FillServer fil;
	// for (size_t pos = 0; pos < tokens.size(); )
	// {
	// 	try
	// 	{
	// 		fil.fillServer(tokens, pos);
	// 	}
	// 	catch(const std::exception& e)
	// 	{
	// 		std::cerr << e.what() << '\n';
	// 		return 0;
	// 	}
	// }
	// for (size_t i = 0; i < fil.server.listen.size(); i++)
	// 	std::cout << "ip: " << fil.server.listen[i].first << "\nport: " << fil.server.listen[i].second << std::endl;
		
	// std::cout << "body size: " << fil.server.body_size << std::endl;
	// std::map<int, std::string>::iterator it;
	// for (it = fil.server.error_page.begin(); it != fil.server.error_page.end(); it++)
	// {
	// 	std::cout << "code: " << it->first << " path: " << it->second << std::endl;
	// }
	
}