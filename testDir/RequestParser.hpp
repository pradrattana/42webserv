#ifndef REQUESTPARSER_HPP
# define REQUESTPARSER_HPP

# include <iostream>
# include <sstream>
# include <string>
# include <cstdlib>
# include <map>
# include <utility>

class RequestParser {
public:
	RequestParser();
	RequestParser(const RequestParser &);
	RequestParser(const std::string &);
	~RequestParser();
	RequestParser &operator= (const RequestParser &);

	void	readRequest(const std::string &);
	void	parseRequestLine(const std::string &);
	void	parseHeaders(std::istringstream &);

	const std::string	&getMethod() const;
	const std::string	&getPath() const;
	const std::string	&getVersion() const;
	const std::map<std::string, std::string>	&getHeaders() const;

private:
	std::string	_method;
	std::string	_path;
	std::string	_version;
	std::map<std::string, std::string>	_headers;
	std::string	_msgBody;

};

#endif
