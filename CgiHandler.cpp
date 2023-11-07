#include "CgiHandler.hpp"

CgiHandler::CgiHandler() {

}

CgiHandler::CgiHandler(Response *res) : _res(res) {

}

const std::string	CgiHandler::getCgiFullPath(const std::string &s) const {
	std::stringstream	ss(getenv("PATH"));
	std::string			dir, absPath;

	while (std::getline(ss, dir, ':')) {
		absPath = dir;
		if (s[0] != '/')
			absPath.append("/");
		absPath.append(s);

		if (access(absPath.c_str(), F_OK) == 0)
			return absPath;
	}
	return "";
}

void	CgiHandler::setBodyAndHeaders(const std::string buf) {
	std::stringstream ss(buf);
	std::string line, key, val;
	
	_res->setCode(200);
	while (std::getline(ss, line)) {
		if (line.empty() || line == "\r")
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
	try {
		_res->getHeaders().at("Content-Length");
	} catch (const std::exception &e) {
		_res->setContentLength();
	}
}


void	CgiHandler::executeCgi() {
	int		pp[4];
	int		pid, pid2;

	pipe(pp);
	if ((pid = fork()) < 0) {
		exit(1);
	} else if (pid == 0) {
		close(pp[0]);
		write(pp[1], _res->getRequest().getMessageBody(),
			_res->getRequest().getMessageBodyLen());
		close(pp[1]);
		exit(0);
	}
	close(pp[1]);
	waitpid(pid, 0, 0);

	pipe(pp + 2);
	if ((pid2 = fork()) < 0) {
		exit(1);
	} else if (pid2 == 0) {
		close(pp[2]);
		dup2(pp[3], 1);
		dup2(pp[0], 0);

		char	**env = NULL;
		_res->toEnv(env);
		char	*args[] = { (char *)"php-cgi", NULL };

		if (execve(getCgiFullPath("php-cgi").c_str(), args, env) < 0)
			perror("execve");
		close(pp[0]);
		close(pp[3]);
		exit(0);
	}
	close(pp[0]);
	close(pp[3]);
	waitpid(pid2, 0, 0);

	char buf[1024];
	int bytes_read = read(pp[2], buf, sizeof(buf));

	close(pp[2]);
	if (bytes_read == -1) {
		perror("read in cgi");
	}
	buf[bytes_read] = 0;

	// std::cout << "\nCGI RES\n" << buf << "\n";
	setBodyAndHeaders(buf);
}

/*void	CgiHandler::exportEnv() {
	std::string	src(_res->toEnv());
	std::string	line;
	std::string::size_type	nlPos, asgPos;

	while ((nlPos = src.find('\n')) != std::string::npos) {
		line = src.substr(0, nlPos);
		asgPos = line.find('=');
		setenv(line.substr(0, asgPos).c_str(),
			line.substr(asgPos + 1).c_str(), 1);
		// std::cerr << line;
		src = src.substr(nlPos + 1);
	}
}*/
