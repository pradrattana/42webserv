#include "ConfigParser.hpp"

ConfigParser::ConfigParser(void) {
	// std::cout << "Default constructor called by <ConfigParser>" << std::endl;
	initFuncMapping();
}

ConfigParser::ConfigParser(const ConfigParser &src) {
	// std::cout << "Copy constructor called by <ConfigParser>" << std::endl;
	*this = src;
}

ConfigParser::~ConfigParser(void) {
	// std::cout << "Destructor called by <ConfigParser>" << std::endl;
}

ConfigParser &ConfigParser::operator= (const ConfigParser &src) {
	// std::cout << "Copy assignment operator called by <ConfigParser>" << std::endl;
	_server = src.getServer();
	return *this;
}

void	ConfigParser::fillDefaultServer(t_serverData &s) {
	if (s.name.empty())
		s.name.push_back("");
	if (s.listen.empty()) {
		t_listenData	lsn = {"*", 80};
		s.listen.push_back(lsn);
		lsn.port = 8000;
		s.listen.push_back(lsn);
	}
	if (s.root.empty())
		s.root = "html";
	if (s.index.empty())
		s.index.push_back("index.html");
	if (s.cliMax.empty())
		s.cliMax = "1m";
	if (s.location.empty())
		s.location.push_back(t_locationData());
	for (std::vector<t_locationData>::iterator it = s.location.begin();
			it != s.location.end(); it++)
		fillDefaultLocation(s, *it);
}

void	ConfigParser::fillDefaultLocation(t_serverData &s, t_locationData &l) {
	if (l.uri.empty())
		l.uri = "/";
	if (l.listen.empty())
		l.listen = s.listen;
	if (l.root.empty())
		l.root = s.root;
	if (l.index.empty())
		l.index = s.index;
	if (l.cliMax.empty())
		l.cliMax = s.cliMax;
	if (l.errPage.empty())
		l.errPage = s.errPage;
}

void	ConfigParser::initFuncMapping() {
	_funcMaping["listen"] = &ConfigParser::parseListen;
	_funcMaping["server_name"] = &ConfigParser::parseServName;
	_funcMaping["root"] = &ConfigParser::parseRoot;
	_funcMaping["index"] = &ConfigParser::parseIndex;
	_funcMaping["autoindex"] = &ConfigParser::parseAutoIndex;
	_funcMaping["client_max_body_size"] = &ConfigParser::parseCliMaxBodySize;
	_funcMaping["error_page"] = &ConfigParser::parseErrPage;
	_funcMaping["limit_except"] = &ConfigParser::parseLimitExcept;
	_funcMaping["cgi_pass"] = &ConfigParser::parseCgiPass;
}

void	ConfigParser::initServPtrMapping(std::map<std::string, uintptr_t> &servMapping, const t_serverData &newServ) {
	servMapping["listen"] = serialize(&newServ.listen);
	servMapping["server_name"] = serialize(&newServ.name);
	servMapping["root"] = serialize(&newServ.root);
	servMapping["index"] = serialize(&newServ.index);
	servMapping["autoindex"] = serialize(&newServ.autoIdx);
	servMapping["client_max_body_size"] = serialize(&newServ.cliMax);
	servMapping["error_page"] = serialize(&newServ.errPage);
	servMapping["location"] = serialize(&newServ.location);
}

void	ConfigParser::initLocPtrMapping(std::map<std::string, uintptr_t> &locMapping, const t_locationData &newLoc) {
	locMapping["listen"] = serialize(&newLoc.listen);
	locMapping["root"] = serialize(&newLoc.root);
	locMapping["index"] = serialize(&newLoc.index);
	locMapping["autoindex"] = serialize(&newLoc.autoIdx);
	locMapping["client_max_body_size"] = serialize(&newLoc.cliMax);
	locMapping["error_page"] = serialize(&newLoc.errPage);
	locMapping["limit_except"] = serialize(&newLoc.limExcept);
	locMapping["cgi_pass"] = serialize(&newLoc.cgiPass);
}

void	ConfigParser::readConfig(const std::string &src) {
	std::ifstream		ifs(src.c_str());
	std::stringstream	ss;
	std::string			line, col;

	while (std::getline(ifs, line)) {
		trim(line);
		// std::cout << line << "\n";
		if (!(line.empty() || line[0] == '#')) {
			if (line[0] == '}')
				break;
			ss.clear();
			ss.str(line);
			ss >> col >> std::ws;

			if (col == "server") {
				_server.push_back(serverData());
				parseServer(ifs, _server.back());
				fillDefaultServer(_server.back());
				std::sort(_server.back().location.begin(),
					_server.back().location.end(), compareByUri);
			}
		}
	}
}

void	ConfigParser::parseServer(std::ifstream &ifs, t_serverData &newServ) {
	std::stringstream	ss;
	std::string			line, col;
	std::map<std::string, uintptr_t>	servMapping;

	initServPtrMapping(servMapping, newServ);

	while (std::getline(ifs, line)) {
		trim(line);
		if (!(line.empty() || line[0] == '#')) {
			if (line[0] == '}')
				break;
			ss.clear();
			ss.str(line);
			ss >> col >> std::ws;

			if (col == "location") {
				newServ.location.push_back(locationData());
				parseLocation(ifs, newServ.location.back(), ss.str().substr(ss.tellg()));
			} else if (servMapping.find(col) != servMapping.end()) {
				(this->*(_funcMaping[col]))(
					ss.str().substr(ss.tellg()), servMapping[col]
				);
			}
		}
	}
}

void	ConfigParser::parseLocation(std::ifstream &ifs, t_locationData &newLoc, const std::string &src) {
	std::stringstream	ss;
	std::string			line, col;
	std::map<std::string, uintptr_t>	locMapping;

	newLoc.uri = parseLocationUri(src);
	initLocPtrMapping(locMapping, newLoc);

	while (std::getline(ifs, line)) {
		trim(line);
		if (!(line.empty() || line[0] == '#')) {
			if (line[0] == '}')
				break;
			ss.clear();
			ss.str(line);
			ss >> col >> std::ws;

			if (locMapping.find(col) != locMapping.end()) {
				(this->*(_funcMaping[col]))(
					ss.str().substr(ss.tellg()), locMapping[col]
				);
			}
		}
	}
}

void	ConfigParser::parseListen(const std::string &s, uintptr_t p) {
	const std::string	res = parseHelper(s);
	std::istringstream	iss(res);
	t_listenData	lsn;
	std::string			col;
	std::string::size_type	pos;

	if (!res.empty()) {
		if ((pos = res.rfind(':')) != std::string::npos) {
			lsn.addr = res.substr(0, pos);
			lsn.port = atoi(res.substr(pos + 1).c_str());
		} else {
			lsn.port = atoi(res.c_str());
		}
		iss >> col;
		if (iss.eof()) {
			deserialize<std::vector<t_listenData> >(p)->push_back(lsn);
			return ;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseServName(const std::string &s, uintptr_t p) {
	const std::string	res = parseHelper(s);
	std::istringstream	iss(res);
	std::vector<std::string>	name;
	std::string					col;

	if (!res.empty()) {
		while (iss >> col) {
			name.push_back(col);
		}
		*deserialize<std::vector<std::string> >(p) = name;
		return ;
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseRoot(const std::string &s, uintptr_t p) {
	const std::string	res = parseHelper(s);
	std::istringstream	iss(res);
	std::string			col;

	if (!res.empty()) {
		iss >> col;
		if (iss.eof()) {
			if (col[col.length() - 1] == '/')
				col.erase(col.length() - 1);
			*deserialize<std::string>(p) = col;
			return ;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseIndex(const std::string &s, uintptr_t p) {
	const std::string	res = parseHelper(s);
	std::istringstream	iss(res);
	std::vector<std::string>	id;
	std::string					col;

	if (!res.empty()) {
		while (iss >> col) {
			id.push_back(col);
		}
		*deserialize<std::vector<std::string> >(p) = id;
		return ;
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseAutoIndex(const std::string &s, uintptr_t p) {
	const std::string	res = parseHelper(s);

	if (!res.empty()) {
		if (res == "on" || res == "off") {
			*deserialize<int>(p) = (res == "on" ? 1 : 0);
			return ;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseCliMaxBodySize(const std::string &s, uintptr_t p) {
	const std::string	res = parseHelper(s);
	std::istringstream	iss(res);
	std::string			col;

	if (!res.empty()) {
		iss >> col;
		if (iss.eof()) {
			*deserialize<std::string>(p) = col;
			return ;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseErrPage(const std::string &s, uintptr_t p) {
	const std::string	res = parseHelper(s);
	std::istringstream	iss(res);
	t_errorPageData		err;
	int					col;

	if (!res.empty()) {
		while (iss >> col)
			err.code.push_back(col);
		iss.clear();
		iss >> err.uri;
		if (iss.eof()) {
			deserialize<std::vector<t_errorPageData> >(p)->push_back(err);
			return ;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseLimitExcept(const std::string &s, uintptr_t p) {
	const std::string	res = parseHelper(s);
	std::istringstream	iss(res);
	std::vector<std::string>	lim;
	std::string					col;

	if (!res.empty()) {
		while (iss >> col) {
			lim.push_back(col);
		}
		*deserialize<std::vector<std::string> >(p) = lim;
		return ;
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseCgiPass(const std::string &s, uintptr_t p) {
	std::istringstream	iss(parseHelper(s));
	std::string			col;

	if (!iss.eof()) {
		iss >> col;
		if (iss.eof()) {
			*deserialize<std::string>(p) = col;
			return ;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

const std::string	ConfigParser::parseLocationUri(const std::string &src) {
	std::string	result, afterRes, col;
	std::string::size_type	pos = src.find('{');

	if (pos != std::string::npos) {
		result = src.substr(0, pos);
		afterRes = src.substr(pos + 1);

		if (trim(afterRes).empty()) {
			if (!rtrim(result).empty()) {
				std::istringstream	iss(result);
				iss >> col;
				if (iss.eof()) {
					if (!rtrim(result).empty())
						return result;
				}
			}
		}
	}
	throw ConfigParser::InvalidConfigException();
}

const std::string	ConfigParser::parseHelper(const std::string &src) {
	std::string	result, afterRes;
	std::string::size_type	pos = src.find(';');

	if (pos != std::string::npos) {
		result = src.substr(0, pos);
		afterRes = src.substr(pos);

		if (trim(afterRes, " \t\n\r\f\v;").empty()) {
			if (!rtrim(result).empty())
				return result;
		}
	}
	return "";
	// throw ConfigParser::InvalidConfigException();
}

bool	ConfigParser::compareByUri(const t_locationData &a, const t_locationData &b) {
	return	a.uri > b.uri;
}

void	ConfigParser::printAll() {
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

const std::vector<t_serverData>	&ConfigParser::getServer() const {
	return _server;
}

const char	*ConfigParser::InvalidConfigException::what(void) const throw() {
	return ("Invalid configuration file syntax.");
}
