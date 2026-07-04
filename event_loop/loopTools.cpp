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
	myclients newClient;
	newClient.clieTime = std::time(NULL);
	newClient.cliConf = linkServConf[server.fd];
	newClient.ofssetResp = 0;
	newClient.info_fd = cli_sock;
	infoClie.push_back(newClient);

	addNewFd(cli_sock, POLLIN);
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
	for (size_t j = 0; j < infoClie.size(); j++)
	{
		if (infoClie[j].info_fd == fd)
			return j;
	}
	return -1;
}

int loopTools::findInVec(int fd)
{
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
	ssize_t reading = read(client.fd, buffer, sizeof(buffer)); // our read is non blocking io mean if our kernel buffer is empty read will not frozen here and wait
	if (reading < 0)
	{
		perror("read: ");
		closeClient(client.fd, clieIdx, idx);
		return false;
	}
	else if (reading > 0)
	{
		infoClie[clieIdx].clieFile.append(buffer, reading);
		// std::cout << "READ FILE >> " << infoClie[clieIdx].clieFile << "\n";
		// std::cout << "size in READ FILE >> " << infoClie[clieIdx].clieFile.size() << "\n";
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
				vecFds.erase(vecFds.begin() + outIdx);
		}
		// std::cout << "MY CLIENT FILES:  \n" << infoClie[clieIdx].clieFile << std::endl;
		closeClient(client.fd, clieIdx, idx);
		return false;
	}
	return true;
}


bool loopTools::CgiWrite(CGIResult &cgiWr, size_t &i)
{
	ssize_t n = write(vecFds[i].fd, cgiWr.body.data() + cgiWr.ofssetCgi, cgiWr.body.size() - cgiWr.ofssetCgi);
	if (n < 0)
	{
		perror("writeCgi < 0 ");
		eraseChild(cgiWr, 500);
		cgiMap.erase(cgiWr.stdinPipe);
		cgiMap.erase(cgiWr.stdoutPipe);
		i -= 2;
		return false;
	}
	if (n > 0)
		cgiWr.ofssetCgi += n;
	if (n == 0)
	{
		perror("writeCgi = 0");
	}
	if (cgiWr.body.size() == cgiWr.ofssetCgi) // writing everything
	{
		std::cout << "--All Body written to the cgiChild--\n";
		close(vecFds[i].fd);
		cgiMap.erase(vecFds[i].fd);
		vecFds.erase(vecFds.begin() + i);
		i--;
	}
	return true;
}

bool loopTools::CgiRead(CGIResult& cgiRd, size_t &i)
{
	int cliIdx = findInVec(cgiRd.clie_fd);
	int xfd = findClient(cgiRd.clie_fd);
	char buffer[BUFFER_SZ];
	ssize_t reading = read(vecFds[i].fd, buffer, sizeof(buffer));
	if (reading < 0)
	{
		perror("read ");
		eraseChild(cgiRd, 500);
		cgiMap.erase(cgiRd.stdoutPipe);
		i--;
		return false;
	}
	else if (reading > 0)
		cgiRd.output.append(buffer, reading);
	else // read == 0
	{
		HttpRequest &requestCli = infoClie[xfd].request;
		int childStatus;
		waitpid(cgiRd.pidChild, &childStatus, 0);
		if (WIFEXITED(childStatus) && WEXITSTATUS(childStatus) == 0) 
			infoClie[xfd].resp = requestCli.route.Cgi.buildCGIResponse(cgiRd, requestCli.req.connection);
    	else
		{
			realResp.routeCheck(infoClie[xfd].cliConf, infoClie[xfd].request.req, 502, cgiRd);
			infoClie[xfd].resp = realResp.getResponse();
		}
		close(vecFds[i].fd); // pollin fd one
		if (cliIdx != -1)
    		vecFds[cliIdx].events = POLLIN | POLLOUT;
		cgiMap.erase(vecFds[i].fd);
		vecFds.erase(vecFds.begin() + i);
		i--;
	}
	return true;
}

void loopTools::eraseChild(CGIResult& Childinfo, int code)
{
	int CliIdx = findClient(Childinfo.clie_fd);
	int CliIdxVec = findInVec(Childinfo.clie_fd);
	int inIdx = findInVec(Childinfo.stdinPipe);

	kill(Childinfo.pidChild, SIGKILL);
	waitpid(Childinfo.pidChild, NULL, 0);
	if (cgiMap.count(Childinfo.stdinPipe))
	{
		close(Childinfo.stdinPipe);
		if (inIdx)
			vecFds.erase(vecFds.begin() + inIdx);
	}
	int outIdx = findInVec(Childinfo.stdoutPipe);
	if (cgiMap.count(Childinfo.stdoutPipe))
	{
		close(Childinfo.stdoutPipe);
		if (outIdx != -1)
			vecFds.erase(vecFds.begin() + outIdx);
	}
	if (CliIdx != -1 && CliIdxVec != -1)
	{
		realResp.routeCheck(infoClie[CliIdx].cliConf, infoClie[CliIdx].request.req, code, Childinfo);
		infoClie[CliIdx].resp = realResp.getResponse();
		vecFds[CliIdxVec].events = POLLIN | POLLOUT;
	}
}

void loopTools::CgiTimout()
{
	std::map<int, struct CGIResult>::iterator it = cgiMap.begin();
	while (it != cgiMap.end())
	{
		if (difftime(std::time(NULL), it->second.start_time) > 15)
		{
			eraseChild(it->second, 504);
			int other_key;
			if (it->first == it->second.stdinPipe)
				other_key = it->second.stdoutPipe;
			else
				other_key = it->second.stdinPipe;
			cgiMap.erase(other_key);
			it = cgiMap.erase(it);
		}
		else
			++it;
	}
}

void loopTools::shutdownCGI()
{
    std::map<int, CGIResult>::iterator it = cgiMap.begin();
    while (it != cgiMap.end())
    {
        // process only once per client (avoid double kill)
        if (it->first == it->second.stdoutPipe)
        {
            kill(it->second.pidChild, SIGKILL);
            waitpid(it->second.pidChild, NULL, 0); // ok to block here, shutting down anyway
            close(it->second.stdinPipe);
            close(it->second.stdoutPipe);
        }
        ++it;
    }
    cgiMap.clear();
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
			if (i >= serv_nb
			&& !(vecFds[i].revents) && newidx != -1 && (difftime(std::time(NULL), infoClie[newidx].clieTime) > 10.0))
			{
				realResp.routeCheck(infoClie[newidx].cliConf, infoClie[newidx].request.req, 408, infoClie[newidx].request.CGIobj);
				infoClie[newidx].resp = realResp.getResponse();
				vecFds[i].events = POLLOUT;
				infoClie[newidx].request.rtype = infoClie[newidx].request.ERROR;
			}
			else if (i < serv_nb  && vecFds[i].revents & POLLIN) // new connection arrived
				newConnection(vecFds[i]);
			else if (i >= serv_nb && cgiMap.count(vecFds[i].fd) && vecFds[i].revents & POLLOUT)
			{
				CGIResult data = cgiMap[vecFds[i].fd];
				CgiWrite(data, i);
			}
			else if (i >= serv_nb && cgiMap.count(vecFds[i].fd) && vecFds[i].revents & POLLIN)
			{
				CGIResult& data = cgiMap[vecFds[i].fd];
				CgiRead(data, i);
			}
			else if (newidx != -1 && vecFds[i].revents & POLLOUT)
			{
				ssize_t n = write(vecFds[i].fd, infoClie[newidx].resp.data() + infoClie[newidx].ofssetResp, infoClie[newidx].resp.size() - infoClie[newidx].ofssetResp);
				if (n < 0)
				{
					perror("write ");
					closeClient(vecFds[i].fd, newidx, &i);
					continue ;
				}
				else if (n > 0)
					infoClie[newidx].ofssetResp += n;
				else if (n == 0)
					perror("write ");
				if (infoClie[newidx].resp.size() == infoClie[newidx].ofssetResp) // writing everything
				{
					vecFds[i].events = POLLIN;
					infoClie[newidx].ofssetResp = 0;
					infoClie[newidx].request.route.isCGI = false;
					// std::cout << "SERVER SENDING RESPONSE..\n";
					if (infoClie[newidx].request.rtype != 0 && infoClie[newidx].request.rtype != 3)
					{
						std::cout << "client: " << vecFds[i].fd << " disconnected after response" << '\n';
						closeClient(vecFds[i].fd, newidx, &i);
					}
				}
			}
			else if (newidx != -1 && (vecFds[i].revents & POLLIN || infoClie[newidx].request.rtype == 3)) // a client want to do smth
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

					fcntl(infoClie[newidx].request.CGIobj.stdinPipe, F_SETFL, O_NONBLOCK);
					fcntl(infoClie[newidx].request.CGIobj.stdoutPipe, F_SETFL, O_NONBLOCK);
					
					cgiMap[infoClie[newidx].request.CGIobj.stdinPipe] = infoClie[newidx].request.CGIobj;
					cgiMap[infoClie[newidx].request.CGIobj.stdoutPipe] = infoClie[newidx].request.CGIobj;
					
				}
				else if (isconnected && infoClie[newidx].request.rtype != 0)
				{
					vecFds[i].events = POLLIN | POLLOUT;
					// std::cout << "set to POLLOUT\n";
				}
			}
		}
	}
	shutdownCGI();
	close_fds();
}

loopTools::~loopTools()
{
}