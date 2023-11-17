#include "Response.hpp"

void	Response::directoryListing() {
	std::stringstream	ss;
	std::stringstream	dirList;
	DIR				*dir;
	struct dirent	*ent;

	ss << "<!DOCYTPE html><html>"
		<< "<head>";
	printStyle(ss);
	ss << "</head>"
		<< "<body>"
		<< "<h1>Index of " << _request.getUri() << "</h1>"
		<< "<p><a href=\"..\">[parent directory]</a></p>";
	if ((dir = opendir(_fullPath.c_str()))) {
		while ((ent = readdir(dir)))
			dirList << ent->d_name << "\n";
		closedir(dir);
	}
	printTable(ss, dirList);
	ss << "</body>"
		<< "</html>";

	setMessageBody(ss);
	_headers["Content-Type"] = "text/html";
}

void	Response::printFileSize(std::stringstream &ss, const std::string &name) {
	struct stat statbuf;

	if (stat(name.c_str(), &statbuf) == 0) {
		if (statbuf.st_size >= 1000) {
			ss << statbuf.st_size / 1000 << " kB";
		} else {
			ss << statbuf.st_size << " B";
		}
	}
}

void	Response::printDateModified(std::stringstream &ss, const std::string &name) {
	struct stat statbuf;

	if (stat(name.c_str(), &statbuf) == 0) {
		char mbstr[100];
		if (std::strftime(mbstr, sizeof(mbstr),
						  "%D %r", std::localtime(&statbuf.st_mtime)))
			ss << mbstr;
	}
}

void	Response::printStyle(std::stringstream &ss) {
	ss << "<style>"
		<< "td {padding: 0 2em}"
		<< "</style>";
}

void	Response::printTable(std::stringstream &ss, std::stringstream &src) {
	std::string	path(_fullPath), dname;

	ss << "<table>"
	   << "<thead><tr><th>Name</th><th>Size</th><th>Date Modified</th></tr></thead>"
	   << "<tbody>";

	while (std::getline(src, dname)) {
		if (dname == "." || dname == "..")
			continue ;
		path.replace(path.rfind('/') + 1, std::string::npos, dname);
		ss << "<tr>";
		ss << "<td><a href=\"" << dname << "\" download>" << dname << "</a></td>";
		ss << "<td class=\"\">";
		printFileSize(ss, path);
		ss << "</td>";
		ss << "<td>";
		printDateModified(ss, path);
		ss << "</td>";
		ss << "</tr>";
	}
	
	ss << "</tobody>"
	   << "</table>";
}
