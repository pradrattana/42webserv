#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <iomanip>
# include <sstream>
# include <fstream>
# include <string>
# include <cstdlib>
# include <ctime>
# include <sys/stat.h>
# include <dirent.h>
# include <map>
# include <vector>
# include <utility>
# include <algorithm>
# include "ConfigParser.hpp"
# include "RequestParser.hpp"
# include "CgiHandler.hpp"
# include "webservStruct.hpp"
# include "helper.hpp"

class Response {
public:
	Response();
	Response(const Response &);
	~Response();
	Response &operator= (const Response &);

	bool	processing(const t_serverData &, int);

	void	setResponse();
	void	setMessageBody();
	void	setMessageBody(std::istream &);
	void	setMessageBody(const std::vector<char> &);
	void	setHeader(const std::string &, const std::string &);
	void	setCode(const int);

	bool	setFullPath();
	void	setErrorPath();

	void	setDate();
	void	setContentLength();
	void	setLocation();
	void	setLocation(const std::string &);
	void	setContentType();

	void	directoryListing();

	void	methodHandler();
	void	methodGet();
	void	methodPost();
	void	methodDelete();

	char	**toEnv(char **&);

	const std::string	getStatusLine() const;
	const std::map<std::string, std::string>	&getHeaders() const;
	const std::string	getHeadersText() const;
	const std::vector<char>	&getResponse() const;
	const RequestParser	&getRequest() const;

	// DirListing
	void	printFileSize(std::stringstream &, const std::string &);
	void	printDateModified(std::stringstream &, const std::string &);
	void	printStyle(std::stringstream &);
	void	printTable(std::stringstream &, std::stringstream &);

private:
	CgiHandler		_cgi;
	RequestParser	_request;
	t_locationData	_reqLoc;

	std::string			_fullPath;
	std::vector<char>	_response;

	int					_code;
	std::map<std::string, std::string>	_headers;
	std::vector<char>	_msgBody;

	std::map<int, std::string>	_statMaping;

	void	initStatusMapping();
	void	setRequestLocation(const t_serverData &);
};

std::string	dropFilename(const std::string &);

#endif
