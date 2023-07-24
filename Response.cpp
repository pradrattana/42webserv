#include "Response.hpp"

Response::Response(void) {
	// std::cout << "Default constructor called by <Response>" << std::endl;
	initStatusMapping();
}

Response::Response(const Response &src) {
	// std::cout << "Copy constructor called by <Response>" << std::endl;
	*this = src;
}

Response::Response(const std::set<t_serverData> &serv, const std::string &buf) {
	initStatusMapping();
	_request.readRequest(buf);

	try {
		setRequestLocation(serv);
	} catch (const int &e) {
		_code = e;
		setResponse();
		return ;
	}
	methodHandler();
	setResponse();
}

Response::~Response(void) {
	// std::cout << "Destructor called by <Response>" << std::endl;
}

Response &Response::operator= (const Response &src) {
	// std::cout << "Copy assignment operator called by <Response>" << std::endl;
	(void)src;
	return *this;
}

void	Response::initStatusMapping() {
	_statMaping[0] = "WHAT";

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
	_statMaping[402] = "Payment Required";
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

void	Response::setRequestLocation(const std::set<t_serverData> &serv) {
	t_listenData			lsn;
	std::string::size_type	pos;

	lsn.addr = _request.getHeaders().at("Host");
	if ((pos = lsn.addr.find(':')) != std::string::npos) {
		lsn.port = atoi(lsn.addr.substr(pos + 1).c_str());
		lsn.addr.erase(pos);
	}

	for (std::set<t_serverData>::const_iterator it = serv.begin();
			it != serv.end(); it++)
	{
		for (std::vector<t_locationData>::const_iterator it2 = it->location.begin();
				it2 != it->location.end(); it2++)
		{
			for (std::vector<t_listenData>::const_iterator it3 = it2->listen.begin();
					it3 != it2->listen.end(); it3++)
			{
				if (it3->addr == lsn.addr && it3->port == lsn.port) {
					if (_request.getUri().find(it2->uri) == 0) {
						_reqLoc = *it2;
						return ;
					}
				}
			}
			
		}
	}
	throw 400;
}

void	Response::setResponse() {
	std::ostringstream	oss;

	if (_code != 200) {
		// std::cout << "error\n";
		setErrorPath();
		setMessageBody();
		setContentType();
		// setContentLength();
	}
	oss << getStatusLine() << getHeadersText() << "\r\n" << _msgBody;
	_response = oss.str();
}

void	Response::setMessageBody() {
	std::ostringstream	oss;
	std::ifstream		ifs(_fullPath.c_str());

	_msgBody.assign(std::istreambuf_iterator<char>(ifs),
					std::istreambuf_iterator<char>());
	ifs.close();
}

void	Response::setFullPath() {
	std::ostringstream	oss;
	struct stat			statBuf;

	// if request index
	if (_request.getUri() == "/") {
		for (std::vector<std::string>::const_iterator it = _reqLoc.index.begin();
				it != _reqLoc.index.end(); it++)
		{
			oss << _reqLoc.root << '/' << *it;
			if (stat(oss.str().c_str(), &statBuf) == 0) {
				_fullPath = oss.str();
				return ;
			}
			oss.clear();
			oss.str("");
		}
	} else {
		oss << _reqLoc.root << _request.getUri();
		if (stat(oss.str().c_str(), &statBuf) == 0) {
			_fullPath = oss.str();
			return ;
		}
	}
	throw errno;
}

void	Response::setErrorPath() {
	std::ostringstream	oss;
	struct stat			statBuf;

	for (std::vector<t_errorPageData>::const_iterator it = _reqLoc.errPage.begin();
			it != _reqLoc.errPage.end(); it++)
	{
		if (std::find(it->code.begin(), it->code.end(), _code) != it->code.end()) {
			oss << _reqLoc.root << it->uri;
			if (stat(oss.str().c_str(), &statBuf) == 0) {
				std::cout << "path: " << oss.str() << "\n";
				_fullPath = oss.str();
				return ;
			}
		}
	}
}

void	Response::setDate() {
	std::time_t	t = std::time(0);
	char		mbstr[100];

	if (std::strftime(mbstr, sizeof(mbstr),
			"%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&t)))
	{
		_headers["Date"] = std::string(mbstr);
	}
}

void	Response::setContentLength() {
	// struct stat	statBuf;
    // int			rc = stat(_fullPath.c_str(), &statBuf);

	// std::ostringstream	oss;
	// oss << (rc == 0 ? statBuf.st_size : -1);

	// _headers["Content-Length"] = oss.str();

	_headers["Content-Length"] = _msgBody.length();
}

// https://www.iana.org/assignments/media-types/media-types.xhtml
void	Response::setContentType() {
	std::string::size_type	pos = _fullPath.rfind('.') + 1;
	std::string				type = "text/plain";

	if (pos != std::string::npos) {
		std::string	ext = _fullPath.substr(pos);
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

	_headers["Content-Type"] = type;
}

void	Response::methodHandler() {
	std::map<std::string, void (Response::*)()>	method;

	method["GET"] = &Response::methodGet;
	method["POST"] = &Response::methodPost;
	method["DELETE"] = &Response::methodDelete;

	if (std::find(_reqLoc.limExcept.begin(), _reqLoc.limExcept.end(),
			_request.getMethod()) != _reqLoc.limExcept.end())
	{
		(this->*method[_request.getMethod()])();
	} else {
		_code = 405;
	}
}

void	Response::methodGet() {
	setDate();

	try {
		setFullPath();
	} catch (const int &e) {
		std::cout << "catch " << e << "\n";
		if (e == EACCES)
			_code = 403;
		else if (e == ENOENT)
			_code = 404;
		return ;
	}

	setMessageBody();
	setContentLength();
	setContentType();

	_code = 200;
}

void	Response::methodPost() {}
void	Response::methodDelete() {}

const std::string	Response::toEnv() const {
	std::ostringstream		oss;
	std::string				key;
	std::string::size_type	pos;

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

const std::string	&Response::getResponse() const {
	return _response;
}

const std::string	Response::getStatusLine() const {
	std::ostringstream	oss;

	oss << _request.getVersion() << " "
		<< _code << " "
		<< _statMaping.at(_code) << "\r\n";
	return oss.str();
}

const std::string	Response::getHeadersText() const {
	std::ostringstream	oss;

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
			it != _headers.end(); it++)
		oss << it->first << ": " << it->second << "\r\n";
	return oss.str();
}

const std::string	&Response::getMessageBody() const {
	return _msgBody;
}

