#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <iostream>
# include <sstream>
# include <fstream>
# include <string>
# include <cstring>
# include <cstdlib>
# include <vector>
# include <map>
# include <algorithm>
# include "helper.hpp"

class ConfigParser {
private:
	struct listenData {
		std::string	addr;
		int			port;
	};

	struct errorPageData {
		std::vector<int>	code;
		std::string			uri;
	};

	struct locationData {
		std::string					uri;
		std::vector<struct listenData>		listen;
		std::string					root;
		std::vector<std::string>	index;
		int							autoIdx;
		std::string 				cliMax;
		std::vector<struct errorPageData>	errPage;
		std::vector<std::string>			limExcept;
	};

	typedef struct serverData {
		std::vector<struct listenData>		listen;
		std::vector<std::string>	name;
		std::string					root;
		std::vector<std::string>	index;
		int							autoIdx;
		std::string					cliMax;
		std::vector<struct errorPageData>	errPage;
		std::vector<struct locationData>	location;
	}	t_serverData;

public:
	ConfigParser();
	ConfigParser(const ConfigParser &);
	ConfigParser(const std::string &);
	~ConfigParser();
	ConfigParser &operator= (const ConfigParser &);

	void	printAll();

private:
	std::vector<struct serverData>	_server;
	std::map<
		std::string, void (ConfigParser::*)(const std::string &, uintptr_t)
	>								_funcMaping;

	void	initFuncMapping();
	void	initServPtrMapping(std::map<std::string, uintptr_t> &, const struct serverData &);
	void	initLocPtrMapping(std::map<std::string, uintptr_t> &, const struct locationData &);

	void	readConfig(const std::string &);
	void	readServer(std::ifstream &, struct serverData &);
	void	readLocation(std::ifstream &, struct locationData &, const std::string &);

	void	readListen(const std::string &, uintptr_t);
	void	readServName(const std::string &, uintptr_t);
	void	readRoot(const std::string &, uintptr_t);
	void	readIndex(const std::string &, uintptr_t);
	void	readAutoIndex(const std::string &, uintptr_t);
	void	readCliMaxBodySize(const std::string &, uintptr_t);
	void	readErrPage(const std::string &, uintptr_t);
	void	readLimitExcept(const std::string &, uintptr_t);

	const std::string	readLocationUri(const std::string &);
	const std::string	readHelper(const std::string &);

	class InvalidConfigException: public std::exception {
	public:
		const char	*what(void) const throw();
	};

};

#endif
