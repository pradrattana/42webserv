#include "Response.hpp"
#include "colour.hpp"

Response::Response(void)
{
	// std::cout << "Default constructor called by <Response>" << std::endl;
	initStatusMapping();
}

Response::Response(const Response &src)
{
	// std::cout << "Copy constructor called by <Response>" << std::endl;
	*this = src;
}

Response::Response(const std::set<t_serverData> &serv, const std::string &buf)
{
	initStatusMapping();
	std::cout << CYN << buf << WHT << std::endl;
	_request.readRequest(buf);
	for (std::map<std::string, std::string>::const_iterator it = _request.getHeaders().begin();
		 it != _request.getHeaders().end(); it++)
		std::cout << CYN << it->first << ": " << it->second << WHT << std::endl;
	try
	{
		setRequestLocation(serv);
		methodHandler();
	}
	catch (const int &e)
	{
		_code = e;
	}
	catch (...)
	{
		return;
	}
	setResponse();
}

Response::~Response(void)
{
	// std::cout << "Destructor called by <Response>" << std::endl;
}

Response &Response::operator=(const Response &src)
{
	// std::cout << "Copy assignment operator called by <Response>" << std::endl;
	(void)src;
	return *this;
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

	_statMaping[500] = "Internal Server Error";
	_statMaping[501] = "Not Implemented";
	_statMaping[502] = "Bad Gateway";
	_statMaping[503] = "Service Unavailable";
	_statMaping[504] = "Gateway Time-out";
	_statMaping[505] = "HTTP Version not supported";
}

void Response::setRequestLocation(const std::set<t_serverData> &serv)
{
	t_listenData lsn;
	std::string::size_type pos;

	lsn.addr = _request.getHeaders().at("Host");
	if ((pos = lsn.addr.find(':')) != std::string::npos)
	{
		lsn.port = atoi(lsn.addr.substr(pos + 1).c_str());
		lsn.addr.erase(pos);
	}
	else
	{
		// lsn.port = 80;
	}

	for (std::set<t_serverData>::const_iterator it = serv.begin();
		 it != serv.end(); it++)
	{
		if (isIPv4(lsn.addr) || std::find(it->name.begin(), it->name.end(), lsn.addr) != it->name.end())
		{
			for (std::vector<t_locationData>::const_iterator it2 = it->location.begin();
				 it2 != it->location.end(); it2++)
			{
				for (std::vector<t_listenData>::const_iterator it3 = it2->listen.begin();
					 it3 != it2->listen.end(); it3++)
				{
					// std::cout << it2->location
					if ((!isIPv4(lsn.addr) || it3->addr == lsn.addr) && it3->port == lsn.port)
					{
						if (_request.getUri().find(it2->uri) == 0)
						{
							_reqLoc = *it2;
							return;
						}
					}
				}
			}
		}
	}
	throw 400;
}

void Response::setResponse()
{
	std::ostringstream oss;

	if (_code != 200)
	{
		// std::cout << "error " << _code << " ";
		setErrorPath();
		// std::cout << "path: " << _fullPath << "\n";
		setMessageBody();
		setContentType();
		setContentLength();
	}
	oss << getStatusLine() << getHeadersText() << "\r\n"
		<< _msgBody;
	_response = oss.str();
}

void Response::setMessageBody()
{
	std::ostringstream oss;
	std::ifstream ifs(_fullPath.c_str());

	_msgBody.assign(std::istreambuf_iterator<char>(ifs),
					std::istreambuf_iterator<char>());
	ifs.close();
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
		oss << _reqLoc.root << _request.getUri();
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
	oss << std::getenv("PWD") << "/webserv_default_error/error.html";
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
		<< (*_request.getUri().begin() == '/' ? "" : "/")
		<< _request.getUri()
		<< '/';

	std::cout << "oos = " << oss.str();
	_headers["Location"] = oss.str();
}

void Response::setContentLength()
{
	std::ostringstream oss;

	oss << _msgBody.length();
	_headers["Content-Length"] = oss.str();
}

// https://www.iana.org/assignments/media-types/media-types.xhtml
void Response::setContentType()
{
	std::string::size_type pos = _fullPath.rfind('.') + 1;
	std::string type = "text/plain";

	if (pos != std::string::npos)
	{
		std::string ext = _fullPath.substr(pos);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		if (ext == "jpeg" || ext == "jpg")
			type = "image/jpeg";
		else if (ext == "png")
			type = "image/png";
		else if (ext == "css")
			type = "text/css";
		else if (ext == "html")
			type = "text/html";
	}

	// if (_request.getHeaders().at("Accept").find("*/*") == std::string::npos) {
	// 	if (_request.getHeaders().at("Accept").find(type) == std::string::npos)
	// 		throw 406;
	// }

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

		if (std::find(_reqLoc.limExcept.begin(), _reqLoc.limExcept.end(),
					  _request.getMethod()) != _reqLoc.limExcept.end())
			(this->*method[_request.getMethod()])();
		else if (_request.getMethod() == "POST")
			(this->*method[_request.getMethod()])();
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
	else
	{
		setMessageBody();
		setContentLength();
		setContentType();
	}

	_code = 200;
}

void Response::methodPost()
{
	std::cout << "methodPost" << std::endl;
	std::cout << getHeadersText() << std::endl;
	// size_t bytes = recv()
}

void Response::methodDelete() {}

const std::string Response::toEnv() const
{
	std::ostringstream oss;
	std::string key;
	std::string::size_type pos;

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		 it != _headers.end(); it++)
	{
		key = it->first;
		while ((pos = key.find('-')) != std::string::npos)
			key.replace(pos, 1, "_");
		std::transform(key.begin(), key.end(), key.begin(), ::toupper);
		oss << key << "=\"" << it->second << "\"\n";
	}

	return oss.str();
}

const std::string &Response::getResponse() const
{
	return _response;
}

const std::string Response::getStatusLine() const
{
	std::ostringstream oss;

	oss << _request.getVersion() << " "
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

const std::string &Response::getMessageBody() const
{
	return _msgBody;
}

