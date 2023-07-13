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
	(void)src;
	return *this;
}

void	RequestParser::readRequest(const std::string &src) {
	std::istringstream	srcIss(src);
	std::string	line;

	if (std::getline(srcIss, line)) {
		if (!line.empty()) {
			parseRequestLine(line);
			parseHeaders(srcIss);
		}
	}
}

void	RequestParser::parseRequestLine(const std::string &line) {
	std::string::size_type	firstSpace = line.find(' ');
	std::string::size_type	lastSpace = line.rfind(' ');

	_method = line.substr(0, firstSpace);
	_path = line.substr(firstSpace + 1, lastSpace - (firstSpace + 1));
	_version = line.substr(lastSpace + 1, 8);
}

void	RequestParser::parseHeaders(std::istringstream &src) {
	std::string	line;
	std::string::size_type	delimPos;

	while (std::getline(src, line)) {
		if (line.empty())
			return ;
		if ((delimPos = line.find(": ")) != std::string::npos)
			_headers.insert(std::make_pair(
				line.substr(0, delimPos), line.substr(delimPos + 2)
			));
	}
}

const std::string	&RequestParser::getMethod() const {
	return _method;
}

const std::string	&RequestParser::getPath() const {
	return _path;
}

const std::string	&RequestParser::getVersion() const {
	return _version;
}

const std::map<std::string, std::string>	&RequestParser::getHeaders() const {
	return _headers;
}
