#include "RequestParser.hpp"

RequestParser::RequestParser(void) : _msgLen(0), _readLen(0)
{
}

RequestParser::RequestParser(const RequestParser &src) : _msgLen(0), _readLen(0)
{
	*this = src;
}

RequestParser::~RequestParser(void)
{
	delete[] _msgBody;
}

RequestParser &RequestParser::operator= (const RequestParser &src)
{
	_reqLine.method = src.getMethod();
	_reqLine.uri = src.getUri();
	_reqLine.query = src.getQuery();
	_reqLine.version = src.getVersion();
	_headers = src.getHeaders();
	_msgBody = src._msgBody;
	return *this;
}

size_t	RequestParser::readToBuf(int sockfd, char *&buf)
{
	size_t	bufsize = _readLen;
	char	recvbuf[MAXLINE] = {0};
	int		tmpReadLen = 0;

	do {
		tmpReadLen = recv(sockfd, recvbuf, MAXLINE, 0);
		// std::cout << tmpReadLen << " ";
		if (tmpReadLen == 0)
		{
			return bufsize > _readLen ? bufsize : 0;
		}
		else if (tmpReadLen < 0)
		{
			perror("recv");
			return 0;
		}
		buf = myRealloc(buf, bufsize, bufsize + tmpReadLen + 1);
		memmove(buf + bufsize, recvbuf, tmpReadLen);
		buf[tmpReadLen] = 0;
		bufsize += tmpReadLen;
	} while (tmpReadLen >= MAXLINE);
	return bufsize;
}

void	RequestParser::readRequest(int sockfd)
{
	char	*buf = NULL;
	size_t	tmpReadLen = 0;
	do
	{
		tmpReadLen = readToBuf(sockfd, buf);
		if (tmpReadLen == 0)
		{
			delete[] buf;
			return ;
		}
		_readLen += tmpReadLen;
	} while (strstr(buf, "\r\n\r\n") == NULL);

	std::istringstream	iss(buf);
	std::string			line;
	while (std::getline(iss, line))
		if (!line.empty())
			break;
	parseRequestLine(line);
	parseHeaders(iss);
	// std::cout << getUri() << " ";
	try {
		parseMessageBody(iss.tellg(), sockfd, buf);
	} catch (int) {
		return ;
	}

	delete[] buf;
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
			val = ss.str().substr(ss.tellg());
			if (val.find("\r") != std::string::npos)
				val.erase(val.find("\r"));
			_headers[key] = val;
		}
	}
}

void	RequestParser::parseMessageBody(int i, int sockfd, char *&buf) {
	size_t	tmpReadLen = 0;
	if (getMethod() == "POST") {
		while ((int)(_msgLen = _readLen - i) < atoi(_headers.at("Content-Length").c_str())) {
			tmpReadLen = readToBuf(sockfd, buf);
			if (tmpReadLen == 0) {
				delete[] buf;
				throw 0;
			}
			_readLen += tmpReadLen;
		}
	}
	_msgLen = _readLen - i;
	_msgBody = new char[_msgLen + 1];
	memmove(_msgBody, buf + i, _msgLen);
	_msgBody[_msgLen] = 0;
}

// void RequestParser::setContentLength()
// {
// 	std::ostringstream oss;

// 	oss << strlen(_msgBody);
// 	_headers["Content-Length"] = oss.str();
// }

char	**RequestParser::toEnv(const t_locationData &servLoc, char **&env) {
	std::map<std::string, std::string>	envMap;
	std::string				host, key, line;
	std::string::size_type	pos;

	host = _headers.at("Host");

	envMap["SERVER_PROTOCOL"] = getVersion();
	if ((pos = host.find(':')) != std::string::npos)
		envMap["SERVER_PORT"] = host.substr(pos + 1);
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

const char	*RequestParser::getMessageBody() const {
	return _msgBody;
}

const size_t	&RequestParser::getMessageBodyLen() const {
	return _msgLen;
}

const size_t	&RequestParser::getReadLen() const {
	return _readLen;
}
