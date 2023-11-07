#ifndef REQUESTPARSER_HPP
# define REQUESTPARSER_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <cstdlib>
# include <cstdio>
# include <unistd.h>
# include <algorithm>
# include <map>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
# include "ConfigParser.hpp"
# include "helper.hpp"
# include "webservStruct.hpp"

# define MAXLINE 3000

class RequestParser {
private:
	typedef struct reqLineData {
		std::string	method;
		std::string	uri;
		std::string	query;
		std::string	version;
	}	t_reqLineData;

public:
	RequestParser();
	RequestParser(const RequestParser &);
	RequestParser(const std::string &);
	~RequestParser();
	RequestParser &operator= (const RequestParser &);

	size_t	readToBuf(int, char *&);
	void	readRequest(int);
	char	**toEnv(const t_locationData &, char **&);
	// void	setContentLength();

	const std::string	&getMethod() const;
	const std::string	&getUri() const;
	const std::string	&getQuery() const;
	const std::string	&getVersion() const;
	const std::map<std::string, std::string>	&getHeaders() const;
	const char		*getMessageBody() const;
	const size_t	&getMessageBodyLen() const;
	const size_t	&getReadLen() const;

	void	parseRequestLine(const std::string &);
	void	parseHeaders(std::istringstream &);
	void	parseMessageBody(int, int, char *&);

private:
	t_reqLineData	_reqLine;
	std::map<std::string, std::string>	_headers;
	char		*_msgBody;
	size_t		_msgLen;
	size_t		_readLen;
};

#endif
