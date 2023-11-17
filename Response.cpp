#include "Response.hpp"

Response::Response(void): _cgi(this), _code(0)
{
	initStatusMapping();
}

Response::Response(const Response &src)
{
	*this = src;
}

Response::~Response(void)
{
}

Response &Response::operator=(const Response &src)
{
	if (this != &src)
	{
		_cgi = src._cgi;
		_request = src._request;
		_fullPath = src._fullPath;
		_response = src._response;
		_code = src._code;
		_headers = src._headers;
		_msgBody = src._msgBody;
		_statMaping = src._statMaping;
	}
	return *this;
}

bool	Response::processing(const t_serverData &serv, int sockfd)
{
	int	ret = _request.readRequest(sockfd);
	if (ret == 0)
		return false;

	initStatusMapping();
	try
	{
		setRequestLocation(serv);
		methodHandler();
	}
	catch (int e)
	{
		_code = e;
	}
	catch (...)
	{
		return false;
	}
	setResponse();
	return true;
}

void Response::initStatusMapping()
{
	_statMaping[0] = "";

	_statMaping[200] = "OK";
	_statMaping[201] = "Created";
	_statMaping[202] = "Accepted";
	_statMaping[203] = "Non-Authoritative Information";
	_statMaping[204] = "No Content";
	_statMaping[205] = "Reset Content";
	_statMaping[206] = "Partial Content";

	_statMaping[300] = "Multiple Choices";
	_statMaping[301] = "Moved Permanently";
	_statMaping[302] = "Found";
	_statMaping[303] = "See Other";
	_statMaping[304] = "Not Modified";
	_statMaping[305] = "Use Proxy";
	_statMaping[307] = "Temporary Redirect";

	_statMaping[400] = "Bad Request";
	_statMaping[401] = "Unauthorized";
	_statMaping[403] = "Forbidden";
	_statMaping[404] = "Not Found";
	_statMaping[405] = "Method Not Allowed";
	_statMaping[406] = "Not Acceptable";
	_statMaping[413] = "Content Too Large";
	_statMaping[415] = "Unsupported Media Type";

	_statMaping[500] = "Internal Server Error";
	_statMaping[501] = "Not Implemented";
	_statMaping[502] = "Bad Gateway";
	_statMaping[503] = "Service Unavailable";
	_statMaping[504] = "Gateway Time-out";
	_statMaping[505] = "HTTP Version not supported";
}

void Response::setRequestLocation(const t_serverData &serv)
{
	t_listenData lsn;
	std::string::size_type pos;

	try
	{
		lsn.addr = _request.getHeaders().at("Host");
	}
	catch (const std::exception &e)
	{
		throw 400;
	}
	if ((pos = lsn.addr.find(':')) != std::string::npos)
	{
		lsn.port = atoi(lsn.addr.substr(pos + 1).c_str());
		lsn.addr.erase(pos);
	}
	else
	{
		// lsn.port = 80;
	}

	if (isIPv4(lsn.addr) || std::find(serv.name.begin(), serv.name.end(), lsn.addr) != serv.name.end())
	{
		for (std::vector<t_locationData>::const_iterator it = serv.location.begin();
				it != serv.location.end(); it++)
		{
			for (std::vector<t_listenData>::const_iterator it2 = it->listen.begin();
					it2 != it->listen.end(); it2++)
			{
				if ((!isIPv4(lsn.addr) || it2->addr == lsn.addr) && it2->port == lsn.port)
				{
					if (dropFilename(_request.getUri()).find(it->uri) == 0)
					{
						// std::cout << "get uri " << dropFilename(_request.getUri()) << '\n';
						// std::cout << "reqloc uri " << it->uri << '\n';
						_reqLoc = *it;
						return;
					}
				}
			}
		}
	}
	throw 400;
}

void Response::setResponse()
{
	std::stringstream ss;

	if (_code / 100 == 4 || _code / 100 == 5)
	{
		setErrorPath();
		setMessageBody();
		setContentType();
	}

	try
	{
		_headers.at("Content-length");
	}
	catch (const std::exception &e)
	{
		setContentLength();
	}
	ss << getStatusLine() << getHeadersText() << "\r\n";
	std::copy(_msgBody.begin(), _msgBody.end(), std::ostreambuf_iterator<char>(ss));
	_response.assign(std::istreambuf_iterator<char>(ss), std::istreambuf_iterator<char>());
}

void	Response::setCode(const int c) {
	_code = c;
}

void	Response::setHeader(const std::string &k, const std::string &v) {
	_headers[k] = v;
}

void Response::setMessageBody()
{
	std::ifstream	ifs(_fullPath.c_str(), std::ios::binary);

	if (!ifs.is_open())
		throw 500;
	_msgBody.assign(std::istreambuf_iterator<char>(ifs),
					std::istreambuf_iterator<char>());
	ifs.close();
}

void	Response::setMessageBody(const std::vector<char> &s) {
	_msgBody = s;
}

void	Response::setMessageBody(std::istream &is) {
	_msgBody.assign(std::istreambuf_iterator<char>(is),
					std::istreambuf_iterator<char>());
}

bool Response::setFullPath()
{
	std::ostringstream oss;
	struct stat statBuf;

	// if request index
	if (_request.getUri() == "/")
	{
		for (std::vector<std::string>::const_iterator it = _reqLoc.index.begin();
			 it != _reqLoc.index.end(); it++)
		{
			oss << _reqLoc.root << '/' << *it;
			if (stat(oss.str().c_str(), &statBuf) == 0)
			{
				_fullPath = oss.str();
				return S_ISDIR(statBuf.st_mode);
			}
			oss.clear();
			oss.str("");
		}
	}
	else
	{
		oss << _reqLoc.root;
		// if location doesn't overridden root
		if (!_reqLoc.isRootOvr) 
			oss << _request.getUri();
		else
			oss << _request.getUri().substr(_request.getUri().find(_reqLoc.uri) + _reqLoc.uri.length());
		// std::cout << "full: " << oss.str() << '\n';
		if (stat(oss.str().c_str(), &statBuf) == 0)
		{
			_fullPath = oss.str();
			return S_ISDIR(statBuf.st_mode);
		}
	}
	if (errno == EACCES)
		throw 403;
	else if (errno == ENOENT)
		throw 404;
	return 0;
}

void Response::setErrorPath()
{
	std::ostringstream oss;
	struct stat statBuf;

	if (_code != 400)
	{
		for (std::vector<t_errorPageData>::const_iterator it = _reqLoc.errPage.begin();
			it != _reqLoc.errPage.end(); it++)
		{
			if (std::find(it->code.begin(), it->code.end(), _code) != it->code.end())
			{
				oss << _reqLoc.root << it->uri;
				if (stat(oss.str().c_str(), &statBuf) == 0)
				{
					_fullPath = oss.str();
					return;
				}
			}
		}
	}
	oss << std::getenv("PWD") << "/webserv_default_error/error.html";
	// oss << std::getenv("PWD") << "/webserv_default_error/error" << _code << ".html";
	_fullPath = oss.str();
}

void Response::setDate()
{
	std::time_t t = std::time(0);
	char mbstr[100];

	if (std::strftime(mbstr, sizeof(mbstr),
					  "%a, %d %b %Y %T GMT", std::gmtime(&t)))
	{
		_headers["Date"] = std::string(mbstr);
	}
}

void Response::setLocation()
{
	std::ostringstream oss;

	oss << "http://"
		<< _request.getHeaders().at("Host")
		// << (*_request.getUri().begin() == '/' ? "" : "/")
		<< _request.getUri()
		<< '/';

	_headers["Location"] = oss.str();
}

void Response::setLocation(const std::string &s)
{
	_headers["Location"] = s;
}

void Response::setContentLength()
{
	std::ostringstream oss;

	oss << _msgBody.size();
	_headers["Content-length"] = oss.str();
}

// https://www.iana.org/assignments/media-types/media-types.xhtml
void Response::setContentType()
{
	std::string type = "text/plain";
	std::string::size_type pos = _fullPath.rfind('.') + 1;

	if (pos != std::string::npos)
	{
		std::string ext = _fullPath.substr(pos);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		if (ext == "jpeg" || ext == "jpg")
			type = "image/jpeg";
		else if (ext == "svg")
			type = "image/svg+xml";
		else if (ext == "png" || ext == "gif" || ext == "bmp")
			type = "image/" + ext;
		else if (ext == "css" || ext == "csv" || ext == "javascript" || ext == "html")
			type = "text/" + ext;
		else if (ext == "json")
			type = "application/json";
		// else
		// 	throw 415;
	}

	_headers["Content-Type"] = type;
}

void Response::methodHandler()
{
	std::map<std::string, void (Response::*)()> method;

	method["GET"] = &Response::methodGet;
	method["POST"] = &Response::methodPost;
	method["DELETE"] = &Response::methodDelete;

	if (method.find(_request.getMethod()) != method.end())
	{

		std::cout << _request.getMethod() << "\n";
		if (std::find(_reqLoc.limExcept.begin(), _reqLoc.limExcept.end(),
					  _request.getMethod()) != _reqLoc.limExcept.end())
		{
			if (!_reqLoc.redir.url.empty())
			{
				setLocation(_reqLoc.redir.url);
				throw _reqLoc.redir.code;
			}
			(this->*method[_request.getMethod()])();
		}
		else
			throw 405;
	}
	else
	{
		throw 501;
	}
}

void Response::methodGet()
{
	setDate();

	// if full path is directory
	if (setFullPath())
	{
		if (_reqLoc.autoIdx == "on")
		{
			if (*(_fullPath.end() - 1) != '/')
			{
				setLocation();
				throw 301;
			}
			directoryListing();
		}
		else
			throw 403;
	}
	else if (_fullPath.find(".php") == std::string::npos)
	{
		setMessageBody();
		// setContentLength();
		setContentType();
	}
	else
	{
		_cgi.executeCgi(_reqLoc.cgiPass);
		return;
	}

	_code = 200;
}

void Response::methodPost()
{
	setDate();
	if (_reqLoc.cliMax != 0)
		if (_request.getMessageBodyLen() > _reqLoc.cliMax)
			throw 413;
	_cgi.executeCgi(_reqLoc.cgiPass);
}

void Response::methodDelete()
{

}

char	**Response::toEnv(char **&env)
{
	return _request.toEnv(_reqLoc, env);
}

const std::vector<char>	&Response::getResponse() const
{
	return _response;
}

const std::string Response::getStatusLine() const
{
	std::ostringstream oss;

	oss << (_request.getVersion().empty() ? "HTTP/1.1" : _request.getVersion()) << " "
		<< _code << " "
		<< _statMaping.at(_code) << "\r\n";
	return oss.str();
}

const std::string Response::getHeadersText() const
{
	std::ostringstream oss;

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		 it != _headers.end(); it++)
		oss << it->first << ": " << it->second << "\r\n";
	return oss.str();
}

const std::map<std::string, std::string> &Response::getHeaders() const
{
	return	_headers;
}

const RequestParser	&Response::getRequest() const
{
	return _request;
}

std::string	dropFilename(const std::string &path)
{
	std::string::size_type	lastSlashPos = path.rfind('/');
	std::string::size_type	lastDotPos = path.rfind('.');

	if (lastDotPos == std::string::npos || lastDotPos < lastSlashPos)
		return path;
	else
		return path.substr(0, lastSlashPos + 1);
}
