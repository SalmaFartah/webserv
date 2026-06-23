#include "event_loop/loopTools.hpp"


int main(int ac, char *av[])
{
    if (ac > 2)
        return std::cerr << "Error: Bad Argument" << std::endl, 1;
    std::ifstream Fileconf;
    if (ac == 2)
	    Fileconf.open(av[1]);
    else
	    Fileconf.open("nginx.conf");
	if (!Fileconf)
		return std::cerr << "Error: could not open file" << std::endl, 1;
	// if the file founded with the right permission above;
	// ------------------------------------------------------------------
	conf confObj;

	confObj.read_file(Fileconf);
	std::vector<serverConf> serv;
	try
	{
		serv = parseConfig(confObj.getTokenz());

	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
	print_config(serv);
	
	// i must read all the file and tooks all the values as tokenz except whitesapces and comments;
	// --------------------------------------------------------------------------------------------
	// confObj.print_tokenz(); // if u want to Print each one
	Fileconf.close();


	/*#################--EVENT LOOP--######################*/

	try
	{
		loopTools lp(serv);
		lp.mainLoop();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	
}