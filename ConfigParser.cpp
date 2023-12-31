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
	bool	defLoc = false;

	if (s.name.empty())
		s.name.push_back("");
	if (s.listen.empty()) {
		t_listenData	lsn = { "*", 80 };
		s.listen.push_back(lsn);
		lsn.port = 8000;
		s.listen.push_back(lsn);
	}
	if (s.root.empty())
		s.root = "html";
	if (s.index.empty())
		s.index.push_back("index.html");
	if (s.autoIdx.empty())
		s.autoIdx = "off";
	if (s.cliMax.empty())
		s.cliMax = "1m";
	for (std::vector<t_locationData>::iterator it = s.location.begin();
			it != s.location.end(); it++)
	{
		if (it->uri == "/")
			defLoc = true;
		fillDefaultLocation(s, *it);
	}
	if (!defLoc) {
		s.location.push_back(t_locationData());
		fillDefaultLocation(s, s.location.back());
	}
	std::sort(s.location.begin(), s.location.end(), compareByUri);
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
	if (l.autoIdx.empty())
		l.autoIdx = s.autoIdx;
	if (l.cliMax.empty())
		l.cliMax = s.cliMax;
	if (l.errPage.empty())
		l.errPage = s.errPage;
	l.limExcept.insert("GET");
	l.limExcept.insert("HEAD");
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

void	ConfigParser::initServPtrMapping(std::map<std::string, uintptr_t> &servMapping, const t_serverData &serv) {
	servMapping["listen"] = serialize(&serv.listen);
	servMapping["server_name"] = serialize(&serv.name);
	servMapping["root"] = serialize(&serv.root);
	servMapping["index"] = serialize(&serv.index);
	servMapping["autoindex"] = serialize(&serv.autoIdx);
	servMapping["client_max_body_size"] = serialize(&serv.cliMax);
	servMapping["error_page"] = serialize(&serv.errPage);
	servMapping["location"] = serialize(&serv.location);
}

void	ConfigParser::initLocPtrMapping(std::map<std::string, uintptr_t> &locMapping, const t_locationData &loc) {
	locMapping["listen"] = serialize(&loc.listen);
	locMapping["root"] = serialize(&loc.root);
	locMapping["index"] = serialize(&loc.index);
	locMapping["autoindex"] = serialize(&loc.autoIdx);
	locMapping["client_max_body_size"] = serialize(&loc.cliMax);
	locMapping["error_page"] = serialize(&loc.errPage);
	locMapping["limit_except"] = serialize(&loc.limExcept);
	locMapping["cgi_pass"] = serialize(&loc.cgiPass);
}

void	ConfigParser::readConfig(const std::string &src) {
	std::ifstream		ifs(src.c_str());
	std::stringstream	ss;
	std::string			line, col;

	while (std::getline(ifs, line)) {
		trim(line);
		if (!(line.empty() || line[0] == '#')) {
			if (line == "}")
				break;

			ss.clear();
			ss.str(line);
			ss >> col >> std::ws;

			if (col == "server") {
				if (ss >> col) {
					if (!(ss.eof() && col == "{"))
						throw ConfigParser::InvalidConfigException();
				} else if (std::getline(ifs, line)) {
					if (trim(line) != "{")
						throw ConfigParser::InvalidConfigException();
				} else {
					throw ConfigParser::InvalidConfigException();
				}

				_server.push_back(serverData());
				parseServer(ifs, _server.back());

				fillDefaultServer(_server.back());
			}
		}
	}
}

void	ConfigParser::parseServer(std::ifstream &ifs, t_serverData &serv) {
	std::stringstream	ss;
	std::string			line, col, uri;
	std::map<std::string, uintptr_t>	servMapping;

	initServPtrMapping(servMapping, serv);

	while (std::getline(ifs, line)) {
		trim(line);
		if (!(line.empty() || line[0] == '#')) {
			if (line == "}")
				break;

			ss.clear();
			ss.str(line);
			ss >> col >> std::ws;

			if (col == "location") {
				if (ss >> uri >> col) {
					if (!(ss.eof() && col == "{"))
						throw ConfigParser::InvalidConfigException();
				} else if (std::getline(ifs, line)) {
					if (trim(line) != "{")
						throw ConfigParser::InvalidConfigException();
				} else {
					throw ConfigParser::InvalidConfigException();
				}

				serv.location.push_back(locationData());
				serv.location.back().uri = uri;
				parseLocation(ifs, serv.location.back());
			} else if (servMapping.find(col) != servMapping.end()) {
				(this->*(_funcMaping[col]))(
					ss.str().substr(ss.tellg()), servMapping[col]
				);
			} else {
				throw ConfigParser::InvalidConfigException();
			}
		}
	}
}

void	ConfigParser::parseLocation(std::ifstream &ifs, t_locationData &loc) {
	std::stringstream	ss;
	std::string			line, col;
	std::map<std::string, uintptr_t>	locMapping;

	initLocPtrMapping(locMapping, loc);

	while (std::getline(ifs, line)) {
		trim(line);
		if (!(line.empty() || line[0] == '#')) {
			if (line == "}")
				break;

			ss.clear();
			ss.str(line);
			ss >> col >> std::ws;

			if (locMapping.find(col) != locMapping.end())
				(this->*(_funcMaping[col]))(
					ss.str().substr(ss.tellg()), locMapping[col]
				);
			else
				throw ConfigParser::InvalidConfigException();
		}
	}
}

void	ConfigParser::parseListen(const std::string &s, uintptr_t p) {
	std::istringstream		iss(parseHelper(s));
	t_listenData			lsn;
	std::string				col;
	std::string::size_type	pos;

	iss >> col;
	if (iss.eof()) {
		if ((pos = col.rfind(':')) != std::string::npos) {
			lsn.addr = col.substr(0, pos);
			iss.seekg(pos + 1);
		} else {
			iss.seekg(0);
		}
		if (iss >> lsn.port) {
			deserialize<std::vector<t_listenData> >(p)->push_back(lsn);
			return ;
		}
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseServName(const std::string &s, uintptr_t p) {
	std::istringstream			iss(parseHelper(s));
	std::vector<std::string>	name;
	std::string					col;

	while (iss >> col)
		name.push_back(col);
	*deserialize<std::vector<std::string> >(p) = name;
}

void	ConfigParser::parseRoot(const std::string &s, uintptr_t p) {
	std::istringstream		iss(parseHelper(s));
	std::string				col;
	std::string::size_type	pos;

	iss >> col;
	if (iss.eof()) {
		if (col[pos = col.length() - 1] == '/')
			col.erase(pos);
		*deserialize<std::string>(p) = col;
		return ;
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseIndex(const std::string &s, uintptr_t p) {
	std::istringstream			iss(parseHelper(s));
	std::vector<std::string>	id;
	std::string					col;

	while (iss >> col)
		id.push_back(col);
	*deserialize<std::vector<std::string> >(p) = id;
}

void	ConfigParser::parseAutoIndex(const std::string &s, uintptr_t p) {
	const std::string	res = parseHelper(s);

	if (res == "on" || res == "off") {
		*deserialize<std::string>(p) = res;
		return ;
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseCliMaxBodySize(const std::string &s, uintptr_t p) {
	std::istringstream	iss(parseHelper(s));
	std::string			col;

	iss >> col;
	if (iss.eof()) {
		*deserialize<std::string>(p) = col;
		return ;
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseErrPage(const std::string &s, uintptr_t p) {
	std::istringstream	iss(parseHelper(s));
	t_errorPageData		err;
	int					col;

	while (iss >> col)
		err.code.push_back(col);
	iss.clear();
	iss >> err.uri;
	if (iss.eof()) {
		deserialize<std::vector<t_errorPageData> >(p)->push_back(err);
		return;
	}
	throw ConfigParser::InvalidConfigException();
}

void	ConfigParser::parseLimitExcept(const std::string &s, uintptr_t p) {
	std::istringstream		iss(parseHelper(s));
	std::set<std::string>	allow;
	std::set<std::string>	lim;
	std::string				col;

	allow.insert("GET");
	allow.insert("HEAD");
	allow.insert("POST");
	allow.insert("DELETE");

	while (iss >> col) {
		if (std::find(allow.begin(), allow.end(), col) != allow.end())
			lim.insert(col);
		else
			throw ConfigParser::InvalidConfigException();
	}
	*deserialize<std::set<std::string> >(p) = lim;
}

void	ConfigParser::parseCgiPass(const std::string &s, uintptr_t p) {
	std::istringstream	iss(parseHelper(s));
	std::string			col;

	iss >> col;
	if (iss.eof()) {
		*deserialize<std::string>(p) = col;
		return ;
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
	throw ConfigParser::InvalidConfigException();
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
