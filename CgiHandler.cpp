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

	if (pipe(pp) < 0)
	{
		perror("pipe in cgi");
		throw 500;
	}
	if ((pid = fork()) < 0)
	{
		perror("fork in cgi");
		throw 500;
	}
	else if (pid == 0)
	{
		close(pp[0]);

		FILE	*fp = fopen("myfile.bin", "rb");
		if (fp == NULL)
			throw 500;

		dup2(fileno(fp), STDIN_FILENO);
		dup2(pp[1], STDOUT_FILENO);

		char	**env = _res->toEnv(env);
		char	*args[] = { (char *)cgi.c_str(), NULL };
		if (execve(getCgiFullPath(cgi).c_str(), args, env) < 0)
		{
			perror("execve in cgi");
			for (int i = 0; env[i] != NULL; i++)
				delete env[i];
			delete[] env;
			throw 500;
		}

		for (int i = 0; env[i] != NULL; i++)
			delete env[i];
		delete[] env;
		close(pp[1]);
		fclose(fp);
		exit(0);
	}
	close(pp[1]);
	waitpid(pid, 0, 0);

	std::vector<char>	data, buf(MAXLINE);
	int	tmpReadLen;
	do
	{
		tmpReadLen = read(pp[0], buf.data(), MAXLINE);
		if (tmpReadLen == -1)
		{
			perror("read in cgi");
			throw 500;
		}
		data.insert(data.end(), buf.begin(), buf.begin() + tmpReadLen);
	} while (tmpReadLen == MAXLINE);
	close(pp[0]);
	data.push_back('\0');

	setBodyAndHeaders(data);
}

void CgiHandler::executeCgiDownload(const std::string &_fullPath){
	int	pipeinfd[2];
	int pipeoutfd[2];

	if (pipe(pipeinfd) == -1 || pipe(pipeoutfd) == -1)
	{
		perror("pipe in cgi");
		throw 500;
	}
	std::string filename = "filename=";
	std::string fullpath = "fullpath=" ;
	std::string filedes = "fd=";
	std::ostringstream num;
	num << pipeoutfd[1];
	filedes.append(num.str());
	fullpath.append(&_fullPath.c_str()[0], _fullPath.length());
	size_t pos = _fullPath.find_last_of('/');
	if (pos != std::string::npos)
		filename.append(_fullPath.substr(pos + 1, _fullPath.length()));
	char *fname = const_cast<char *>(filename.c_str());
	char cgi[27] = "cgi-bin/download-file.py";
	char *save_path = const_cast<char*>(fullpath.c_str());
	char *fd = const_cast<char *>(filedes.c_str());
	char *arg[2] = {cgi, NULL};
	char *env[4] = {save_path, fname, fd, NULL};
	std::cout << filename << " " << fullpath << " " << filedes << '\n';
	int pid = fork();
	if (pid < 0)
	{
		perror("fork in cgi");
		throw 500;
	}
	if (pid == 0) {
		close(pipeinfd[1]); //close read
		close(pipeoutfd[0]);
		dup2(pipeinfd[0], STDIN_FILENO);
		dup2(pipeoutfd[1], STDOUT_FILENO);
		close(pipeinfd[0]);
		close(pipeoutfd[0]);
		if (execve(arg[0], &arg[0], env) == -1)
		{
			perror("execve in cgi");
			throw 500;
		}
		exit(0);
	}
	close(pipeoutfd[1]); //close write
	close(pipeinfd[0]);
	close(pipeinfd[1]);

	std::vector<char>	data, buf(MAXLINE);
	int	tmpReadLen;
	do
	{
		tmpReadLen = read(pipeoutfd[0], buf.data(), MAXLINE);
		if (tmpReadLen == -1)
		{
			perror("read in cgi");
			throw 500;
		}
		data.insert(data.end(), buf.begin(), buf.begin() + tmpReadLen);
	} while (tmpReadLen == MAXLINE);
	close(pipeoutfd[0]);
	data.push_back('\0');

	waitpid(pid, 0, 0);
	setBodyAndHeaders(data);
}

const std::string	CgiHandler::getCgiFullPath(const std::string &s) const
{
	std::stringstream	ss(getenv("PATH"));
	std::string			dir, absPath;

	if (s != "php-cgi")
		return s;
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

void	CgiHandler::setBodyAndHeaders(const std::vector<char> &buf)
{
	std::stringstream ss(buf.data());
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
	_res->setMessageBody(ss);
}
