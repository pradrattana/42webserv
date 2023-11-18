#ifndef WEBSERVSTRUCT_HPP
# define WEBSERVSTRUCT_HPP

# include <string>
# include <vector>
# include <set>

typedef struct listenData
{
	std::string	addr;
	int			port;

	inline bool operator==(const listenData &rhs) const
	{
		if (addr == rhs.addr)
			return port == rhs.port;
		return false;
	}
	inline bool operator<(const listenData &rhs) const
	{
		if (addr == rhs.addr)
			return port < rhs.port;
		return addr < rhs.addr;
	}

}	t_listenData;

typedef struct errorPageData
{
	std::vector<int>	code;
	std::string			uri;
}	t_errorPageData;

typedef struct returnData
{
	int			code;
	std::string	url;
}	t_returnData;

typedef struct locationData
{
	std::string		uri;
	std::vector<t_listenData>	listen;
	std::string		root;
	std::vector<std::string>	index;
	std::string		autoIdx;
	unsigned long 	cliMax;
	std::vector<t_errorPageData>	errPage;
	t_returnData	redir;
	std::set<std::string>		limExcept;
	std::string		cgiPass;
	bool			isRootOvr; // is root overridden

	locationData()
	{
		cliMax = 0;
		isRootOvr = true;
	}

}	t_locationData;

typedef struct serverData
{
	std::vector<std::string>	name;
	std::vector<t_listenData>	listen;
	std::string		root;
	std::vector<std::string>	index;
	std::string		autoIdx;
	unsigned long	cliMax;
	std::vector<t_errorPageData>	errPage;
	t_returnData	redir;
	std::vector<t_locationData>	location;

    serverData()
    {
        cliMax = 0;
    }

    inline bool operator<(const serverData& rhs) const
    {
        if (name == rhs.name)
        {
            return listen < rhs.listen;
        }
        return name < rhs.name;
    }

}	t_serverData;

#endif
