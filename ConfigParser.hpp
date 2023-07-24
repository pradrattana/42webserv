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
# include "webservStruct.hpp"

class ConfigParser {
public:
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

	void	fillDefaultServer(t_serverData &);
	void	fillDefaultLocation(t_serverData &, t_locationData &);
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
	static bool			compareByUri(const t_locationData &, const t_locationData &);

	class InvalidConfigException: public std::exception {
	public:
		const char	*what(void) const throw();
	};

};

#endif
