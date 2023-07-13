#include "ConfigParser.hpp"

ConfigParser::ConfigParser(void) {
	// std::cout << "Default constructor called by <ConfigParser>" << std::endl;
	initFuncMapping();
}

ConfigParser::ConfigParser(const ConfigParser &src) {
	// std::cout << "Copy constructor called by <ConfigParser>" << std::endl;
	*this = src;
}

ConfigParser::ConfigParser(const std::string &src) {
	initFuncMapping();
	readConfig(src);
}

ConfigParser::~ConfigParser(void) {
	// std::cout << "Destructor called by <ConfigParser>" << std::endl;
}

ConfigParser &ConfigParser::operator= (const ConfigParser &src) {
	// std::cout << "Copy assignment operator called by <ConfigParser>" << std::endl;
	(void)src;
	return *this;
}

void	ConfigParser::initFuncMapping() {
	_funcMaping["listen"] = &ConfigParser::readListen;
	_funcMaping["server_name"] = &ConfigParser::readServName;
	_funcMaping["root"] = &ConfigParser::readRoot;
	_funcMaping["index"] = &ConfigParser::readIndex;
	_funcMaping["autoindex"] = &ConfigParser::readAutoIndex;
	_funcMaping["client_max_body_size"] = &ConfigParser::readCliMaxBodySize;
	_funcMaping["error_page"] = &ConfigParser::readErrPage;
	_funcMaping["limit_except"] = &ConfigParser::readLimitExcept;
}

void	ConfigParser::initServPtrMapping(std::map<std::string, uintptr_t> &servMapping, const struct serverData &newServ) {
	servMapping["listen"] = serialize(&newServ.listen);
	servMapping["server_name"] = serialize(&newServ.name);
	servMapping["root"] = serialize(&newServ.root);
	servMapping["index"] = serialize(&newServ.index);
	servMapping["autoindex"] = serialize(&newServ.autoIdx);
	servMapping["client_max_body_size"] = serialize(&newServ.cliMax);
	servMapping["error_page"] = serialize(&newServ.errPage);
	servMapping["location"] = serialize(&newServ.location);
}

void	ConfigParser::initLocPtrMapping(std::map<std::string, uintptr_t> &locMapping, const struct locationData &newLoc) {
	locMapping["listen"] = serialize(&newLoc.listen);
	locMapping["root"] = serialize(&newLoc.root);
	locMapping["index"] = serialize(&newLoc.index);
	locMapping["autoindex"] = serialize(&newLoc.autoIdx);
	locMapping["client_max_body_size"] = serialize(&newLoc.cliMax);
	locMapping["error_page"] = serialize(&newLoc.errPage);
	locMapping["limit_except"] = serialize(&newLoc.limExcept);
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
				readServer(ifs, _server.back());
			}
		}
	}
}

void	ConfigParser::readServer(std::ifstream &ifs, struct serverData &newServ) {
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
				readLocation(ifs, newServ.location.back(), ss.str().substr(ss.tellg()));
			} else if (servMapping.find(col) != servMapping.end()) {
				(this->*(_funcMaping[col]))(
					ss.str().substr(ss.tellg()), servMapping[col]
				);
			}
		}
	}
}

void	ConfigParser::readLocation(std::ifstream &ifs, struct locationData &newLoc, const std::string &src) {
	std::stringstream	ss;
	std::string			line, col;
	std::map<std::string, uintptr_t>	locMapping;

	newLoc.uri = readLocationUri(src);
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

void	ConfigParser::readListen(const std::string &s, uintptr_t p) {
	const std::string	res = readHelper(s);
	std::istringstream	iss(res);
	struct listenData	lsn;
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
			deserialize<std::vector<struct listenData> >(p)->push_back(lsn);
			return ;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::readServName(const std::string &s, uintptr_t p) {
	const std::string	res = readHelper(s);
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

void	ConfigParser::readRoot(const std::string &s, uintptr_t p) {
	const std::string	res = readHelper(s);
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

void	ConfigParser::readIndex(const std::string &s, uintptr_t p) {
	const std::string	res = readHelper(s);
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

void	ConfigParser::readAutoIndex(const std::string &s, uintptr_t p) {
	const std::string	res = readHelper(s);

	if (!res.empty()) {
		if (res == "on" || res == "off") {
			*deserialize<int>(p) = (res == "on" ? 1 : 0);
			return ;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::readCliMaxBodySize(const std::string &s, uintptr_t p) {
	const std::string	res = readHelper(s);
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

void	ConfigParser::readErrPage(const std::string &s, uintptr_t p) {
	const std::string	res = readHelper(s);
	std::istringstream	iss(res);
	struct errorPageData	err;
	int						col;

	if (!res.empty()) {
		while (iss >> col)
			err.code.push_back(col);
		iss.clear();
		iss >> err.uri;
		if (iss.eof()) {
			deserialize<std::vector<struct errorPageData> >(p)->push_back(err);
			return ;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::readLimitExcept(const std::string &s, uintptr_t p) {
	const std::string	res = readHelper(s);
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

const std::string	ConfigParser::readLocationUri(const std::string &src) {
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
					// return rtrim(result);
				}
			}
		}
	}
	throw ConfigParser::InvalidConfigException();
	// return "";
}

const std::string	ConfigParser::readHelper(const std::string &src) {
	std::string	result, afterRes;

	std::string::size_type	pos = src.find(';');
	if (pos != std::string::npos) {
		result = src.substr(0, pos);
		afterRes = src.substr(pos);

		if (trim(afterRes, " \t\n\r\f\v;").empty()) {
			// if (!rtrim(result).empty()) {
			// 	return result;
			// }
			return rtrim(result);
		}
	}
	return "";
}

void	ConfigParser::printAll() {
	for (std::vector<struct serverData>::iterator it = _server.begin();
			it != _server.end(); it++)
	{
		std::cout << "SERVER" << std::endl;

		for (std::vector<struct listenData>::iterator it2 = it->listen.begin();
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

		for (std::vector<struct errorPageData>::iterator it2 = it->errPage.begin();
			it2 != it->errPage.end(); it2++)
		{
			std::cout << "err-uri: " << it2->uri << std::endl;
			std::cout << "err-code: ";
			for (std::vector<int>::iterator it3 = it2->code.begin();
					it3 != it2->code.end(); it3++)
				std::cout << *it3 << " ";
			std::cout << std::endl;
		}

		for (std::vector<struct locationData>::iterator it2 = it->location.begin();
			it2 != it->location.end(); it2++)
			std::cout << "location: " << it2->uri << std::endl;

		std::cout << std::endl;
	}
}

const char	*ConfigParser::InvalidConfigException::what(void) const throw() {
	return ("Invalid configuration file syntax.");
}
