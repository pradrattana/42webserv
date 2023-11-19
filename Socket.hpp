#ifndef SOCKET_HPP
# define SOCKET_HPP

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
# include <vector>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "ConfigParser.hpp"
# include "webservStruct.hpp"

# define BACKLOG 1000

class Socket {
public:
	Socket();
	Socket(const t_listenData &);
	Socket(const Socket &);
	~Socket();
	Socket &operator= (const Socket &);

	void		initSocket(const t_listenData &);
	int			getNewConnection();
	const int	&getListeningFd() const;
	const int	&getListeningPort() const;

private:
	int					_lsnPort;
	int					_lsnFd;
	struct sockaddr_in	_servAddr;
	struct sockaddr_in	_cliAddr;
};

#endif
