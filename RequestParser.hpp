#ifndef REQUESTPARSER_HPP
# define REQUESTPARSER_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <cstdlib>
# include <cstdio>
# include <algorithm>
# include <map>
# include "ConfigParser.hpp"
# include "helper.hpp"

class RequestParser {
private:
	typedef struct reqLineData {
		std::string	method;
		std::string	path;
		std::string	query;
		std::string	version;
	}	t_reqLineData;

public:
	RequestParser();
	RequestParser(const RequestParser &);
	RequestParser(const std::string &);
	~RequestParser();
	RequestParser &operator= (const RequestParser &);

	void	readRequest(const std::string &);
	const std::string	toEnv(const ConfigParser::t_locationData &) const;

	const std::string	&getMethod() const;
	const std::string	&getPath() const;
	const std::string	&getQuery() const;
	const std::string	&getVersion() const;
	const std::map<std::string, std::string>	&getHeaders() const;


private:
	t_reqLineData	_reqLine;
	std::map<std::string, std::string>	_headers;
	std::string		_msgBody;

	void	parseRequestLine(const std::string &);
	void	parseHeaders(std::istringstream &);

};

#endif
