// main src: https://notes.shichao.io/unp/ch6/#tcp-echo-server-revisited
// The only part I modified was on line 121

#include "Server.hpp"

Server::Server(void) {
	// std::cout << "Default constructor called by <Server>" << std::endl;
	listeningSockCreate();
	connectionWait();
}

Server::Server(const Server &src) {
	// std::cout << "Copy constructor called by <Server>" << std::endl;
	*this = src;
}

Server::~Server(void) {
	// std::cout << "Destructor called by <Server>" << std::endl;
}

Server &Server::operator= (const Server &src) {
	// std::cout << "Copy assignment operator called by <Server>" << std::endl;
	(void)src;
	return *this;
}

void	Server::listeningSockCreate() {
	if ((_listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket");
		exit(EXIT_FAILURE);
	}

	bzero((char *)&_servAddr, sizeof(_servAddr)); 
	_servAddr.sin_family = AF_INET; 
	_servAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	_servAddr.sin_port = htons(PORT);

	if (bind(_listenFd, (struct sockaddr *)&_servAddr,
				sizeof(_servAddr)) < 0) {
		perror("Bind failed"); 
		exit(EXIT_FAILURE);
	}

	if (listen(_listenFd, 5) < 0) {
		perror("In listen");
		exit(EXIT_FAILURE);
	}
}

void	Server::connectionWait() {
	int	maxCli = -1;                  /* index into client[] array */
	for (int i = 0; i < FD_SETSIZE; i++)
		_client[i] = -1;         /* -1 indicates available entry */
	FD_ZERO(&_allSet);
	FD_SET(_listenFd, &_allSet);
	int	maxfd = _listenFd;           /* initialize */

	while (1) {
		_readSet = _allSet;      /* structure assignment */
		int	nready = select(maxfd + 1, &_readSet, NULL, NULL, NULL);

		if (FD_ISSET(_listenFd, &_readSet)) {    /* new client connection */
			if (newConnectionAccept(maxCli, maxfd, nready))
				continue;
		}
		existingConnectonCheck(maxCli, nready);
	}
}

int	Server::newConnectionAccept(int &maxCli, int &maxfd, int &nready) {
	int	cliAddrLen = sizeof(_cliAddr);
	int	newFd = accept(_listenFd, (struct sockaddr *)&_cliAddr,
						(socklen_t*)&cliAddrLen);

	char	str[INET_ADDRSTRLEN];
	printf("new client: %s, port %d\n",
			inet_ntop(AF_INET, &_cliAddr.sin_addr, str, INET_ADDRSTRLEN),
			ntohs(_cliAddr.sin_port));

	int	i;
	for (i = 0; i < FD_SETSIZE; i++) {
		if (_client[i] < 0) {
			_client[i] = newFd;
			break ;
		}
	}
	if (i == FD_SETSIZE) {
		perror("too many clients");
		exit(EXIT_FAILURE);
	}

	FD_SET(newFd, &_allSet);    /* add new descriptor to set */
	if (newFd > maxfd)
		maxfd = newFd;         /* for select */
	if (i > maxCli)
		maxCli = i;               /* max index in client[] array */

	return (--nready <= 0);      /* no more readable descriptors */
}

void	Server::existingConnectonCheck(const int maxCli, int &nready) {
	int		sockfd, readLen;

	for (int i = 0; i <= maxCli; i++) {   /* check all clients for data */
		char	buf[MAXLINE] = {0};

		if ((sockfd = _client[i]) < 0)
			continue ;

		if (FD_ISSET(sockfd, &_readSet)) {
			if ((readLen = read(sockfd, buf, MAXLINE)) == 0) {
				/* connection closed by client */
				close(sockfd);
				FD_CLR(sockfd, &_allSet);
				_client[i] = -1;
			} else if (readLen < 0) {

			} else {
				std::cout << buf << "\n";

// This is the part where I mentioned above
				Response	rp((std::string(buf)));
				std::cout << rp.getResponse() << "\n";
				const char *hello = rp.getResponse().c_str();
				write(sockfd, hello, strlen(hello));
// end

			}

			if (--nready <= 0)    /* no more readable descriptors */
				break;
		}
	}
}
