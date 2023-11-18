#include "Server.hpp"

bool Server::_isRunning = true;

static void sigHandler(int signo)
{
	if (signo == SIGINT || signo == SIGQUIT)
	{
		std::cout << "\b\b \b\b";
		Server::_isRunning = false;
	}
}

Server::Server(void)
{
}

Server::Server(const ConfigParser &conf)
{
	initAllSocket(conf.getServer());
	initAllFdset();
	waiting();
}

Server::Server(const Server &src)
{
	*this = src;
}

Server::~Server(void)
{
}

Server &Server::operator=(const Server &src)
{
	if (this != &src)
	{
		_allSock = src._allSock;
		_allSet = src._allSet;
		_readSet = src._readSet;
		_maxFd = src._maxFd;
		_portToServ = src._portToServ;
		_cli = src._cli;
	}
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
				_portToServ[it3->port] = *it;
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

void Server::waiting()
{
	int nready;

	signal(SIGINT, sigHandler);
	signal(SIGQUIT, sigHandler);
	while (_isRunning)
	{
		_readSet = _allSet;
		nready = select(_maxFd + 1, &_readSet, &_writeSet, &_exceptSet, NULL);
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
			for (std::map<int, t_serverData>::iterator it = _cli.begin();
				 it != _cli.end();)
			{
				std::map<int, t_serverData>::iterator itNext = ++it;
				it--;
				if (checkClient(*it, nready)) {
					std::cerr << "client: " << it->first << '\n';
					break;
				}
				it = itNext;
			}
		}
	}
	//clear all bit fd in _cli
	// for (std::map<int, std::set<t_serverData> >::iterator it = _cli.begin();
	// 	 it != _cli.end();)
	// {
	// 	std::map<int, std::set<t_serverData> >::iterator itNext = ++it;
	// 	it--;
	// 	int sockfd;
	// 	if (FD_ISSET(sockfd, &_readSet))
	// 	{
	// 		std::cout << close(sockfd) << "\n";
	// 		FD_CLR(sockfd, &_allSet);
	// 		_cli.erase(sockfd);
	// 	}
	// 	it = itNext;
	// }
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

int Server::checkClient(std::pair<const int, t_serverData> &fdToServ, int &nready)
{
	int 	sockfd;

	if ((sockfd = fdToServ.first) < 0)
	{
		return 0;
	}

	if (FD_ISSET(sockfd, &_readSet))
	{
		Response rp;
		if (!rp.processing(fdToServ.second, sockfd))
		{
			std::cout << "erase cli " << sockfd << std::endl;
			FD_CLR(sockfd, &_allSet);
			_cli.erase(sockfd);
			return 0;
		}
		std::cout << "send response\n";
		FD_SET(sockfd, &_writeSet);
		if (FD_ISSET(sockfd, &_writeSet))
		// write(1, rp.getResponse().data(), rp.getResponse().size());
			write(sockfd, rp.getResponse().data(), rp.getResponse().size());
		// else
			// FD_SET(sockfd, &_writeSet);

		return (--nready <= 0);
	}
	return 0;
}
