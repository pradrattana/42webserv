#include "RequestParser.hpp"

RequestParser::RequestParser(void) {
	// std::cout << "Default constructor called by <RequestParser>" << std::endl;
}

RequestParser::RequestParser(const RequestParser &src) {
	// std::cout << "Copy constructor called by <RequestParser>" << std::endl;
	*this = src;
}

RequestParser::RequestParser(const std::string &src) {
	readRequest(src);
}

RequestParser::~RequestParser(void) {
	// std::cout << "Destructor called by <RequestParser>" << std::endl;
}

RequestParser &RequestParser::operator= (const RequestParser &src) {
	// std::cout << "Copy assignment operator called by <RequestParser>" << std::endl;
	_reqLine.method = src.getMethod();
	_reqLine.uri = src.getUri();
	_reqLine.query = src.getQuery();
	_reqLine.version = src.getVersion();
	_headers = src.getHeaders();
	_msgBody = src._msgBody;
	return *this;
}

void	RequestParser::readRequest(const std::string &src) {
	std::istringstream	iss(src);
	std::string			line;

	while (std::getline(iss, line))
		if (!line.empty())
			break;
	parseRequestLine(line);
	parseHeaders(iss);
	parseMessageBody(iss);
}

void	RequestParser::parseRequestLine(const std::string &line) {
	std::istringstream		iss(line);
	std::string::size_type	queryPos;

	if (iss >> _reqLine.method >> _reqLine.uri >> _reqLine.version >> std::ws) {
		if ((queryPos = getUri().find('?')) != std::string::npos) {
			_reqLine.query = _reqLine.uri.substr(queryPos + 1);
			_reqLine.uri.erase(queryPos);
		}
		if (_reqLine.uri[0] != '/')
			_reqLine.uri.insert(_reqLine.uri.begin(), '/');
	}
	// throw
}

void	RequestParser::parseHeaders(std::istringstream &src) {
	std::stringstream	ss;
	std::string			line, key, val;

	while (std::getline(src, line)) {
		if (line.empty() || line == "\r")
			break;

		ss.clear();
		ss.str(line);
		ss >> key >> std::ws;

		if (key[key.length() - 1] == ':') {
			key.erase(key.length() - 1);
			if (ss >> val >> std::ws) {
				_headers[key] = val;
				continue;
			}
		}
	}
}

void	RequestParser::parseMessageBody(std::istringstream &src) {
	try {
		_msgBody = src.str().substr(src.tellg());
	} catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
	}
	
}

char	**RequestParser::toEnv(const t_locationData &servLoc, char **&env) {
	std::map<std::string, std::string>	envMap;
	std::string				host, key, line;
	std::string::size_type	pos;

	host = _headers.at("Host");

	envMap["SERVER_PROTOCOL"] = getVersion();
	if ((pos = host.find(':')) != std::string::npos)
		envMap["SERVER_PORT"] = host.substr(host.find(':') + 1);
	envMap["REQUEST_METHOD"] = getMethod();

	pos = getUri().find('.');
	if ((pos = getUri().find('/', pos)) != std::string::npos) {
		envMap["PATH_INFO"] = getUri().substr(pos);
		envMap["PATH_TRANSLATED"] = servLoc.root + getUri().substr(pos);
	}
	envMap["SERVER_PROTOCOL"] = getVersion();
	envMap["SCRIPT_NAME"] = getUri().substr(0, pos);
	envMap["SCRIPT_FILENAME"] = servLoc.root + getUri().substr(0, pos);
	if (!getQuery().empty())
		envMap["QUERY_STRING"] = getQuery();

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
			it != _headers.end(); it++)
	{
		key = it->first;
		while ((pos = key.find('-')) != std::string::npos)
			key.replace(pos, 1, "_");
		std::transform(key.begin(), key.end(), key.begin(), ::toupper);
		if (key == "CONTENT_LENGTH" || key == "CONTENT_TYPE")
			envMap[key] = it->second;
		else
			envMap["HTTP_" + key] = it->second;
	}

	env = new char*[envMap.size() + 2];
	int	i = 0;
	for (std::map<std::string, std::string>::const_iterator it = envMap.begin();
			it != envMap.end(); it++)
	{
		line = it->first + "=";
		line += it->second;
		env[i] = new char[line.length() + 1];
		strcpy(env[i++], line.c_str());
	}
	line = "REDIRECT_STATUS=200";
	env[i] = new char[line.length() + 1];
	strcpy(env[i++], line.c_str());
	env[i] = NULL;
	return env;
}

const std::string	&RequestParser::getMethod() const {
	return _reqLine.method;
}

const std::string	&RequestParser::getUri() const {
	return _reqLine.uri;
}

const std::string	&RequestParser::getQuery() const {
	return _reqLine.query;
}

const std::string	&RequestParser::getVersion() const {
	return _reqLine.version;
}

const std::map<std::string, std::string>	&RequestParser::getHeaders() const {
	return _headers;
}

const std::string	&RequestParser::getMessageBody() const {
	return _msgBody;
}
