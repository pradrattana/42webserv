#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <sstream>
# include <fstream>
# include <string>
# include <cstdlib>
# include <ctime>
# include <sys/stat.h>
# include <map>
# include <utility>
# include "RequestParser.hpp"

class Response {
public:
	Response();
	Response(const Response &);
	Response(const std::string &);
	~Response();
	Response &operator= (const Response &);

	void	setResponse();
	void	setStatus(const std::string &code);
	void	setHeaders();
	void	setMessageBody();

	// void	setCacheControl();
	void	setConnection();
	void	setDate();

	// void	setContentEncoding();
	// void	setContentLanguage();
	void	setContentLength();
	// void	setContentLocation();
	// void	setContentMD5();
	// void	setContentRange();
	void	setContentType();
	// void	setExpires();
	// void	setLastModified();
	// void	setExtensionHeader();

	void	methodGet();
	void	methodPost();
	void	methodDelete();

	const std::string	&getMessageBody() const;
	const std::string	&getResponse() const;


private:
	RequestParser	_request;

	std::string		_response;

	std::string		_statCode;
	std::string		_reasonPhrase;
	std::map<std::string, std::string>	_headers;
	std::string		_msgBody;

};

#endif
