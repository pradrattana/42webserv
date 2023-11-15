#include "CgiHandler.hpp"

CgiHandler::CgiHandler(): _res(NULL)
{
}

CgiHandler::CgiHandler(const CgiHandler &src)
{
	*this = src;
}

CgiHandler::CgiHandler(Response *res): _res(res)
{
}

CgiHandler::~CgiHandler()
{
}

CgiHandler &CgiHandler::operator=(const CgiHandler &src)
{
	if (this != &src)
	{
		_res = src._res;
	}
	return *this;
}

void	CgiHandler::executeCgi(const std::string &cgi)
{
	int	pp[2];
	int	pid;

	pipe(pp);
	if ((pid = fork()) < 0)
	{
		exit(1);
	}
	else if (pid == 0)
	{
		close(pp[0]);

		FILE	*fp = fopen("myfile.bin", "r");
		dup2(fileno(fp), 0);
		dup2(pp[1], 1);

		char	**env = _res->toEnv(env);
		char	*args[] = { (char *)cgi.c_str(), NULL };
		if (execve(getCgiFullPath(cgi).c_str(), args, env) < 0)
			perror("execve");

		for (int i = 0; env[i] != NULL; i++)
			delete env[i];
		delete[] env;
		fclose(fp);
		close(pp[1]);
		exit(0);
	}
	close(pp[1]);
	waitpid(pid, 0, 0);

	char	buf[1024];
	int		bytes_read = read(pp[0], buf, sizeof(buf));

	close(pp[0]);
	if (bytes_read == -1)
	{
		perror("read in cgi");
	}
	buf[bytes_read] = 0;

	setBodyAndHeaders(buf);
}

const std::string	CgiHandler::getCgiFullPath(const std::string &s) const
{
	std::stringstream	ss(getenv("PATH"));
	std::string			dir, absPath;

	while (std::getline(ss, dir, ':'))
	{
		absPath = dir;
		if (s[0] != '/')
			absPath.append("/");
		absPath.append(s);

		if (access(absPath.c_str(), F_OK) == 0)
			return absPath;
	}
	return "";
}

void	CgiHandler::setBodyAndHeaders(const std::string &buf)
{
	std::stringstream ss(buf);
	std::string line, key, val;
	
	_res->setCode(200);
	while (std::getline(ss, line))
	{
		if (line == "\r")
			break;
		std::string::size_type	pos = line.find(": ");
		key = line.substr(0, pos);
		val = line.substr(pos + 2);
		if ((pos = val.rfind("\r")) != std::string::npos)
			val.erase(pos);
		if (key == "Status")
			_res->setCode(atoi(val.c_str()));
		else
			_res->setHeader(key, val);
	}
	_res->setMessageBody(buf.substr(ss.tellg()));
	// try
	// {
	// 	_res->getHeaders().at("Content-Length");
	// }
	// catch (const std::exception &e)
	// {
	// 	_res->setContentLength();
	// }
}
