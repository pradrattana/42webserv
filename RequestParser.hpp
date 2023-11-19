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

# define MAXLINE 10000

class RequestParser {
private:
	typedef struct reqLine
	{
		std::string	method;
		std::string	uri;
		std::string	query;
		std::string	version;
		// reqLine &operator=(const reqLine &src);
	}	t_reqLine;

public:
	RequestParser();
	RequestParser(const std::string &);
	RequestParser(const RequestParser &);
	~RequestParser();
	RequestParser &operator=(const RequestParser &);

	int		readRequest(int);
	size_t	readToBuf(int, char *&);
	size_t	readToFile(int, int);
	char	**toEnv(const t_locationData &, char **&);

	bool	parseRequestLine(const std::string &);
	bool	parseHeaders(std::istringstream &);
	void	parseMessageBody(int, int, char *&);
	void	parseMessageBody(int, int);

	const std::string	&getMethod() const;
	const std::string	&getUri() const;
	const std::string	&getQuery() const;
	const std::string	&getVersion() const;
	const std::map<std::string, std::string>	&getHeaders() const;
	// const char		*getMessageBody() const;
	const size_t	&getMessageBodyLen() const;
	// const size_t	&getReadLen() const;

private:
	t_reqLine	_reqLine;
	std::map<std::string, std::string>	_headers;
	size_t		_msgLen;
};

#endif
