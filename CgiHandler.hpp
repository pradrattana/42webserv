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

class Response;

class CgiHandler {
public:
	CgiHandler();
	CgiHandler(Response *);

	const std::string	getCgiFullPath(const std::string &) const;
	void	executeCgi();
	// void	exportEnv();

	void	setBodyAndHeaders(const std::string);

private:
	Response	*_res;
	std::string	_env;
};

# include "Response.hpp"

#endif