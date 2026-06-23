#include "loopTools.hpp"

bool sign = true;

loopTools::loopTools(){};
void loopTools::close_fds()
{
	for (size_t i = 0; i < vecFds.size(); i++)
		close(vecFds[i].fd);
}

loopTools::loopTools(std::vector<serverConf>& servers) : serv_nb(0)
{

	for (size_t i = 0; i < servers.size(); i++) // EACH SERVER
	{
		for (size_t j = 0; j < servers[i].listen.size(); j++) // EACH LISTEN ON SERVER
		{
			int serverFd = socket(AF_INET, SOCK_STREAM, 0);
			if (serverFd < 0)
			{
				perror("socket: ");
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


bool loopTools::existClient(struct pollfd& client, int clieIdx, size_t *idx)
{
	
	char buffer[BUFFER_SZ];
	ssize_t reading = read(client.fd, buffer, sizeof(buffer) - 1); // our read is non blocking io mean if our kernel buffer is empty read will not frozen here and wait
	if (reading < 0)
		perror("read: ");
	if (reading > 0)
	{
		buffer[reading] = '\0';
		infoClie[clieIdx].clieFile += buffer; // this one accumulate buffer

        std::cout << "server read from client " << client.fd << ": \n[" << buffer << "]" << std::endl;
		infoClie[clieIdx].request.parse_request(infoClie[clieIdx].clieFile, infoClie[clieIdx].cliConf);
	}
    else if (reading == 0) // connection closed cleanly by the client (TCP FIN)
	{
		std::cout << "client: " << client.fd << " disconnected" << '\n';
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
void loopTools::mainLoop()
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

    while (sign)
	{
		int ready = poll(vecFds.data(), vecFds.size(), 1000);
		if (ready < 0)
		{	
			perror("poll: ");
			break;
		}
		for (size_t i = 0; i < vecFds.size(); i++)
		{
			if (i >= serv_nb && !(vecFds[i].revents) && (difftime(std::time(NULL), infoClie[i - serv_nb].clieTime) > 30.0))
			{
				// close the connection and fds, and remove this client and continue
				std::cout << "-----CLIENT " << vecFds[i].fd << " TIME OUT-------\n";
				// Note: --NO RESPONSE YET--
				// callRespErr()
				infoClie[i - serv_nb].resp = "HTTP/1.1 408 Request Timeout\r\nContent-Type: text/plain\r\nContent-Length: 15\r\n\r\nRequest Timeout";
				vecFds[i].events = POLLOUT;
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
				infoClie.push_back(newClient);
			}
			// RESPONSE --------->
			else if (vecFds[i].revents & POLLOUT)
			{
				// std::cout << "-------------CHECK FOR POLLOUT REVENTS-----------\n";
				ssize_t n = write(vecFds[i].fd, infoClie[i - serv_nb].resp.data() + infoClie[i - serv_nb].ofssetResp, infoClie[i - serv_nb].resp.size() - infoClie[i - serv_nb].ofssetResp);
				if (n < 0)
					perror("write: ");
				else if (n > 0)
					infoClie[i - serv_nb].ofssetResp += n;
				else if (n == 0)
					perror("write: ");
				if (infoClie[i - serv_nb].resp.size() == infoClie[i - serv_nb].ofssetResp) // writing everything
				{
					vecFds[i].events = POLLIN;
					infoClie[i - serv_nb].ofssetResp = 0;
					std::cout << "SERVER SENDING RESPONSE.. DONE\n";
					if (infoClie[i - serv_nb].request.rtype != 3)
					{
						std::cout << "client: " << vecFds[i].fd << " disconnected after response" << '\n';
						closeClient(vecFds[i].fd, i - serv_nb, &i);
					}
				}
			}
			else if (i >= serv_nb && (vecFds[i].revents & POLLIN || infoClie[i - serv_nb].request.rtype == 3)) // a client want to do smth
			{
				// handle this data on existing client
				infoClie[i - serv_nb].clieTime = std::time(NULL);
				if (infoClie[i - serv_nb].request.rtype == 3) // keep alive
				{
					infoClie[i - serv_nb].request.parse_request(infoClie[i - serv_nb].clieFile, infoClie[i - serv_nb].cliConf);
					isconnected = true;
				}
				else
					isconnected = existClient(vecFds[i], i - serv_nb, &i);
				if (isconnected && infoClie[i - serv_nb].request.rtype != 0)
				{
					infoClie[i - serv_nb].resp = "HTTP/1.1 200 OK\r\nDate: Mon, 15 Jun 2026 12:00:00 GMT\r\nContent-Type: text/plain\r\nContent-Length: 2\r\nConnection: close\r\n\r\nOK";
					vecFds[i].events = POLLOUT;
					std::cout << "set to POLLOUT\n";
				}
					// std::cout << "rtype "<< infoClie[i - serv_nb].request.rtype << "\n";
			}

		}
	}
	close_fds();
}

loopTools::~loopTools()
{
}