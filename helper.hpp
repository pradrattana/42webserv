#ifndef HELPER_HPP
# define HELPER_HPP

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

#endif
