#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <cstring>
# include <cstdio>
# include <cstdlib>
# include <climits>
# include <unistd.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <poll.h>
# include <arpa/inet.h>
# include <netinet/in.h>
#include "RequestParser.hpp"
#include "Response.hpp"

# define PORT 8080
# define MAXLINE 3000

class Server {
public:
	Server();
	Server(const Server &);
	~Server();
	Server &operator= (const Server &);

	void	connectionWait();

	void	listeningSockCreate();
	int		newConnectionAccept(int &, int &, int &);
	void	existingConnectonCheck(const int, int &);

private:
	int	_listenFd;
	// struct pollfd		_client[FOPEN_MAX];
	int	_client[FD_SETSIZE];
	fd_set	_readSet;
	fd_set	_allSet;
	struct sockaddr_in	_servAddr;
	struct sockaddr_in	_cliAddr;
};

#endif
