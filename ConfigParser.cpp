#include "ConfigParser.hpp"

ConfigParser::ConfigParser(void)
{
	initFuncMapping();
}

ConfigParser::ConfigParser(const char *src)
{
	initFuncMapping();
	readConfig(src);
}

ConfigParser::ConfigParser(const ConfigParser &src)
{
	*this = src;
}

ConfigParser::~ConfigParser(void)
{
}

ConfigParser &ConfigParser::operator=(const ConfigParser &src)
{
	if (this != &src)
	{
		_server = src._server;
		_func = src._func;
	}
	return *this;
}

void	ConfigParser::readConfig(const char *src)
{
	std::ifstream	ifs(src);
	std::string		line, col;

	std::getline(ifs, line);
	if (ifs.fail())
	{
		line = "Could not read the file: ";
		if (ifs.eof())
			line = "Reached the end of the file prematurely: ";
		throw std::runtime_error(line.append(src));
	}

	do
	{
		// if (line.find('#') != std::string::npos)
		// 	line.erase(line.find('#'));
		// trim(line);
		if (!trim(eraseIfFound(line, "#")).empty())
		{
			std::stringstream	ss(line);
			ss >> col >> std::ws;
			if (col == "server")
			{
				if (ss.eof())
				{
					while (std::getline(ifs, line))
					{
						if (!trim(eraseIfFound(line, "#")).empty())
						{
							if (line != "{")
								throw ConfigParser::InvalidConfigException();
							break;
						}
					}
				}
				else if (ss.str().substr(ss.tellg()) != "{")
					throw ConfigParser::InvalidConfigException();

				_server.push_back(serverData());
				parseServer(ifs, _server.back());
				fillDefaultServer(_server.back());
			}
			else
				throw ConfigParser::InvalidConfigException();
		}
	} while (std::getline(ifs, line));
	if (!ifs.eof())
		throw ConfigParser::InvalidConfigException();
	ifs.close();
}

void	ConfigParser::initFuncMapping()
{
	_func["listen"] = &ConfigParser::parseListen;
	_func["server_name"] = &ConfigParser::parseServName;
	_func["root"] = &ConfigParser::parseRoot;
	_func["index"] = &ConfigParser::parseIndex;
	_func["autoindex"] = &ConfigParser::parseAutoIndex;
	_func["client_max_body_size"] = &ConfigParser::parseCliMaxBodySize;
	_func["error_page"] = &ConfigParser::parseErrPage;
	_func["return"] = &ConfigParser::parseReturn;
	_func["limit_except"] = &ConfigParser::parseLimitExcept;
	_func["cgi_pass"] = &ConfigParser::parseCgiPass;
}

void	ConfigParser::initServMapping(std::map<std::string, uintptr_t> &servMap, const t_serverData &serv)
{
	servMap["listen"] = serialize(&serv.listen);
	servMap["server_name"] = serialize(&serv.name);
	servMap["root"] = serialize(&serv.root);
	servMap["index"] = serialize(&serv.index);
	servMap["autoindex"] = serialize(&serv.autoIdx);
	servMap["client_max_body_size"] = serialize(&serv.cliMax);
	servMap["error_page"] = serialize(&serv.errPage);
	servMap["return"] = serialize(&serv.redir);
	servMap["location"] = serialize(&serv.location);
}

void	ConfigParser::initLocMapping(std::map<std::string, uintptr_t> &locMap, const t_locationData &loc)
{
	locMap["root"] = serialize(&loc.root);
	locMap["index"] = serialize(&loc.index);
	locMap["autoindex"] = serialize(&loc.autoIdx);
	locMap["client_max_body_size"] = serialize(&loc.cliMax);
	locMap["error_page"] = serialize(&loc.errPage);
	locMap["return"] = serialize(&loc.redir);
	locMap["limit_except"] = serialize(&loc.limExcept);
	locMap["cgi_pass"] = serialize(&loc.cgiPass);
}

void	ConfigParser::fillDefaultServer(t_serverData &s)
{
	bool	defLoc = false;

	if (s.name.empty())
		s.name.push_back("");
	if (s.root.empty())
		s.root = "html";
	if (s.index.empty())
		s.index.push_back("index.html");
	if (s.autoIdx.empty())
		s.autoIdx = "off";
	if (s.cliMax == 0)
		s.cliMax = 1000000;
	for (std::vector<t_locationData>::iterator it = s.location.begin();
			it != s.location.end(); it++)
	{
		if (it->uri == "/")
			defLoc = true;
		fillDefaultLocation(*it, s);
	}
	if (!defLoc)
	{
		s.location.push_back(t_locationData());
		fillDefaultLocation(s.location.back(), s);
	}
	std::sort(s.location.begin(), s.location.end(), compareByUri);
	if (s.location.back().limExcept.empty())
		s.location.back().limExcept.insert("GET");
	for (std::vector<t_locationData>::iterator it = s.location.begin();
			it != s.location.end() - 1; it++)
	{
		if (it->limExcept.empty())
			it->limExcept = s.location.back().limExcept;
	}
}

void	ConfigParser::fillDefaultLocation(t_locationData &l, const t_serverData &s)
{
	if (l.uri.empty())
		l.uri = "/";
	if (l.listen.empty())
		l.listen = s.listen;
	if (l.root.empty())
	{
		l.isRootOvr = false;
		l.root = s.root;
	}
	if (l.index.empty())
		l.index = s.index;
	if (l.autoIdx.empty())
		l.autoIdx = s.autoIdx;
	if (l.cliMax == 0)
		l.cliMax = s.cliMax;
	if (l.errPage.empty())
		l.errPage = s.errPage;
	if (l.redir.url.empty())
		l.redir = s.redir;
	// if (l.limExcept.empty())
	// 	l.limExcept.insert("GET");
	if (l.cgiPass.empty())
		l.cgiPass = "php-cgi";
}

void	ConfigParser::parseServer(std::ifstream &ifs, t_serverData &serv)
{
	std::map<std::string, uintptr_t>	servMap;
	std::string	line, col;

	initServMapping(servMap, serv);
	_isListen = false;
	while (std::getline(ifs, line))
	{
		if (!trim(eraseIfFound(line, "#")).empty())
		{
			std::stringstream	ss(line);
			ss >> col >> std::ws;
			if (col == "location")
			{
				if (ss >> col >> std::ws)
				{
					if (isPathValid(col))
					{
						if (ss.eof())
						{
							while (std::getline(ifs, line))
							{
								if (!trim(eraseIfFound(line, "#")).empty())
								{
									if (line != "{")
										throw ConfigParser::InvalidConfigException();
									break;
								}
							}
						}
						else if (ss.str().substr(ss.tellg()) != "{")
							throw ConfigParser::InvalidConfigException();
						serv.location.push_back(locationData());
						serv.location.back().uri = formatPath(col);
						parseLocation(ifs, serv.location.back());
						continue;
					}
				}
			}
			else if (servMap.find(col) != servMap.end())
			{
				(this->*(_func[col]))(
					line.substr(ss.tellg()), servMap[col]
				);
				continue;
			}
			else if (col == "}" && ss.eof())
				break;
			throw ConfigParser::InvalidConfigException();
		}
	}
	if (!_isListen)
		throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseLocation(std::ifstream &ifs, t_locationData &loc)
{
	std::map<std::string, uintptr_t>	locMap;
	std::string	line, col;

	initLocMapping(locMap, loc);
	while (std::getline(ifs, line))
	{
		// if (!(line.empty() || line[0] == '#'))
		if (!trim(eraseIfFound(line, "#")).empty())
		{
			std::stringstream	ss(line);
			ss >> col >> std::ws;
			if (locMap.find(col) != locMap.end())
				(this->*(_func[col]))(
					line.substr(ss.tellg()), locMap[col]
				);
			else if (col == "}" && ss.eof())
				return;
			else
				throw ConfigParser::InvalidConfigException();
		}
	}
}

void	ConfigParser::parseListen(const std::string &s, uintptr_t p)
{
	std::istringstream		iss(parseHelper(s));
	t_listenData			lsn;
	std::string				col;
	std::string::size_type	pos;

	iss >> col;
	if (iss.eof())
	{
		if ((pos = col.rfind(':')) != std::string::npos)
		{
			if (!isIPv4(lsn.addr = col.substr(0, pos)))
				throw ConfigParser::InvalidConfigException();
			iss.seekg(pos + 1);
		}
		else
		{
			lsn.addr = "127.0.0.1";
			iss.seekg(0);
		}
		if ((iss >> lsn.port >> std::ws).eof())
		{
			if (_port.find(lsn.port) == _port.end())
			{
				_isListen = true;
				_port.insert(lsn.port);
				deserialize<std::vector<t_listenData> >(p)->push_back(lsn);
				return ;
			}
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseServName(const std::string &s, uintptr_t p)
{
	std::istringstream			iss(parseHelper(s));
	std::vector<std::string>	name;
	std::string					col;

	while (iss >> col)
		name.push_back(col);
	*deserialize<std::vector<std::string> >(p) = name;
}

void	ConfigParser::parseRoot(const std::string &s, uintptr_t p)
{
	std::istringstream	iss(parseHelper(s));
	std::string			col;

	if ((iss >> col).eof())
	{
		if (isPathValid(col))
		{
			if (*(col.end() - 1) == '/')
				col.erase(col.end() - 1);
			*deserialize<std::string>(p) = col;
			return ;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseIndex(const std::string &s, uintptr_t p)
{
	std::istringstream			iss(parseHelper(s));
	std::vector<std::string>	id;
	std::string					col;

	while (iss >> col)
		id.push_back(col);
	*deserialize<std::vector<std::string> >(p) = id;
}

void	ConfigParser::parseAutoIndex(const std::string &s, uintptr_t p)
{
	const std::string	res = parseHelper(s);

	if (res == "on" || res == "off")
	{
		*deserialize<std::string>(p) = res;
		return ;
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseCliMaxBodySize(const std::string &s, uintptr_t p)
{
	std::istringstream	iss(parseHelper(s));
	std::string	col;
	int			size;
	char		multiplicand = '0';

	if ((iss >> col).eof())
	{
		iss.seekg(0);
		if (iss >> size)
		{
			if ((iss >> multiplicand >> std::ws).eof())
			{
				std::map<char, unsigned long>	bytesMap;
				bytesMap['k'] = 1000;
				bytesMap['m'] = 1000000;
				bytesMap['g'] = 1000000000;
				try
				{
					*deserialize<unsigned long>(p) =
						size * ((multiplicand = tolower(multiplicand)) != '0' ?
							bytesMap.at(multiplicand) : 1);
					return;
				}
				catch (...)
				{
				}
			}
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseErrPage(const std::string &s, uintptr_t p)
{
	std::istringstream	iss(parseHelper(s));
	t_errorPageData		err;
	int					col;

	while (iss >> col)
		err.code.push_back(col);
	iss.clear();
	if ((iss >> err.uri).eof())
	{
		if (isPathValid(err.uri))
		{
			formatPath(err.uri);
			// if is file
			deserialize<std::vector<t_errorPageData> >(p)->push_back(err);
			return;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseReturn(const std::string &s, uintptr_t p)
{
	std::istringstream	iss(parseHelper(s));
	t_returnData	ret;

	if (iss >> ret.code >> std::ws)
	{
		// std::cout << iss.str().substr(iss.tellg()) << '\n';
		ret.url = iss.str().substr(iss.tellg());
		if (ret.url.find("://") != std::string::npos)
		{
			if (isURIValid(ret.url))
			{
				*deserialize<t_returnData>(p) = ret;
				return;
			}
		}
		else
		{
			std::string::size_type	pos = formatPath(ret.url).find('?');
			if (pos != std::string::npos)
			{
				if (!isURIQueryValid(ret.url.substr(pos + 1)))
					throw ConfigParser::InvalidConfigException();
			}
			if (isURIPathValid(ret.url.substr(0, pos)))
			{
				*deserialize<t_returnData>(p) = ret;
				return;
			}
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseLimitExcept(const std::string &s, uintptr_t p)
{
	std::istringstream		iss(parseHelper(s));
	std::set<std::string>	allow;
	std::set<std::string>	lim;
	std::string				col;

	allow.insert("GET");
	allow.insert("POST");
	allow.insert("DELETE");
	while (iss >> col)
	{
		if (std::find(allow.begin(), allow.end(), col) != allow.end())
			lim.insert(col);
		else
			throw ConfigParser::InvalidConfigException();
	}
	*deserialize<std::set<std::string> >(p) = lim;
}

void	ConfigParser::parseCgiPass(const std::string &s, uintptr_t p)
{
	const std::string		res = parseHelper(s);
	std::set<std::string>	allow;

	allow.insert("php-cgi");
	allow.insert("cgi-bin/download-file.py");
	allow.insert("cgi-bin/delete-file.perl");
	if (std::find(allow.begin(), allow.end(), res) != allow.end())
	{
		*deserialize<std::string>(p) = res;
		return ;
	}
	throw ConfigParser::InvalidConfigException();
}

const std::string	ConfigParser::parseHelper(const std::string &src)
{
	std::string	result, afterRes;
	std::string::size_type	pos;

	if ((pos = src.find(';')) != std::string::npos)
	{
		result = src.substr(0, pos);
		afterRes = src.substr(pos);

		if (trim(afterRes, " \t\n\r\f\v;").empty())
		{
			if (!rtrim(result).empty())
				return result;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::printAll()
{
	for (std::vector<t_serverData>::iterator it = _server.begin();
			it != _server.end(); it++)
	{
		std::cout << "SERVER" << std::endl;

		for (std::vector<t_listenData>::iterator it2 = it->listen.begin();
			it2 != it->listen.end(); it2++)
			std::cout << "listen: " << it2->addr << ":" << it2->port << std::endl;

		std::cout << "name: ";
		for (std::vector<std::string>::iterator it2 = it->name.begin();
			it2 != it->name.end(); it2++)
			std::cout << *it2 << " ";
		std::cout << std::endl;
			
		std::cout << "root: " << it->root << std::endl;

		std::cout << "index: ";
		for (std::vector<std::string>::iterator it2 = it->index.begin();
			it2 != it->index.end(); it2++)
			std::cout << *it2 << " ";
		std::cout << std::endl;

		std::cout << "autoindex: " << it->autoIdx << std::endl;
		std::cout << "climax: " << it->cliMax << std::endl;

		for (std::vector<t_errorPageData>::iterator it2 = it->errPage.begin();
			it2 != it->errPage.end(); it2++)
		{
			std::cout << "err-uri: " << it2->uri << std::endl;
			std::cout << "err-code: ";
			for (std::vector<int>::iterator it3 = it2->code.begin();
					it3 != it2->code.end(); it3++)
				std::cout << *it3 << " ";
			std::cout << std::endl;
		}

		for (std::vector<t_locationData>::iterator it2 = it->location.begin();
			it2 != it->location.end(); it2++)
			std::cout << "location: " << it2->uri << std::endl;

		std::cout << std::endl;
	}
}

const std::vector<t_serverData>	&ConfigParser::getServer() const
{
	return _server;
}

const char	*ConfigParser::InvalidConfigException::what(void) const throw()
{
	return ("Invalid configuration file syntax.");
}

bool	compareByUri(const t_locationData &a, const t_locationData &b)
{
	return	a.uri > b.uri;
}
