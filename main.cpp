#include "tokenz/parse.hpp"
#include <iostream>
#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>

// #define MAX_FDS 5


int main(int ac, char *av[])
{
    if (ac > 2)
        return std::cerr << "Error: Bad Argument" << std::endl, 1;
    std::ifstream Fileconf("test.conf");
    if (ac == 2)
    	std::ifstream Fileconf(av[1]);
	if (!Fileconf)
		return std::cerr << "Error: could not open file" << std::endl, 1;
	// if the file founded with the right permission above;
	// ------------------------------------------------------------------
	conf confObj;
	confObj.read_file(Fileconf);
	// i must read all the file and tooks all the values as tokenz except whitesapces and comments;
	// --------------------------------------------------------------------------------------------

	// confObj.print_tokenz(); // if u want to Print each one
	Fileconf.close();
	/*#################--EVENT LOOP--######################*/
	sockaddr_in servaddr;
	sockaddr_in cliaddr;
	socklen_t client_len = sizeof(cliaddr);

	int servsock = socket(AF_INET, SOCK_STREAM, 0);
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
	std::vector<struct pollfd> vecFds;

	
	fds.fd = servsock;
	fds.events = POLLIN;
	vecFds.push_back(fds);

	// std::vector<std::string> wrifiles;
	// int nfds = 1; // this second param of poll, its like how many slots of your array to read, starting from index 0, to know exactly when to stop scanning memory, without loosing cpu for all fds only the active ones

	// fds[0].fd = servsock; // assign the server socket to fds[0] and waiting for a connection
	// fds[0].events = POLLIN; // if someone knock the door(there is a data to read) watchout

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
				int cli_sock = accept(servsock, (sockaddr *)&cliaddr, &client_len);
				if (cli_sock < 0)
					perror("accept: ");
				if (cli_sock >= 0)
					printf("[SERVER] New connection accepted on FD: %d\n", cli_sock);

				fcntl(cli_sock, F_SETFL, O_NONBLOCK);

				vecFds.push_back(fds);
				vecFds.back().fd = cli_sock;
				vecFds.back().events = POLLIN;
				// inite the revents of master to 0
				vecFds[0].revents = 0;
			}
			else if (vecFds[i].revents & POLLIN) // a client want to do smth
			{
				// handle this data on existing client
				// printf("IN READ...\n");
				char buffer[1024];
				ssize_t reading = read(vecFds[i].fd, buffer, sizeof(buffer) - 1); // our read is non blocking io mean if our kernel buffer is empty read will not frozen here and wait
				if (reading > 0)
				{
					// wrifiles[i] += buffer;
					buffer[reading] = '\0';
					printf("server read from client [%d]: %s", vecFds[i].fd, buffer);
				}
				else if (reading == 0) // connection closed cleanly by the client (TCP FIN)
				{
					std::cout << "client: " << vecFds[i].fd << " disconnected" << '\n';
					close(vecFds[i].fd);
					vecFds.erase(vecFds.begin() + i); // vector element = 4 so size 5; we are in element 2; remove element 2 from vector; so size will decrement to 4 and elements became 3 
					i--;
				}
				
			}
		}
		

		// if (bytes_read > 0) 
		// {
		// 	printf("[SERVER] Client A sent: %s", buffer);
		// 	// Send a quick thank you message back to Client A
		// 	write(cli_sock, "Message received!\n", 18);
		// }
	}




}