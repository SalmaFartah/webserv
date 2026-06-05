#include "parse_config/inc/parse.hpp"
#include "parse_config/inc/FillLocation.hpp"
#include "parse_config/inc/FillServer.hpp"
#include "loopTools.hpp"


int main(int ac, char *av[])
{
    if (ac > 2)
        return std::cerr << "Error: Bad Argument" << std::endl, 1;
    std::ifstream Fileconf("nginx.conf");
    if (ac == 2)
    	std::ifstream Fileconf(av[1]);
	if (!Fileconf)
		return std::cerr << "Error: could not open file" << std::endl, 1;
	// if the file founded with the right permission above;
	// ------------------------------------------------------------------
	conf confObj;
	confObj.read_file(Fileconf);
	try
	{
		std::vector<serverConf> servers = parseConfig(confObj.getTokenz());
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	// i must read all the file and tooks all the values as tokenz except whitesapces and comments;
	// --------------------------------------------------------------------------------------------

	// confObj.print_tokenz(); // if u want to Print each one
	Fileconf.close();


	/*#################--EVENT LOOP--######################*/

	// try
	// {
	// 	loopTools lp;
	// 	lp.mainLoop();
	// }
	// catch(const std::exception& e)
	// {
	// 	std::cerr << e.what() << '\n';
	// }
	
	
}