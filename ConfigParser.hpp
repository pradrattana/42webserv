#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <iostream>
# include <sstream>
# include <fstream>
# include <string>
# include <cstring>
# include <cstdlib>
# include <vector>
# include <stack>
# include <map>
# include <algorithm>
# include "helper.hpp"
# include "webservStruct.hpp"

class ConfigParser {
public:
	ConfigParser();
	ConfigParser(const char *);
	ConfigParser(const ConfigParser &);
	~ConfigParser();
	ConfigParser &operator= (const ConfigParser &);

	void	readConfig(const char *);

	const std::vector<t_serverData>	&getServer() const;
	void	printAll();

private:
	std::vector<t_serverData>	_server;
	std::map<
		std::string, void (ConfigParser::*)(const std::string &, uintptr_t)
	>							_func;
	std::set<int>	_port;
	bool	_isListen;

	void	initFuncMapping();
	void	initServMapping(std::map<std::string, uintptr_t> &, const t_serverData &);
	void	initLocMapping(std::map<std::string, uintptr_t> &, const t_locationData &);

	void	fillDefaultServer(t_serverData &);
	void	fillDefaultLocation(t_locationData &, const t_serverData &);

	void	parseServer(std::ifstream &, t_serverData &);
	void	parseLocation(std::ifstream &, t_locationData &);

	void	parseListen(const std::string &, uintptr_t);
	void	parseServName(const std::string &, uintptr_t);
	void	parseRoot(const std::string &, uintptr_t);
	void	parseIndex(const std::string &, uintptr_t);
	void	parseAutoIndex(const std::string &, uintptr_t);
	void	parseCliMaxBodySize(const std::string &, uintptr_t);
	void	parseErrPage(const std::string &, uintptr_t);
	void	parseReturn(const std::string &, uintptr_t);
	void	parseLimitExcept(const std::string &, uintptr_t);
	void	parseCgiPass(const std::string &, uintptr_t);

	const std::string	parseHelper(const std::string &);

	class InvalidConfigException: public std::exception {
	public:
		const char	*what(void) const throw();
	};
};

bool	compareByUri(const t_locationData &, const t_locationData &);

#endif
