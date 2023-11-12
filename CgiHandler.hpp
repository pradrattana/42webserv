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
	CgiHandler(const CgiHandler &);
	CgiHandler(Response *);
	~CgiHandler();
	CgiHandler &operator=(const CgiHandler &);

	const std::string	getCgiFullPath(const std::string &) const;
	void	executeCgi(const std::string &);
	// void	exportEnv();

	void	setBodyAndHeaders(const std::string &);

private:
	Response	*_res;
};

# include "Response.hpp"

#endif