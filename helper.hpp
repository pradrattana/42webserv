#ifndef HELPER_HPP
# define HELPER_HPP

# include <sstream>
# include <string>
# include <cstring>
# include <stdint.h>

template <class T>
uintptr_t	serialize(T *ptr)
{
	return (reinterpret_cast<uintptr_t>(ptr));
}

template <class T>
T	*deserialize(uintptr_t raw)
{
	return (reinterpret_cast<T *>(raw));
}

inline std::string	&rtrim(std::string &s, const char *t = " \t\n\r\f\v")
{
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

inline std::string	&ltrim(std::string &s, const char *t = " \t\n\r\f\v")
{
	s.erase(0, s.find_first_not_of(t));
	return s;
}

inline std::string	&trim(std::string &s, const char *t = " \t\n\r\f\v")
{
	return ltrim(rtrim(s, t), t);
}

inline bool isIPv4(const std::string &s)
{
	std::string	trimS(s), tmp;
	std::istringstream  iss(trim(trimS));
	int		i1, i2, i3, i4;
	char	d1, d2, d3;

	if ((iss >> tmp).eof())
	{
		iss.seekg(0);
		if ((iss >> i1 >> d1 >> i2 >> d2 >> i3 >> d3 >> i4).eof())
		{
			if (d1 == '.' && d2 == '.' && d3 == '.')
			{
				if ((i1 >= 0 && i1 <= 255) && (i2 >= 0 && i2 <= 255)
						&& (i3 >= 0 && i3 <= 255) && (i4 >= 0 && i4 <= 255))
					return true;
			}
		}
	}
	return false;
}

inline bool	isPathValid(const std::string &s)
{
	std::string	path(s);

	if (trim(path).empty())
		return false;

	for (std::string::size_type i = 0; i < path.length(); i++)
	{
		if (!std::isalnum(path[i]) && path[i] != '.' && path[i] != '-'
				&& path[i] != '_' && path[i] != '/')
			return false;
	}
	return true;
}

inline bool	isURIHostValid(const std::string &s)
{
	std::string	uri(s);

	if (trim(uri).empty())
		return false;

	for (std::string::size_type i = 0; i < uri.length(); i++)
	{
		if (!isalnum(uri[i]) && uri[i] != '.' && uri[i] != '-')
			return false;
		else if (uri[i] == '.' && i + 1 < uri.length())
			if (uri[i + 1] == '.')
				return false;
	}
	return true;
}

inline bool	isURIPathValid(const std::string &s)
{
	std::string	uri(s);

	if (trim(uri).empty())
		return false;

	for (std::string::size_type i = 0; i < uri.length(); i++)
	{
		if (!isalnum(uri[i]) && uri[i] != '.' && uri[i] != '-'
				&& uri[i] != '_' && uri[i] != '~'
				&& uri[i] != '%' && uri[i] != '/')
			return false;
		else if (uri[i] == '%')
		{
			if (!(isxdigit(uri[i + 1]) && isxdigit(uri[i + 2])))
				return false;
			i += 2;
		}
		// else if (uri[i] == '/' && i + 1 < uri.length())
		// {
		// 	if (uri[i + 1] == '/')
		// 		return false;
		// }
	}
	return true;
}

inline bool	isURIQueryValid(const std::string &s)
{
	std::string	uri(s);

	if (trim(uri).empty())
		return false;

	for (std::string::size_type i = 0; i < uri.length(); i++)
	{
		if (!isalnum(uri[i]) && uri[i] != '.' && uri[i] != '-'
				&& uri[i] != '_' && uri[i] != '~'
				&& uri[i] != '%' && uri[i] != '&'
				&& uri[i] != '=')
			return false;
		else if (uri[i] == '%')
		{
			if (!(isxdigit(uri[i + 1]) && isxdigit(uri[i + 2])))
				return false;
			i += 2;
		}
		else if (uri[i] == '.' && i + 1 < uri.length())
		{
			if (uri[i + 1] == '.')
				return false;
		}
		else if ((uri[i] == '&' || uri[i] == '=') && i + 1 < uri.length())
		{
			if (uri[i + 1] == '&' || uri[i + 1] == '=')
				return false;
		}
	}
	return true;
}

inline bool	isURIValid(const std::string &s)
{
	std::string	uri(s);

	if (trim(uri).empty())
		return false;

	std::string::size_type	hostPos = uri.find("://") + 3;
	if (hostPos == std::string::npos)
		hostPos = uri.length();
	std::string::size_type	pathComponentPos = uri.find('/', hostPos);
	if (pathComponentPos == std::string::npos)
		pathComponentPos = uri.length();
	std::string::size_type	queryPos = uri.find('?', pathComponentPos);
	if (queryPos == std::string::npos)
		queryPos = uri.length();

	// scheme
	for (std::string::size_type i = 0; i < hostPos; i++)
	{
		if (!isalnum(uri[i]) && std::string(":/").find(uri[i]) == std::string::npos)
			return false;
	}
	// host
	if (!isURIHostValid(uri.substr(hostPos, pathComponentPos - hostPos)))
		return false;
	// path com
	if (pathComponentPos < uri.length())
		if (!isURIPathValid(uri.substr(pathComponentPos, queryPos - pathComponentPos)))
			return false;
	// query
	if (queryPos < uri.length())
		if (!isURIQueryValid(uri.substr(queryPos + 1)))
			return false;

	return true;
}

inline std::string &formatPath(std::string &dst)
{
	if (*dst.begin() != '/')
		dst.insert(dst.begin(), '/');
	if (dst.length() > 1)
		if (*(dst.end() - 1) == '/')
			dst.erase(dst.end() - 1);
	return dst;
}

inline std::string	&eraseIfFound(std::string &dst, const std::string &target)
{
	std::string::size_type	pos = dst.find(target);
	if (pos != std::string::npos)
		dst.erase(pos);
	return dst;
}

#endif
