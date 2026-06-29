#include "loopTools.hpp"

bool sign = true;

loopTools::loopTools(){};
void loopTools::close_fds()
{
	for (size_t i = 0; i < vecFds.size(); i++)
		close(vecFds[i].fd);
}

loopTools::loopTools(std::vector<serverConf>& servers) : isconnected(false), serv_nb(0)
{

	for (size_t i = 0; i < servers.size(); i++) // EACH SERVER
	{
		for (size_t j = 0; j < servers[i].listen.size(); j++) // EACH LISTEN ON SERVER
		{
			int serverFd = socket(AF_INET, SOCK_STREAM, 0);
			if (serverFd < 0)
			{
				perror("socket: ");
				close_fds();
				throw std::runtime_error("");
			}
			serv_nb++;
			
			sockaddr_in servaddr;
			// initliaze a fresh struct and avoid any garbage
			memset(&servaddr, 0, sizeof(servaddr));

		// The socket now needs to be connected to an internet port and an IP address:
			servaddr.sin_family = AF_INET; // match the socket() call -- /* internetwork: UDP, TCP, etc. */
			servaddr.sin_addr.s_addr = inet_addr(servers[i].listen[j].first.c_str()); // /allow the server to accept a client connection on any interface
			servaddr.sin_port = htons(servers[i].listen[j].second); // specify port to listen on

		// used to configure various options and behaviors for a network socket, such as setting timeouts, enabling broadcasts, or reusing addresses.
			int enable = 1; // 1 = ON, 0 = OFF
		// Reusing an Address (SO_REUSEADDR): Allows a socket to forcibly bind to a port in use by another socket in the TIME_WAIT state.
		// SO_REUSEADDR → allows the socket address to be reused immediately, even if it is in the wait state;
			setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

		// fcntl modifies the fundamental input/output (I/O) behavior of the socket descriptor
			fcntl(serverFd, F_SETFL, O_NONBLOCK);
			
			struct pollfd fds;

			fds.fd = serverFd; //// this second param of poll, its like how many slots of your array to read, starting from index 0, to know exactly when to stop scanning memory, without loosing cpu for all fds only the active ones
			fds.events = POLLIN; // if someone knock the door(there is a data to read) watchout

			vecFds.push_back(fds);

			// attaches the socket to a specific port on the machine
			if (bind(serverFd, (const sockaddr *)&servaddr, sizeof(servaddr)) < 0)
			{
				perror("bind: ");
				close_fds();
				throw std::runtime_error("");
			}
			
		// tells the OS to start queuing incoming connections
		// backlog — how many connections can queue up waiting to be accepted
			if (listen(serverFd, 128) < 0)
			{
				perror("listen: ");
				close_fds();
				throw std::runtime_error("");
			}
			linkServConf[serverFd] = &servers[i];

		}
	}
	
}
void loopTools::closeClient(int fd, int clieIdx, size_t *i)
{
	close(fd);
	infoClie.erase(infoClie.begin() + clieIdx);
	vecFds.erase(vecFds.begin() + *i);
	(*i)--;
}

void loopTools::newConnection(struct pollfd& server)
{
	// handle a client conenction
	struct pollfd client;
	sockaddr_in cliaddr;
	socklen_t client_len = sizeof(cliaddr);
	// extract the first connection request on the queue of pending connections. It creates a brand-new socket descriptor specifically for communicating with that unique client.
	// the second parametre: the operating system kernel fills it in with the incoming client's network identity once a connection lands
	// If your server does not care about the IP address or port of incoming clients, we can set sersock, NULL, NULL 
	int cli_sock = accept(server.fd, (sockaddr *)&cliaddr, &client_len);
	if (cli_sock < 0)
		perror("accept: ");
	if (cli_sock >= 0)
		printf("[SERVER] New connection accepted on FD: %d\n", cli_sock);

	fcntl(cli_sock, F_SETFL, O_NONBLOCK);

	
//	add the master then override it with new client
	vecFds.push_back(client);
	vecFds.back().fd = cli_sock;
	vecFds.back().events = POLLIN;
	
}

void loopTools::addNewFd(int fd, short event)
{
	struct pollfd newFd;
	newFd.fd = fd;
	newFd.events = event;
	vecFds.push_back(newFd);

	
}
int loopTools::findClient(int fd)
{
	// find client by fd instead of index
	for (size_t j = 0; j < infoClie.size(); j++)
	{
		if (infoClie[j].info_fd == fd)
			return j;
	}
	return -1;
}
bool loopTools::existClient(struct pollfd& client, int clieIdx, size_t *idx)
{
	
	char buffer[BUFFER_SZ];
	ssize_t reading = read(client.fd, buffer, sizeof(buffer) - 1); // our read is non blocking io mean if our kernel buffer is empty read will not frozen here and wait
	if (reading < 0)
	{
		perror("read: ");
		closeClient(client.fd, *idx - serv_nb, idx);
		return false;
	}
	else if (reading > 0)
	{
		buffer[reading] = '\0';
		infoClie[clieIdx].clieFile += buffer; // this one accumulate buffer

        std::cout << "server read from client " << client.fd << ": \n[" << buffer << "]" << std::endl;
		infoClie[clieIdx].resp  = infoClie[clieIdx].request.parse_request(infoClie[clieIdx].clieFile, infoClie[clieIdx].cliConf);
	}
    else if (reading == 0) // connection closed cleanly by the client (TCP FIN)
	{
		std::cout << "client: " << client.fd << " disconnected" << '\n';
		if (infoClie[clieIdx].request.route.isCGI)
		{
			CGIResult cgidead = infoClie[clieIdx].request.CGIobj;
			kill(cgidead.pidChild, SIGKILL);
			waitpid(cgidead.pidChild, NULL, 0);
			close(cgidead.stdoutPipe); // pollin fd one
			cgiMap.erase(cgidead.stdoutPipe); // erase from map
			vecFds.erase(vecFds.begin() + cgidead.idxOut - 1); // pollout one 
		}
		
		// std::cout << "MY CLIENT FILES:  \n" << infoClie[clieIdx].clieFile << std::endl;
		closeClient(client.fd, *idx - serv_nb, idx);
		return false;
	}
	return true;
}
void signalHandler(int signal)
{
	(void)signal;
	sign = false;
}

// void loopTools::closeCgi(CGIResult& cgi, int fd, size_t &i)
// {
// 	kill(cgi.pidChild, SIGKILL);
// 	close(vecFds[i].fd);
// 	vecFds.erase(vecFds.begin() + i);
// }
bool loopTools::CgiWrite(CGIResult& cgiWr, size_t &i)
{
	std::cout << ">>>>>>>>>HERE\n";
	std::cout << "In EXECUTE " << cgiWr.body << "\n";
	ssize_t n = write(vecFds[i].fd, cgiWr.body.data() + cgiWr.ofssetCgi, cgiWr.body.size() - cgiWr.ofssetCgi);
	if (n < 0)
	{
		perror("write: ");
		kill(cgiWr.pidChild, SIGKILL);
		close(vecFds[i].fd); // pollout fd one
		close(vecFds[i + 1].fd); // pollin fd one
		vecFds.erase(vecFds.begin() + i); // pollout one 
		vecFds.erase(vecFds.begin() + i); // pollin one
		// cgiWr.ofssetCgi = 0;
		// cgiWr.output.clear();
		cgiMap.erase(vecFds[i].fd);
		cgiMap.erase(vecFds[i + 1].fd); // pollin fd one
		i -= 2;
		return false;
	}
	if (n > 0)
		cgiWr.ofssetCgi += n;
	if (n == 0)
	{
		std::cout << "in cgiwrite >>\n";
		perror("writeCgi: ");
	}
	if (cgiWr.body.size() == cgiWr.ofssetCgi) // writing everything
	{
		std::cout << "cgiWrite read all\n";
		std::cout << "fd in cgiwrite: " << vecFds[i].fd << "\n";
		std::cout << "index cgiWrite: " << i << "\n";
		close(vecFds[i].fd);
		cgiMap.erase(vecFds[i].fd);
		vecFds.erase(vecFds.begin() + i);
	}
	return true;
}

bool loopTools::CgiRead(CGIResult& cgiRd, size_t &i)
{
	char buffer[BUFFER_SZ];
		std::cout << "IN CGI READ\n";
	ssize_t reading = read(vecFds[i].fd, buffer, sizeof(buffer)); // our read is non blocking io mean if our kernel buffer is empty read will not frozen here and wait
	if (reading < 0)
	{
		perror("read: ");
		kill(cgiRd.pidChild, SIGKILL);
		close(vecFds[i].fd); // pollin fd one
		vecFds.erase(vecFds.begin() + i);
		cgiRd.ofssetCgi = 0;
		cgiRd.output.clear();
		cgiMap.erase(vecFds[i].fd);
		i--;
		return false;
	}
	else if (reading > 0)
	{
		cgiRd.output.append(buffer, reading); // this one accumulate buff
	}
	else // read == 0
	{
		// std::cout << "IN CGI READ = 0\n";
		std::cout << "fd clie : " << cgiRd.clie_fd << "\n";
		int xfd = findClient(cgiRd.clie_fd);
		HttpRequest &requestCli = infoClie[xfd].request;
		std::cout << "idx: " << xfd << " infoClie size: " << infoClie.size() << "\n";
		std::cout << "output size: " << cgiRd.output.size() << "\n";
		std::cout << "output: " << cgiRd.output << "\n";
		infoClie[xfd].resp = requestCli.route.Cgi.buildCGIResponse(cgiRd, requestCli.req.connection);
		waitpid(cgiRd.pidChild, NULL, 0);
		close(vecFds[i].fd); // pollin fd one
		cgiRd.ofssetCgi = 0;
		cgiRd.output.clear();
		// set client to pollout

		// std::cout << "index client: " << xfd + serv_nb << "\n";
		vecFds[xfd + serv_nb].events = POLLIN | POLLOUT;
		cgiMap.erase(vecFds[i].fd);
		vecFds.erase(vecFds.begin() + i);
	}
	return true;
}
void loopTools::mainLoop()
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

    while (sign)
	{
		int ready = poll(vecFds.data(), vecFds.size(), 1000);
		if (ready < 0)
			break;
		for (size_t i = 0; i < vecFds.size(); i++)
		{
			if (i >= serv_nb && !cgiMap.count(vecFds[i].fd) && !(vecFds[i].revents) && (difftime(std::time(NULL), infoClie[i - serv_nb].clieTime) > 30.0))
			{
				// close the connection and fds, and remove this client and continue
				std::cout << "-----CLIENT " << vecFds[i].fd << " TIME OUT-------\n";
				realResp.routeCheck(infoClie[i - serv_nb].cliConf, infoClie[i - serv_nb].request.req, 408, infoClie[i - serv_nb].request.CGIobj);
				infoClie[i - serv_nb].resp = realResp.getResponse();
				vecFds[i].events = POLLOUT;
				infoClie[i - serv_nb].request.rtype = infoClie[i - serv_nb].request.ERROR;
				continue;
			}
			else if (i < serv_nb  && vecFds[i].revents & POLLIN) // new connection arrived
			{
				// handle a client conenction
				myclients newClient;
				// start count time of a client
				newClient.clieTime = std::time(NULL);
				newConnection(vecFds[i]);
				// new client file and config
				newClient.cliConf = linkServConf[vecFds[i].fd];
				newClient.ofssetResp = 0;
				newClient.info_fd = vecFds.back().fd;
				infoClie.push_back(newClient);
			}
			// RESPONSE --------->
			else if (i >= serv_nb && cgiMap.count(vecFds[i].fd) && vecFds[i].revents & POLLOUT)
			{
				CGIResult &data = cgiMap[vecFds[i].fd];
				if (!CgiWrite(data, i)) // error in write
				{
					int xfd = findClient(data.clie_fd);
					realResp.routeCheck(infoClie[xfd].cliConf, infoClie[xfd].request.req, 500, data);
					infoClie[xfd].resp = realResp.getResponse();
					vecFds[serv_nb + xfd].events = POLLOUT;
				}
			}
			else if (i >= serv_nb && cgiMap.count(vecFds[i].fd) && vecFds[i].revents & POLLIN)
			{
				CGIResult &data = cgiMap[vecFds[i].fd];
				if (!CgiRead(data, i))
				{
					int xfd = findClient(data.clie_fd);
					realResp.routeCheck(infoClie[xfd].cliConf, infoClie[xfd].request.req, 500, data);
					infoClie[xfd].resp = realResp.getResponse();
				}
			}
			else if (!cgiMap.count(vecFds[i].fd) && vecFds[i].revents & POLLOUT)
			{
				
				std::cout << "-------------CHECK FOR POLLOUT REVENTS-----------\n";
				ssize_t n = write(vecFds[i].fd, infoClie[i - serv_nb].resp.data() + infoClie[i - serv_nb].ofssetResp, infoClie[i - serv_nb].resp.size() - infoClie[i - serv_nb].ofssetResp);
				if (n < 0)
				{
					perror("write: ");
					closeClient(vecFds[i].fd, i - serv_nb, &i);
				}
				else if (n > 0)
					infoClie[i - serv_nb].ofssetResp += n;
				else if (n == 0)
					perror("write == 0: ");
				if (infoClie[i - serv_nb].resp.size() == infoClie[i - serv_nb].ofssetResp) // writing everything
				{
					vecFds[i].events = POLLIN;
					infoClie[i - serv_nb].ofssetResp = 0;
					infoClie[i - serv_nb].request.route.isCGI = false;
					std::cout << "SERVER SENDING RESPONSE.. DONE\n";
					if (infoClie[i - serv_nb].request.rtype != 0 && infoClie[i - serv_nb].request.rtype != 3)
					{
						std::cout << "client: " << vecFds[i].fd << " disconnected after response" << '\n';
						closeClient(vecFds[i].fd, i - serv_nb, &i);
					}
				}
			}
			else if (i >= serv_nb && !cgiMap.count(vecFds[i].fd) && (vecFds[i].revents & POLLIN || infoClie[i - serv_nb].request.rtype == 3)) // a client want to do smth
			{
				// handle this data on existing client
				infoClie[i - serv_nb].clieTime = std::time(NULL);
				if (infoClie[i - serv_nb].request.route.isCGI)
					continue;
				if (infoClie[i - serv_nb].request.rtype == 3) // keep alive
				{
					infoClie[i - serv_nb].resp  = infoClie[i - serv_nb].request.parse_request(infoClie[i - serv_nb].clieFile, infoClie[i - serv_nb].cliConf);
					isconnected = true;
				}
				else
					isconnected = existClient(vecFds[i], i - serv_nb, &i);
				/* if cgi is true, create struct pollfd and add those pipes to the vecFds */
				if (isconnected && infoClie[i - serv_nb].request.rtype != 0 && infoClie[i - serv_nb].request.route.isCGI)
				{
					infoClie[i - serv_nb].request.CGIobj.start_time = std::time(NULL);
					infoClie[i - serv_nb].request.CGIobj.clie_fd = vecFds[i].fd;
					addNewFd(infoClie[i - serv_nb].request.CGIobj.stdinPipe, POLLOUT);
					addNewFd(infoClie[i - serv_nb].request.CGIobj.stdoutPipe, POLLIN);
					infoClie[i - serv_nb].request.CGIobj.idxIn = i + 1;
					infoClie[i - serv_nb].request.CGIobj.idxOut = i + 2;
					cgiMap[infoClie[i - serv_nb].request.CGIobj.stdinPipe] = infoClie[i - serv_nb].request.CGIobj;
					cgiMap[infoClie[i - serv_nb].request.CGIobj.stdoutPipe] = infoClie[i - serv_nb].request.CGIobj;
				}
				else if (isconnected && infoClie[i - serv_nb].request.rtype != 0)
				{
					vecFds[i].events = POLLIN | POLLOUT;
					std::cout << "set to POLLOUT\n";
				}
			}
		}
	}
	close_fds();
}

loopTools::~loopTools()
{
}