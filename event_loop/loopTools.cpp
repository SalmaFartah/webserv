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

void signalHandler(int signal)
{
	(void)signal;
	sign = false;
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
int loopTools::findInVec(int fd)
{
	// find client by fd instead of index
	for (size_t i = 0; i < vecFds.size(); i++)
	{
		if (vecFds[i].fd == fd)
			return i;
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
			int outIdx = findInVec(cgidead.stdoutPipe);
			kill(cgidead.pidChild, SIGKILL);
			waitpid(cgidead.pidChild, NULL, 0);
			close(cgidead.stdoutPipe); // pollin fd one
			cgiMap.erase(cgidead.stdoutPipe); // erase from map
			if (outIdx != -1)
				vecFds.erase(vecFds.begin() + outIdx); // pollout one 
			// CGIResult cgidead = infoClie[clieIdx].request.CGIobj;
			// kill(cgidead.pidChild, SIGKILL);
			// waitpid(cgidead.pidChild, NULL, 0);

			// close(cgidead.stdinPipe);    // ← add this
			// close(cgidead.stdoutPipe);

			// int inIdx  = findInVec(cgidead.stdinPipe);   // ← add this
			// int outIdx = findInVec(cgidead.stdoutPipe);

			// if (inIdx  != -1) vecFds.erase(vecFds.begin() + inIdx);
			// if (outIdx != -1) vecFds.erase(vecFds.begin() + outIdx);

			// cgiMap.erase(cgidead.stdinPipe);   // ← add this
			// cgiMap.erase(cgidead.stdoutPipe);
		}
		
		// std::cout << "MY CLIENT FILES:  \n" << infoClie[clieIdx].clieFile << std::endl;
		closeClient(client.fd, *idx - serv_nb, idx);
		return false;
	}
	return true;
}


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
		i--;
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
		int cliIdx = findInVec(cgiRd.clie_fd);
		if (cliIdx != -1)
    		vecFds[cliIdx].events = POLLIN | POLLOUT;
		cgiMap.erase(vecFds[i].fd);
		vecFds.erase(vecFds.begin() + i);
		i--;
	}
	return true;
}

void loopTools::CgiTimout()
{
	std::map<int, struct CGIResult>::iterator it = cgiMap.begin();
	while (it != cgiMap.end())
	{
		if (difftime(std::time(NULL), it->second.start_time) > 15)
		{

			int inIdx  = findInVec(it->second.stdinPipe);
			int outIdx = findInVec(it->second.stdoutPipe);
			int xfd    = findClient(it->second.clie_fd);
			int cliIdx = findInVec(it->second.clie_fd);

			std::cout << "TIMEOUT → key: " << it->first << " clie_fd: " << it->second.clie_fd \
			<< " xfd: " << xfd << " cliIdx: " << cliIdx << "\n";
			kill(it->second.pidChild, SIGKILL);
			waitpid(it->second.pidChild, NULL, 0);
			if (cgiMap.count(it->second.stdinPipe))
				close(it->second.stdinPipe);
			
			if (cgiMap.count(it->second.stdoutPipe))
				close(it->second.stdoutPipe);
			/*----------------BUILD RESPONSE-------------*/
			realResp.routeCheck(infoClie[xfd].cliConf, infoClie[xfd].request.req, 504, it->second);
			infoClie[xfd].resp = realResp.getResponse();

			if (cliIdx != -1)
			{
				std::cout << ">>>>>>>>>>>>>>>>>>>clie fd: " << it->second.clie_fd << "\n";
				vecFds[cliIdx].events = POLLIN | POLLOUT;
			}

			/*----------------ERASE PIPES FROM VECT-------------*/
			if (cgiMap.count(it->second.stdinPipe) && inIdx != -1)
				vecFds.erase(vecFds.begin() + inIdx);

			if (cgiMap.count(it->second.stdoutPipe) && outIdx != -1)
				vecFds.erase(vecFds.begin() + outIdx);

			// erase the OTHER entry first (by key, safe)
			int other_key;
			if (it->first == it->second.stdinPipe)
				other_key = it->second.stdoutPipe;
			else
				other_key = it->second.stdinPipe;
			cgiMap.erase(other_key);
			// now erase current and advance
			it = cgiMap.erase(it);
		}
		else
			++it;
	}
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
		CgiTimout();
		for (size_t i = 0; i < vecFds.size(); i++)
		{
			int newidx = findClient(vecFds[i].fd);
			if (i >= serv_nb && !cgiMap.count(vecFds[i].fd) \
			&& !(vecFds[i].revents) && newidx != -1 && (difftime(std::time(NULL), infoClie[newidx].clieTime) > 10.0))
			{
				// close the connection and fds, and remove this client and continue
				std::cout << "-----CLIENT " << vecFds[i].fd << " TIME OUT-------\n";
				realResp.routeCheck(infoClie[newidx].cliConf, infoClie[newidx].request.req, 408, infoClie[newidx].request.CGIobj);
				infoClie[newidx].resp = realResp.getResponse();
				vecFds[i].events = POLLOUT;
				infoClie[newidx].request.rtype = infoClie[newidx].request.ERROR;
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
			else if (newidx != -1 && !cgiMap.count(vecFds[i].fd) && vecFds[i].revents & POLLOUT)
			{
				
				std::cout << "-------------CHECK FOR POLLOUT REVENTS-----------\n";
				ssize_t n = write(vecFds[i].fd, infoClie[newidx].resp.data() + infoClie[newidx].ofssetResp, infoClie[newidx].resp.size() - infoClie[newidx].ofssetResp);
				if (n < 0)
				{
					perror("write: ");
					closeClient(vecFds[i].fd, newidx, &i);
				}
				else if (n > 0)
					infoClie[newidx].ofssetResp += n;
				else if (n == 0)
					perror("write == 0: ");
				if (infoClie[newidx].resp.size() == infoClie[newidx].ofssetResp) // writing everything
				{
					vecFds[i].events = POLLIN;
					infoClie[newidx].ofssetResp = 0;
					infoClie[newidx].request.route.isCGI = false;
					std::cout << "SERVER SENDING RESPONSE.. DONE\n";
					if (infoClie[newidx].request.rtype != 0 && infoClie[newidx].request.rtype != 3)
					{
						std::cout << "client: " << vecFds[i].fd << " disconnected after response" << '\n';
						closeClient(vecFds[i].fd, newidx, &i);
					}
				}
			}
			else if (newidx != -1 && i >= serv_nb && !cgiMap.count(vecFds[i].fd) && (vecFds[i].revents & POLLIN || infoClie[newidx].request.rtype == 3)) // a client want to do smth
			{
				// handle this data on existing client
				infoClie[newidx].clieTime = std::time(NULL);
				if (infoClie[newidx].request.route.isCGI)
					continue;
				if (infoClie[newidx].request.rtype == 3) // keep alive
				{
					infoClie[newidx].resp  = infoClie[newidx].request.parse_request(infoClie[newidx].clieFile, infoClie[newidx].cliConf);
					isconnected = true;
				}
				else
					isconnected = existClient(vecFds[i], newidx, &i);
				/* if cgi is true, create struct pollfd and add those pipes to the vecFds */
				if (isconnected && infoClie[newidx].request.rtype != 0 && infoClie[newidx].request.route.isCGI)
				{
					infoClie[newidx].request.CGIobj.start_time = std::time(NULL);
					infoClie[newidx].request.CGIobj.clie_fd = vecFds[i].fd;
					addNewFd(infoClie[newidx].request.CGIobj.stdinPipe, POLLOUT);
					addNewFd(infoClie[newidx].request.CGIobj.stdoutPipe, POLLIN);
					cgiMap[infoClie[newidx].request.CGIobj.stdinPipe] = infoClie[newidx].request.CGIobj;
					cgiMap[infoClie[newidx].request.CGIobj.stdoutPipe] = infoClie[newidx].request.CGIobj;
				}
				else if (isconnected && infoClie[newidx].request.rtype != 0)
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