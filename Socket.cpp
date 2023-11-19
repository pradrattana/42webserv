#include "Socket.hpp"

Socket::Socket(void)
{
}

Socket::Socket(const t_listenData &lsn)
{
	_lsnPort = lsn.port;
	initSocket(lsn);
}

Socket::Socket(const Socket &src)
{
	*this = src;
}

Socket::~Socket(void)
{
}

Socket &Socket::operator=(const Socket &src)
{
	if (this != &src)
	{
		_lsnPort = src._lsnPort;
		_lsnFd = src._lsnFd;
		_servAddr = src._servAddr;
		_cliAddr = src._cliAddr;
	}
	return *this;
}

void Socket::initSocket(const t_listenData &lsn)
{
	// creates socket to perform network I/O
	if ((_lsnFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	fcntl(_lsnFd, F_SETFL, O_NONBLOCK);
	// int    on = 1;
	// ioctl(_lsnFd, FIONBIO, (char *)&on);

	bzero((char *)&_servAddr, sizeof(_servAddr));
	_servAddr.sin_family = AF_INET;
	_servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_servAddr.sin_port = htons(lsn.port);

	// assigns a local protocol address to a socket
	setsockopt(_lsnFd, SOL_SOCKET, SO_REUSEADDR, (char *)&_servAddr, sizeof(_servAddr));
	if (bind(_lsnFd, (struct sockaddr *)&_servAddr,
			 sizeof(_servAddr)) < 0)
	{
		std::ostringstream oss;
		oss << "Bind " << lsn.port << " failed";
		perror(oss.str().c_str());
		// exit(EXIT_FAILURE);
	}

	// converts an unconnected socket into a passive socket
	if (listen(_lsnFd, BACKLOG) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
}

int Socket::getNewConnection()
{
	int cliAddrLen = sizeof(_cliAddr);
	int newFd = accept(_lsnFd, (struct sockaddr *)&_cliAddr,
					   (socklen_t *)&cliAddrLen);
	if (newFd < 0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}
	return newFd;
}

const int &Socket::getListeningFd() const
{
	return _lsnFd;
}

const int &Socket::getListeningPort() const
{
	return _lsnPort;
}

// const std::string	Socket::toEnv() const {
// 	std::ostringstream		oss;
// 	// std::string				key;
// 	// std::string::size_type	pos;
// 	char	str[INET_ADDRSTRLEN];

// 	oss << "REMOTE_ADDR=\"" << inet_ntop(AF_INET, &_cliAddr.sin_addr, str, INET_ADDRSTRLEN) << "\"\n";
// 	oss << "REMOTE_PORT=\"" << ntohs(_cliAddr.sin_port) << "\"\n";

// 	return oss.str();
// }
