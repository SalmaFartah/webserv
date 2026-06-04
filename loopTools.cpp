#include "loopTools.hpp"

loopTools::loopTools()
{
    sockaddr_in servaddr;
	// sockaddr_in cliaddr;
	// socklen_t client_len = sizeof(cliaddr);

	servsock = socket(AF_INET, SOCK_STREAM, 0);
	int enable = 1; // 1 = ON, 0 = OFF
// used to configure various options and behaviors for a network socket, such as setting timeouts, enabling broadcasts, or reusing addresses.
// Reusing an Address (SO_REUSEADDR): Allows a socket to forcibly bind to a port in use by another socket in the TIME_WAIT state.
// SO_REUSEADDR → allows the socket address to be reused immediately, even if it is in the wait state;
	setsockopt(servsock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

// fcntl modifies the fundamental input/output (I/O) behavior of the socket descriptor
	fcntl(servsock, F_SETFL, O_NONBLOCK);

// initliaze a fresh struct and avoid any garbage
	memset(&servaddr, 0, sizeof(servaddr));
// The socket now needs to be connected to an internet port and an IP address:
	servaddr.sin_family = AF_INET; // match the socket() call -- /* internetwork: UDP, TCP, etc. */
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // /allow the server to accept a client connection on any interface
	servaddr.sin_port = htons(8080); // specify port to listen on
// attaches the socket to a specific port on the machine
	bind(servsock, (const sockaddr *)&servaddr, sizeof(servaddr));

// tells the OS to start queuing incoming connections
// backlog — how many connections can queue up waiting to be accepted
	listen(servsock, 1);
// extract the first connection request on the queue of pending connections. It creates a brand-new socket descriptor specifically for communicating with that unique client.
// the second parametre: the operating system kernel fills it in with the incoming client's network identity once a connection lands

// If your server does not care about the IP address or port of incoming clients, we can set sersock, NULL, NULL 
// accept() return a new socket descriptor if OK for communicating with the client

    struct pollfd fds;

    fds.fd = servsock; //// this second param of poll, its like how many slots of your array to read, starting from index 0, to know exactly when to stop scanning memory, without loosing cpu for all fds only the active ones
	fds.events = POLLIN; // if someone knock the door(there is a data to read) watchout
	vecFds.push_back(fds);
}

std::vector<struct pollfd> loopTools::getFds() const
{
    return vecFds;
}

void loopTools::newConnection()
{
	// handle a client conenction
	sockaddr_in cliaddr;
	socklen_t client_len = sizeof(cliaddr);

	int cli_sock = accept(servsock, (sockaddr *)&cliaddr, &client_len);
	if (cli_sock < 0)
		perror("accept: ");
	if (cli_sock >= 0)
		printf("[SERVER] New connection accepted on FD: %d\n", cli_sock);

	fcntl(cli_sock, F_SETFL, O_NONBLOCK);

//	add the master then override it with new client
	vecFds.push_back(vecFds[0]);
	vecFds.back().fd = cli_sock;
	vecFds.back().events = POLLIN;
	// inite the revents of master to 0
	vecFds[0].revents = 0;
	
}


void loopTools::existClient(int i)
{
	// HttpRequest parserObj;
	char buffer[BUFFER_SZ];
	ssize_t reading = read(vecFds[i].fd, buffer, sizeof(buffer) - 1); // our read is non blocking io mean if our kernel buffer is empty read will not frozen here and wait
	if (reading > 0)
	{
		buffer[reading] = '\0';
		
		infoClie[i - 1].clieFile += buffer; // this one accumulate buffer

		// if (parserObj.parse_request(infoClie[i - 1].clieFile))
		// handleRequest(i - 1);

        
        std::cout << "server read from client " << vecFds[i].fd << ": " << buffer << std::endl;
	}
    else if (reading == 0) // connection closed cleanly by the client (TCP FIN)
	{
		std::cout << "client: " << vecFds[i].fd << " disconnected" << '\n';
		std::cout << "my clients files:  " << infoClie[i - 1].clieFile << std::endl;
		close(vecFds[i].fd);
		vecFds.erase(vecFds.begin() + i);
		infoClie.erase(infoClie.begin() + i - 1);
		infoClie[i - 1].erase(infoClie.begin() + i - 1); //
		i--;
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
			if (vecFds[0].revents & POLLIN) // new connection arrived
			{
				// handle a client conenction
				newConnection();
				// new client file
				myclients newClient;

				infoClie.push_back(newClient);
			}
			else if (vecFds[i].revents & POLLIN) // a client want to do smth
			{
				// handle this data on existing client
				existClient(i);
				
			}
		}
	}
}    

loopTools::~loopTools()
{
}