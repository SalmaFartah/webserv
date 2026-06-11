#include "loopTools.hpp"


loopTools::loopTools(){};
void loopTools::close_fds()
{
	for (size_t i = 0; i < serv_nb; i++)
		close(3 + i);
}
loopTools::loopTools(std::vector<serverConf> servers) : serv_nb(0)
{

	for (size_t i = 0; i < servers.size(); i++) // EACH SERVER
	{
		for (size_t j = 0; j < servers[i].listen.size(); j++) // EACH LISTEN ON SERVER
		{
			int serverFd = socket(AF_INET, SOCK_STREAM, 0);
			if (serverFd < 0)
				throw std::logic_error("failed socket");
			serv_nb++;
			

			sockaddr_in servaddr;
			// initliaze a fresh struct and avoid any garbage
			memset(&servaddr, 0, sizeof(servaddr));

		// The socket now needs to be connected to an internet port and an IP address:
			servaddr.sin_family = AF_INET; // match the socket() call -- /* internetwork: UDP, TCP, etc. */
			servaddr.sin_addr.s_addr = inet_addr(servers[i].listen[j].first.c_str()); // /allow the server to accept a client connection on any interface
			servaddr.sin_port = htons(servers[i].listen[j].second); // specify port to listen on

			int enable = 1; // 1 = ON, 0 = OFF
		// used to configure various options and behaviors for a network socket, such as setting timeouts, enabling broadcasts, or reusing addresses.
		// Reusing an Address (SO_REUSEADDR): Allows a socket to forcibly bind to a port in use by another socket in the TIME_WAIT state.
		// SO_REUSEADDR → allows the socket address to be reused immediately, even if it is in the wait state;
			setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

		// fcntl modifies the fundamental input/output (I/O) behavior of the socket descriptor
			fcntl(serverFd, F_SETFL, O_NONBLOCK);
			// attaches the socket to a specific port on the machine
			// extract the first connection request on the queue of pending connections. It creates a brand-new socket descriptor specifically for communicating with that unique client.
			// the second parametre: the operating system kernel fills it in with the incoming client's network identity once a connection lands
			// If your server does not care about the IP address or port of incoming clients, we can set sersock, NULL, NULL 
			if (bind(serverFd, (const sockaddr *)&servaddr, sizeof(servaddr)) < 0)
			{
				perror("bind: ");
				close_fds();
				exit(1);
			}
			
		// tells the OS to start queuing incoming connections
		// backlog — how many connections can queue up waiting to be accepted
			listen(serverFd, 128);
		
			struct pollfd fds;

			fds.fd = serverFd; //// this second param of poll, its like how many slots of your array to read, starting from index 0, to know exactly when to stop scanning memory, without loosing cpu for all fds only the active ones
			fds.events = POLLIN; // if someone knock the door(there is a data to read) watchout
			fds.revents = 0;

			vecFds.push_back(fds);

			linkServConf[serverFd] = &servers[i];
		}
	}
	
}

void loopTools::incompleteCase()
{
	
}

void loopTools::newConnection(struct pollfd& server)
{
	// handle a client conenction
	sockaddr_in cliaddr;
	socklen_t client_len = sizeof(cliaddr);

	int cli_sock = accept(server.fd, (sockaddr *)&cliaddr, &client_len);

	if (cli_sock < 0)
		perror("accept: ");
	if (cli_sock >= 0)
		printf("[SERVER] New connection accepted on FD: %d\n", cli_sock);

	fcntl(cli_sock, F_SETFL, O_NONBLOCK);

	server.revents = 0;
	
//	add the master then override it with new client
	vecFds.push_back(vecFds[0]);
	vecFds.back().fd = cli_sock;
	vecFds.back().revents = 0;
	
}


void loopTools::existClient(struct pollfd& client, int clieIdx)
{
	char buffer[BUFFER_SZ];
	ssize_t reading = read(client.fd, buffer, sizeof(buffer) - 1); // our read is non blocking io mean if our kernel buffer is empty read will not frozen here and wait
	if (reading > 0)
	{
		buffer[reading] = '\0';
		
		infoClie[clieIdx].clieFile += buffer; // this one accumulate buffer
		infoClie[clieIdx].request.parse_request(infoClie[clieIdx].clieFile, infoClie[clieIdx].cliConf);
		
		// if (infoClie[clieIdx].request.rtype == 0) // INCOMPLETE
		// {
		// 	// set timer overide each time
		// 	infoClie[clieIdx].clieTime = std::time(NULL);
		// 	// continue;
		// 	incompleteCase();
		// }
		// else if (infoClie[clieIdx].request.rtype == 3) // THIS case is complete and i must send the file again till done and treate each response
		// {
		// 	// the parser will return each request seprately

		// }
		

        std::cout << "server read from client " << client.fd << ": " << buffer << std::endl;
	}
    else if (reading == 0) // connection closed cleanly by the client (TCP FIN)
	{
		std::cout << "client: " << client.fd << " disconnected" << '\n';
		// std::cout << "MY CLIENT FILES:  \n" << infoClie[clieIdx].clieFile << std::endl;
		// std::cout << "THIS VECeRase >>>>: " << serv_nb + clieIdx << std::endl;
		close(client.fd);
		vecFds.erase(vecFds.begin() + serv_nb + clieIdx);
		infoClie.erase(infoClie.begin() + clieIdx);
		// std::cout << "THIS INFOCLIE ERASE >>>>: " << clieIdx << std::endl;
	}
}

void loopTools::mainLoop()
{
    while (1)
	{
		int ready = poll(vecFds.data(), vecFds.size(), -1);
		if (ready < 0)
		{
			perror("poll: ");
			break;
		}
		for (size_t i = 0; i < vecFds.size(); i++)
		{
			if (i >= serv_nb && !(vecFds[i].revents) && difftime(std::time(NULL), infoClie[i].clieTime > 30.0))
			{
				// close the connection and fds, and remove this client and continue
				continue;
			}
			else if (i < serv_nb  && vecFds[i].revents & POLLIN) // new connection arrived
			{
				// handle a client conenction
				newConnection(vecFds[i]);
				// new client file and config
				// aboutClient()
				myclients newClient;

				newClient.cliConf = linkServConf[vecFds[i].fd];
				// start count time of a client
				newClient.clieTime = std::time(NULL);
				//
				infoClie.push_back(newClient);
			}
			else if (vecFds[i].revents & POLLIN) // a client want to do smth
			{
				// handle this data on existing client
				int clieIdx = i - serv_nb;
				size_t tmp = vecFds.size();
				existClient(vecFds[i], clieIdx);
				if (tmp != vecFds.size()) // if a client disco
					i--;
			}
		}
	}
}    

loopTools::~loopTools()
{
}