#include "Server.hpp"

bool Server::_isRunning = true;

Server::Server(void)
{
	// std::cout << "Default constructor called by <Server>" << std::endl;
}

Server::Server(const ConfigParser &conf)
{
	initAllSocket(conf.getServer());
	initAllFdset();
	waiting();
}

Server::Server(const Server &src)
{
	// std::cout << "Copy constructor called by <Server>" << std::endl;
	*this = src;
}

Server::~Server(void)
{
	// std::cout << "Destructor called by <Server>" << std::endl;
}

Server &Server::operator=(const Server &src)
{
	// std::cout << "Copy assignment operator called by <Server>" << std::endl;
	_allSock = src._allSock;
	_allSet = src._allSet;
	_readSet = src._readSet;
	_maxFd = src._maxFd;
	_portToServ = src._portToServ;
	_cli = src._cli;
	return *this;
}

void Server::initAllSocket(const std::vector<t_serverData> &allServ)
{
	std::vector<int> tmpPort;

	for (std::vector<t_serverData>::const_iterator it = allServ.begin();
		 it != allServ.end(); it++)
	{
		for (std::vector<t_locationData>::const_iterator it2 = it->location.begin();
			 it2 != it->location.end(); it2++)
		{
			for (std::vector<t_listenData>::const_iterator it3 = it2->listen.begin();
				 it3 != it2->listen.end(); it3++)
			{
				_portToServ[it3->port].insert(*it);
				if (std::find(tmpPort.begin(), tmpPort.end(), it3->port) == tmpPort.end())
				{
					tmpPort.push_back(it3->port);
					_allSock.push_back(new Socket(*it3));
				}
			}
		}
	}
}

void Server::initAllFdset()
{
	int lsnFd;

	FD_ZERO(&_allSet);
	for (std::vector<Socket *>::iterator it = _allSock.begin();
		 it != _allSock.end(); it++)
	{
		lsnFd = (*it)->getListeningFd();
		FD_SET(lsnFd, &_allSet);
		if (lsnFd > _maxFd)
			_maxFd = lsnFd;
	}
}

static void sigHandler(int signo)
{
	if (signo == SIGINT || signo == SIGQUIT)
	{
		std::cout << "\b\b \b\b";
		Server::_isRunning = false;
	}
}

void Server::waiting()
{
	int nready;
	// struct timeval       timeout;

	signal(SIGINT, sigHandler);
	signal(SIGQUIT, sigHandler);
	// timeout.tv_sec  = 0; //3 * 60;
	// timeout.tv_usec = 0;

	while (_isRunning)
	{
		_readSet = _allSet;
		nready = select(_maxFd + 1, &_readSet, NULL, NULL, NULL);
		if (nready >= 0)
		{
			for (std::vector<Socket *>::iterator it = _allSock.begin();
				 it != _allSock.end(); it++)
			{
				if (FD_ISSET((*it)->getListeningFd(), &_readSet))
				{
					if (addNewConnection(*it, nready))
						break;
				}
			}
			for (std::map<int, std::set<t_serverData> >::iterator it = _cli.begin();
				 it != _cli.end();)
			{
				std::map<int, std::set<t_serverData> >::iterator itNext = ++it;
				it--;
				if (checkClient(*it, nready))
					break;
				it = itNext;
			}
		}
	}
	//clear memory in _allSock
	for (std::vector<Socket *>::iterator it = _allSock.begin();
		 it != _allSock.end(); it++)
		delete (*it);
	std::cout << "Server shutdown\n";
}

int Server::addNewConnection(Socket *s, int &nready)
{
	const int newFd = s->getNewConnection();

	_cli[newFd] = _portToServ[s->getListeningPort()];
	if (_cli.size() >= FD_SETSIZE)
	{
		perror("too many clients");
		exit(EXIT_FAILURE);
	}

	FD_SET(newFd, &_allSet);
	if (newFd > _maxFd)
		_maxFd = newFd;

	return (--nready <= 0);
}

const char *successResponse =
	"HTTP/1.1 200 OK\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<html><body>File received successfully</body></html>";


int Server::checkClient(std::pair<const int, std::set<t_serverData> > &fdToServ, int &nready)
{
	// char buf[MAXLINE] = {0};
	int sockfd;
	(void)nready;

	fcntl(fdToServ.first, F_SETFL, O_NONBLOCK);
	// fcntl(fdToServ.first, F_SETFF, FNDELAY);
	if ((sockfd = fdToServ.first) < 0)
		return 0;
	// ioctl(sockfd, FIONREAD, &readLen);
	if (FD_ISSET(sockfd, &_readSet))
	{
		std::ostringstream num;
		num << sockfd;
		std::cout << "sockd fd : " << sockfd << std::endl;
		char *sock = new char[num.str().length() + 1];
		strcpy(sock,const_cast<char *>(num.str().c_str()));
		char cmd[18] = "./cgi-bin/testcgi";
		int pid = fork();
		if (pid == 0) {
			char *arg[4] = {cmd, sock, sock, NULL};
			if (execve(arg[0], &arg[0], NULL) == -1){
				perror("");
				std::cout << "execve failed\n";
			}
		}
		waitpid(pid, NULL, 0);
		delete[] sock;
		FD_CLR(sockfd, &_allSet);
		close(sockfd);
		_cli.erase(sockfd);
		// // std::cout << "cli accept fd " << sockfd << "\n";
		// if ((readLen = read(sockfd, buf, MAXLINE)) == 0)
		// {
		// 	std::cout << "readlen=0 close=";
		// 	std::cout << close(sockfd) << "\n";
		// 	FD_CLR(sockfd, &_allSet);
		// 	close(sockfd);
		// 	_cli.erase(sockfd);
		// }
		// else if (readLen < 0)
		// {
		// 	if (errno == EAGAIN || errno == EWOULDBLOCK) {
        //     // Handle the case when the operation would block
        //     	std::cout << "No data available right now." << std::endl;
       	// 	} else {
        //     // Handle other possible errors
        //     	perror("Error while reading");
        //     return 1;
        // }
		// }
		// else
		// {
		// 	Response rp(fdToServ.second, (std::string(buf)));
		// 	const char *response = rp.getResponse().c_str();

		// 	std::cout << "Request\n"
		// 			  << buf << "\n";
		// 	write(sockfd, response, strlen(response));
		// }
		return (--nready <= 0);
	}
	return 0;
}
