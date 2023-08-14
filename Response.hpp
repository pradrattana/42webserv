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
# include <set>
# include <utility>
# include "ConfigParser.hpp"
# include "RequestParser.hpp"
# include "webservStruct.hpp"
# include "helper.hpp"

class Response {
public:
	Response();
	Response(const Response &);
	Response(const std::set<t_serverData> &, const std::string &);
	~Response();
	Response &operator= (const Response &);

	const std::string	getStatusLine() const;
	const std::string	getHeadersText() const;
	const std::string	&getMessageBody() const;
	const std::string	&getResponse() const;

	const std::string	toEnv() const;

	void	setResponse();
	void	setMessageBody();

	// void	setCacheControl();
	void	setConnection();
	void	setDate();

	// void	setContentEncoding();
	// void	setContentLanguage();
	void	setContentLength();
	void	setLocation();
	// void	setContentMD5();
	// void	setContentRange();
	void	setContentType();
	// void	setExpires();
	// void	setLastModified();
	// void	setExtensionHeader();

	void	directoryListing();

	void	methodHandler();
	void	methodGet();
	void	methodPost();
	void	methodDelete();

	bool	setFullPath();
	void	setErrorPath();

	void	printFileSize(std::ostringstream &, const std::string &);
	void	printDateModified(std::ostringstream &, const std::string &);
	void	printStyle(std::ostringstream &);
	void	printTable(std::ostringstream &, std::stringstream &);


private:
	RequestParser	_request;
	t_locationData	_reqLoc; 
	t_locationData	_errLoc; 

	std::string		_fullPath;
	std::string		_response;

	int									_code;
	std::map<std::string, std::string>	_headers;
	std::string							_msgBody;

	std::map<int, std::string>	_statMaping;

	void	initStatusMapping();
	void	setRequestLocation(const std::set<t_serverData> &);
};

#endif
