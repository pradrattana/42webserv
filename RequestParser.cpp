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
		if (!line.empty()) {
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
			// throw
		}
	}
}

const std::string	RequestParser::toEnv(const t_locationData &servLoc) const {
	std::ostringstream		oss;
	std::string				host, extrauri, key;
	std::string::size_type	pos;

	host = _headers.at("Host");
	extrauri = getUri().substr(getUri().find_first_not_of(servLoc.cgiPass));

	oss << "SERVER_PROTOCOL=\"" << getVersion() << "\"\n";
	if ((pos = host.find(':')) != std::string::npos)
		oss << "SERVER_PORT=\"" << host.substr(host.find(':') + 1) << "\"\n";
	oss << "REQUEST_METHOD=\"" << getMethod() << "\"\n";

	oss << "SCRIPT_NAME=\"" << servLoc.cgiPass << "\"\n";
	oss << "PATH_INFO=\"" << extrauri << "\"\n";
	oss << "PATH_TRANSLATED=\"" << std::getenv("PWD") << servLoc.root << extrauri << "\"\n";
	oss << "QUERY_STRING=\"" << getQuery() << "\"\n";

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
			it != _headers.end(); it++)
	{
		key = it->first;
		while ((pos = key.find('-')) != std::string::npos)
			key.replace(pos, 1, "_");
		std::transform(key.begin(), key.end(), key.begin(), ::toupper);
		oss << "HTTP_" << key << "=\"" << it->second << "\"\n";
	}

	return oss.str();
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
