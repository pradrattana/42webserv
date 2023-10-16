#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <poll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <set>
#include <map>
#include <signal.h>
#include "ConfigParser.hpp"
#include "RequestParser.hpp"
#include "Response.hpp"
#include "Socket.hpp"
#include "webservStruct.hpp"

#define MAXLINE 3000

class Server
{
public:
	Server();
	Server(const ConfigParser &);
	Server(const Server &);
	~Server();
	Server &operator=(const Server &);

	void initAllSocket(const std::vector<t_serverData> &);
	void initAllFdset();
	void waiting();
	int addNewConnection(Socket *, int &);
	int checkClient(std::pair<const int, std::set<t_serverData> > &, int &);

	static bool _isRunning;

private:
	std::vector<Socket *> _allSock;
	fd_set _allSet;
	fd_set _readSet;
	int _maxFd;
	std::map<int, std::set<t_serverData> > _portToServ;
	std::map<int, std::set<t_serverData> > _cli;
};

#endif
