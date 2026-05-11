#include "include/checkDirect.hpp"

int main()
{
	std::vector<std::pair<tokenType, std::string> > tokens;

	tokens.push_back(std::make_pair(WORD, "client_max_body_size"));
	tokens.push_back(std::make_pair(WORD, "1G"));
	tokens.push_back(std::make_pair(SEMI_COL, ";"));

	int pos = 0;
	FillServer fil;
	try
	{
		fil.fillServer(tokens, pos);
		fil.fillServer(tokens, pos);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 0;
	}
	std::cout << fil.server.body_size << std::endl;
	// std::map<int, std::string>::iterator it;
	// for (it = fil.server.error_page.begin(); it != fil.server.error_page.end(); it++)
	// {
	// 	std::cout << "code: " << it->first << " path: " << it->second << std::endl;
	// }
	
	// std::cout << "ip: " << fil.server.listen[0].first << "\nport: " << fil.server.listen[0].second << "\npos: " << pos << std::endl;
}