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
		{
			perror("execve in cgi");
			throw 500;
		}

		for (int i = 0; env[i] != NULL; i++)
			delete env[i];
		delete[] env;
		fclose(fp);
		close(pp[1]);
		exit(0);
	}
	close(pp[1]);
	waitpid(pid, 0, 0);

	std::vector<char>	buf(1024);
	int	bytesRead = read(pp[0], buf.data(), 1024);

	close(pp[0]);
	if (bytesRead == -1)
	{
		perror("read in cgi");
		throw 500;
	}
	buf.resize(bytesRead);

	setBodyAndHeaders(buf);
}

void CgiHandler::executeCgiDownload(std::string _fullPath, std::string &_response){
	int	pipeinfd[2];
	int pipeoutfd[2];

	if (pipe(pipeinfd) == -1 || pipe(pipeoutfd) == -1) {
		perror("error pipe");
		exit(EXIT_FAILURE);
	}
	std::string filename = "filename=";
	std::string fullpath = "fullpath=" ;
	std::string filedes = "fd=";
	std::ostringstream num;
	num << pipeoutfd[1];
	filedes.append(num.str());
	fullpath.append(&_fullPath.c_str()[1], _fullPath.length());
	size_t pos = _fullPath.find_last_of('/');
	if (pos != std::string::npos)
		filename.append(_fullPath.substr(pos + 1, _fullPath.length()));
	char *fname = const_cast<char *>(filename.c_str());
	char cgi[27] = "cgi-bin/download-file.py";
	char *save_path = const_cast<char*>(fullpath.c_str());
	char *fd = const_cast<char *>(filedes.c_str());
	char *arg[2] = {cgi, NULL};
	char *env[4] = {save_path, fname, fd, NULL};
	int pid = fork();
	if (pid < 0)
		perror("fork failed : ");
	if (pid == 0) {
		close(pipeinfd[1]); //close read
		close(pipeoutfd[0]);
		dup2(pipeinfd[0], STDIN_FILENO);
		dup2(pipeoutfd[1], STDOUT_FILENO);
		close(pipeinfd[0]);
		close(pipeoutfd[0]);
		if (execve(arg[0], &arg[0], env) == -1){
			perror("");
			std::cout << "execve failed\n";
		}
		exit(0);
	}
	close(pipeoutfd[1]); //close write
	close(pipeinfd[0]);
	close(pipeinfd[1]);
	char	buf[MAXLINE];
	int status;
	std::string temp  = "";
	int		bytes_read = 1;
	while (bytes_read > 0) {
		bytes_read = read(pipeoutfd[0], buf, MAXLINE);
		if (bytes_read < 0){
			perror("read failed : ");
			break;
		}
		temp.append(buf, bytes_read);
	}
	close(pipeoutfd[0]);
	waitpid(pid, &status, 0);
	_response = temp;
}

void CgiHandler::executeCgiDelete(RequestParser &_request, std::string &res){
	int	pipefd[2];

	if (pipe(pipefd) == -1) {
		perror("error pipe");
		exit(EXIT_FAILURE);
	}
	std::string filename = "filename=";
	std::string fullpath = "fullpath=" ;
	fullpath.append(_request.getUri().c_str(), _request.getUri().length());
	size_t pos = _request.getUri().find_last_of('/');
	if (pos != std::string::npos)
		filename.append(_request.getUri().substr(pos + 1, _request.getUri().length()));
	char *fname = const_cast<char *>(filename.c_str());
	char cgi[27] = "cgi-bin/delete-file.perl";
	char *save_path = const_cast<char*>(fullpath.c_str());
	char *arg[2] = {cgi, NULL};
	char *env[3] = {save_path, fname, NULL};
	int pid = fork();
	if (pid < 0)
		perror("fork failed : ");
	if (pid == 0) {
		close(pipefd[0]); //close read
		dup2(pipefd[1], STDOUT_FILENO);
		if (execve(arg[0], &arg[0], env) == -1){
			perror("");
			std::cout << "execve failed\n";
		}
		exit(0);
	}
	close(pipefd[1]);
	char	buf[MAXLINE];
	int status;
	int		bytes_read = 1;
	while (bytes_read > 0) {
		bytes_read = read(pipefd[0], buf, MAXLINE);
		if (bytes_read < 0){
			perror("read failed : ");
			break;
		}
		res.append(buf, bytes_read);
	}
	close(pipefd[0]);
	waitpid(pid, &status, 0);
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
