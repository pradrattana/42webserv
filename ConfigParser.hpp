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
public:
	typedef struct listenData {
		std::string	addr;
		int			port;
	}	t_listenData;

	typedef struct errorPageData {
		std::vector<int>	code;
		std::string			uri;
	}	t_errorPageData;

	typedef struct locationData {
		std::string					uri;
		std::vector<t_listenData>	listen;
		std::string					root;
		std::vector<std::string>	index;
		int							autoIdx;
		std::string 				cliMax;
		std::vector<t_errorPageData>	errPage;
		std::vector<std::string>	limExcept;
		std::string					cgiPass;
	}	t_locationData;

	typedef struct serverData {
		std::vector<std::string>	name;
		std::vector<t_listenData>	listen;
		std::string					root;
		std::vector<std::string>	index;
		int							autoIdx;
		std::string					cliMax;
		std::vector<t_errorPageData>	errPage;
		std::vector<t_locationData>	location;
	}	t_serverData;

	ConfigParser();
	ConfigParser(const ConfigParser &);
	~ConfigParser();
	ConfigParser &operator= (const ConfigParser &);

	void	readConfig(const std::string &);
	void	printAll();
	const std::vector<t_serverData>	&getServer() const;

private:
	std::vector<t_serverData>	_server;
	std::map<
		std::string, void (ConfigParser::*)(const std::string &, uintptr_t)
	>								_funcMaping;

	void	initFuncMapping();
	void	initServPtrMapping(std::map<std::string, uintptr_t> &, const t_serverData &);
	void	initLocPtrMapping(std::map<std::string, uintptr_t> &, const t_locationData &);
	void	fillConfig();

	void	parseServer(std::ifstream &, t_serverData &);
	void	parseLocation(std::ifstream &, t_locationData &, const std::string &);

	void	parseListen(const std::string &, uintptr_t);
	void	parseServName(const std::string &, uintptr_t);
	void	parseRoot(const std::string &, uintptr_t);
	void	parseIndex(const std::string &, uintptr_t);
	void	parseAutoIndex(const std::string &, uintptr_t);
	void	parseCliMaxBodySize(const std::string &, uintptr_t);
	void	parseErrPage(const std::string &, uintptr_t);
	void	parseLimitExcept(const std::string &, uintptr_t);
	void	parseCgiPass(const std::string &, uintptr_t);

	const std::string	parseLocationUri(const std::string &);
	const std::string	parseHelper(const std::string &);

	class InvalidConfigException: public std::exception {
	public:
		const char	*what(void) const throw();
	};

};

#endif
