#include "Response.hpp"

Response::Response(void) {
	// std::cout << "Default constructor called by <Response>" << std::endl;
}

Response::Response(const Response &src) {
	// std::cout << "Copy constructor called by <Response>" << std::endl;
	*this = src;
}

Response::Response(const std::string &src) {
	_request.readRequest(src);
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

void	Response::setResponse() {
	std::string			statLine, headers;
	std::ostringstream	oss;

	setStatus("200");
	setHeaders();
	setMessageBody();

	oss << _request.getVersion() << " "
		<< _statCode << " "
		<< _reasonPhrase << "\r\n";
	statLine = oss.str();
	oss.str("");
	oss.clear();

	oss << _headers.at("Date") << "\r\n"
		<< _headers.at("Content-Length") << "\r\n"
		<< _headers.at("Content-Type") << "\r\n";
	headers = oss.str();
	oss.str("");
	oss.clear();

	oss << statLine
		<< headers
		<< "\r\n"
		<< _msgBody;
	_response = oss.str();
}

void	Response::setHeaders() {
	setDate();
	setContentLength();
	setContentType();
}

void	Response::setMessageBody() {
	std::string		path = "a.txt"; // It has to be changed to "config.root + request.path"
	std::ifstream	ifs(path.c_str());

	ifs >> _msgBody;
}

void	Response::setDate() {
	std::time_t	t = std::time(0);
	char		mbstr[100];

	if (std::strftime(mbstr, sizeof(mbstr),
			"%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&t)))
		_headers["Date"] = "Date: " + std::string(mbstr);
}

void	Response::setContentLength() {
	std::string	path = "a.txt"; // It has to be changed to "config.root + request.path"
	struct stat	statBuf;
    int			rc = stat(path.c_str(), &statBuf);

	std::ostringstream	oss;
	oss << (rc == 0 ? statBuf.st_size : -1);

	_headers["Content-Length"] = "Content-Length: " + oss.str();
}

// https://www.iana.org/assignments/media-types/media-types.xhtml
void	Response::setContentType() {
	std::string::size_type	extPos = _request.getPath().rfind('.') + 1;
	std::string				ext = _request.getPath().substr(extPos);
	std::string				type = "text/plain";

	if (ext == "jpg")
		type = "image/jpeg";
	else if (ext == "png")
		type = "image/png";
	else if (ext == "css")
		type = "text/css";
	else if (ext == "html")
		type = "text/html";

	_headers["Content-Type"] = "Content-Type: " + type;
}

void	Response::setStatus(const std::string &code) {
	if (code == "200")
		_reasonPhrase = "OK";
	else if (code == "201")
		_reasonPhrase = "Created";
	else if (code == "400")
		_reasonPhrase = "Bad Request";
	else if (code == "401")
		_reasonPhrase = "Unauthorized";
	_statCode = code;
}

void	Response::methodGet() {

}

void	Response::methodPost() {

}

void	Response::methodDelete() {

}

const std::string	&Response::getMessageBody() const {
	return _msgBody;
}

const std::string	&Response::getResponse() const {
	return _response;
}
