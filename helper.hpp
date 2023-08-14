#ifndef HELPER_HPP
# define HELPER_HPP

# include <sstream>
# include <string>
# include <cstring>
# include <stdint.h>

template <class T>
uintptr_t	serialize(T *ptr) {
	return (reinterpret_cast<uintptr_t>(ptr));
}

template <class T>
T	*deserialize(uintptr_t raw) {
	return (reinterpret_cast<T *>(raw));
}

inline std::string	&rtrim(std::string &s, const char *t = " \t\n\r\f\v") {
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

inline std::string	&ltrim(std::string &s, const char *t = " \t\n\r\f\v") {
	s.erase(0, s.find_first_not_of(t));
	return s;
}

inline std::string	&trim(std::string &s, const char *t = " \t\n\r\f\v") {
	return ltrim(rtrim(s, t), t);
}

inline bool isIPv4(const std::string &s) {
	std::istringstream  iss(s);
	int		i1, i2, i3, i4;
	char	d1, d2, d3;

	if (iss >> i1 >> d1 >> i2 >> d2 >> i3 >> d3 >> i4)  {
		if (iss.eof()) {
			if (d1 == '.' && d2 == '.' && d3 == '.') {
				if ((i1 >= 0 && i1 <= 255) && (i2 >= 0 && i2 <= 255)
						&& (i3 >= 0 && i3 <= 255) && (i4 >= 0 && i4 <= 255))
					return true;
			}
		}
	}
	return false;
}

#endif
