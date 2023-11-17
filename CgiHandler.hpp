#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <iostream>
# include <sstream>
# include <fstream>
# include <string>
# include <cstring>
# include <cstdlib>
# include <unistd.h>
# include <vector>
# include <map>
# include <algorithm>
# include <sys/wait.h>
# include "helper.hpp"
# include "webservStruct.hpp"
# include "RequestParser.hpp"

class Response;

class CgiHandler {
public:
	CgiHandler();
	CgiHandler(const CgiHandler &);
	CgiHandler(Response *);
	~CgiHandler();
	CgiHandler &operator=(const CgiHandler &);

	void	executeCgi(const std::string &);
	void	executeCgiDownload(std::string , RequestParser , std::string &);
	void	setBodyAndHeaders(const std::vector<char> &);
	const std::string	getCgiFullPath(const std::string &) const;

private:
	Response	*_res;
};

# include "Response.hpp"

#endif
