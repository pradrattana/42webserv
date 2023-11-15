#include "RequestParser.hpp"

RequestParser::RequestParser(void): _msgLen(0), _readLen(0)
{
}

RequestParser::RequestParser(const RequestParser &src): _msgLen(0), _readLen(0)
{
	*this = src;
}

RequestParser::~RequestParser(void)
{
}

RequestParser &RequestParser::operator=(const RequestParser &src)
{
	if (this != &src)
	{
		_reqLine = src._reqLine;
		_headers = src._headers;
		_msgLen = src._msgLen;
		_readLen = src._readLen;
	}
	return *this;
}

int	RequestParser::readRequest(int sockfd)
{
	char	*buf = NULL;
	size_t	tmpReadLen;

	if ((tmpReadLen = readToBuf(sockfd, buf)) == 0)
	{
		free(buf);
		return 0;
	}
	_readLen += tmpReadLen;

	std::istringstream	iss(buf);
	std::string			line;

	if (!std::getline(iss, line))
		return 400;
	if (!parseRequestLine(line))
		return 400;
	if (!parseHeaders(iss))
		return 400;

	FILE	*fp = fopen("myfile.bin", "wb");
	if (fp == NULL)
	{
		fclose(fp);
		free(buf);
		return 500;
	}
	try
	{
		parseMessageBody(sockfd, fileno(fp));
	}
	catch (int)
	{
		fclose(fp);
		free(buf);
		return 0;
	}

	fclose(fp);
	free(buf);
	return 200;
}

// int	RequestParser::readRequest(int sockfd)
// {
// 	char	*buf = NULL;
// 	size_t	tmpReadLen = 0;

// 	do
// 	{
// 		if ((tmpReadLen = readToBuf(sockfd, buf)) == 0)
// 		{
// 			free(buf);
// 			return 0;
// 		}
// 		_readLen += tmpReadLen;
// 	} while (strstr(buf, "\r\n\r\n") == NULL);

// 	std::istringstream	iss(buf);
// 	std::string			line;

// 	if (!std::getline(iss, line))
// 		return 400;
// 	if (!parseRequestLine(line))
// 		return 400;
// 	if (!parseHeaders(iss))
// 		return 400;

// 	try
// 	{
// 		parseMessageBody(iss.tellg(), sockfd, buf);
// 		if (_msgLen > 0)
// 		{
// 			FILE	*fp = fopen("myfile.bin", "wb");
// 			if (fp == NULL)
// 			{
// 				free(buf);
// 				return 500;
// 			}
// 			fwrite(_msgBody, sizeof(_msgBody[0]), _msgLen, fp);
// 			fclose(fp);
// 		}
// 	}
// 	catch (int)
// 	{
// 		free(buf);
// 		return 0;
// 	}

// 	free(buf);
// 	return 200;
// }

size_t	RequestParser::readToBuf(int sockfd, char *&buf)
{
	char	recvbuf;
	size_t	bufsize = 0;
	int		tmpReadLen = 0;

	do
	{
		tmpReadLen = recv(sockfd, &recvbuf, 1, 0);
		if (tmpReadLen == 0)
		{
			return 0;
		}
		else if (tmpReadLen < 0)
		{
			perror("recv");
			return 0;
		}
		buf = (char *)realloc(buf, bufsize + tmpReadLen + 1);
		memmove(buf + bufsize, &recvbuf, tmpReadLen);
		buf[bufsize + tmpReadLen] = 0;
		bufsize += tmpReadLen;
	} while (strstr(buf, "\r\n\r\n") == NULL);
	return bufsize;
}

size_t	RequestParser::readToFile(int sockfd, int filefd)
{
	char	recvbuf[MAXLINE] = {0};
	int		tmpReadLen = 0;

	tmpReadLen = recv(sockfd, recvbuf, MAXLINE, 0);
	if (tmpReadLen == 0)
	{
		return 0;
	}
	else if (tmpReadLen < 0)
	{
		perror("recv");
		return 0;
	}
	write(filefd, recvbuf, tmpReadLen);
	return tmpReadLen;
}

bool	RequestParser::parseRequestLine(const std::string &line)
{
	std::string::size_type	spPos1, spPos2, queryPos;
	std::set<std::string>	allow;

	if (*(line.end() - 1) != '\r') // if not cr at the end
		return false;

	allow.insert("GET");
	allow.insert("HEAD");
	allow.insert("POST");
	allow.insert("PUT");
	allow.insert("DELETE");
	allow.insert("CONNECT");
	allow.insert("OPIONS");
	allow.insert("TRACE");
	allow.insert("PATCH");

	if ((spPos1 = line.find(' ')) != std::string::npos)
		_reqLine.method = line.substr(0, spPos1);
	if (std::find(allow.begin(), allow.end(), _reqLine.method) == allow.end())
		return false;
	// std::cout << _reqLine.method << '\n';

	if ((spPos2 = line.find(' ', spPos1 + 1)) != std::string::npos)
		_reqLine.uri = line.substr(spPos1 + 1, spPos2 - spPos1 - 1);
	if ((queryPos = _reqLine.uri.find('?')) != std::string::npos)
	{
		_reqLine.query = _reqLine.uri.substr(queryPos + 1);
		_reqLine.uri.erase(queryPos);
		if (!isURIQueryValid(_reqLine.query))
			return false;
	}
	if (*_reqLine.uri.begin() != '/')
		return false;
		// _reqLine.uri.insert(_reqLine.uri.begin(), '/');
	if (!isURIPathValid(_reqLine.uri))
		return false;
	// std::cout << _reqLine.uri << '\n';

	_reqLine.version = line.substr(spPos2 + 1, 8);
	if (_reqLine.version.find("HTTP/") != 0)
		return false;
	if (!isdigit(_reqLine.version[5]) || _reqLine.version[6] != '.' || !isdigit(_reqLine.version[7]))
		return false;
	// std::cout << _reqLine.version << '\n';
	
	return true;
}

bool	RequestParser::parseHeaders(std::istringstream &src)
{
	std::string	line, key, val;
	std::string::size_type	valPos;

	while (std::getline(src, line))
	{
		if (line == "\r")
			return true;
		
		if (*(line.end() - 1) != '\r') // if not cr at the end
			return false;

		if ((valPos = line.find(": ")) == std::string::npos)
			return false;
		key = line.substr(0, valPos);
		val = line.substr(valPos + 2, line.length() - valPos - 3);
		_headers[key] = val;
		// std::cout << key << ": " << val << '\n';
	}
	return false;
}

void	RequestParser::parseMessageBody(int sockfd, int filefd)
{
	size_t	bufsize = 0;
	size_t	tmpReadLen = 0;

	if (getMethod() == "POST")
	{
		_msgLen = atoi(_headers.at("Content-Length").c_str());
		while (bufsize < _msgLen)
		{
			tmpReadLen = readToFile(sockfd, filefd);
			if (tmpReadLen == 0)
			{
				throw 0;
			}
			bufsize += tmpReadLen;
		}
		std::cout << "msglen = " << _msgLen << '\n';
		std::cout << "bufsize = " << bufsize << '\n';
	}
	_readLen += bufsize;
}

// void	RequestParser::parseMessageBody(int i, int sockfd, char *&buf)
// {
// 	size_t	tmpReadLen = 0;

// 	if (getMethod() == "POST")
// 	{
// 		_msgLen = atoi(_headers.at("Content-Length").c_str());
// 		while (_readLen - i < _msgLen)
// 		{
// 			tmpReadLen = readToBuf(sockfd, buf);
// 			if (tmpReadLen == 0)
// 			{
// 				throw 0;
// 			}
// 			_readLen += tmpReadLen;
// 		}
// 	}
// 	_msgBody = new char[_msgLen + 1];
// 	memmove(_msgBody, buf + i, _msgLen);
// 	_msgBody[_msgLen] = 0;
// }

char	**RequestParser::toEnv(const t_locationData &servLoc, char **&env)
{
	std::map<std::string, std::string>	envMap;
	std::string				host, key, line;
	std::string::size_type	pos;

	host = _headers.at("Host");

	envMap["SERVER_PROTOCOL"] = getVersion();
	if ((pos = host.find(':')) != std::string::npos)
		envMap["SERVER_PORT"] = host.substr(pos + 1);
	envMap["REQUEST_METHOD"] = getMethod();

	pos = getUri().find('.');
	if ((pos = getUri().find('/', pos)) != std::string::npos)
	{
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

const std::string	&RequestParser::getMethod() const
{
	return _reqLine.method;
}

const std::string	&RequestParser::getUri() const
{
	return _reqLine.uri;
}

const std::string	&RequestParser::getQuery() const
{
	return _reqLine.query;
}

const std::string	&RequestParser::getVersion() const
{
	return _reqLine.version;
}

const std::map<std::string, std::string>	&RequestParser::getHeaders() const
{
	return _headers;
}

// const char	*RequestParser::getMessageBody() const
// {
// 	return _msgBody;
// }

const size_t	&RequestParser::getMessageBodyLen() const
{
	return _msgLen;
}

// const size_t	&RequestParser::getReadLen() const
// {
// 	return _readLen;
// }

RequestParser::t_reqLine &RequestParser::t_reqLine::operator=(const t_reqLine &src)
{
	if (this == &src)
	{
		method = src.method;
		uri = src.uri;
		query = src.query;
		version = src.version;
	}
	return *this;
}
