#ifndef WEBSERVSTRUCT_HPP
# define WEBSERVSTRUCT_HPP

# include <string>
# include <vector>
# include <set>

typedef struct listenData {
    std::string	addr;
    int			port;

    inline bool operator==(const listenData &rhs) const {
        return addr == rhs.addr
            && port == rhs.port;
    }
    inline bool operator<(const listenData &rhs) const {
        if (port == rhs.port)
            return addr < rhs.addr;
        return port < rhs.port;
    }
}	t_listenData;

typedef struct errorPageData {
    std::vector<int>	code;
    std::string			uri;

    inline bool operator==(const errorPageData &rhs) const {
        return code == rhs.code
            && uri == rhs.uri;
    }
    inline bool operator<(const errorPageData &rhs) const {
        return code < rhs.code
            && uri < rhs.uri;
    }
}	t_errorPageData;

typedef struct returnData {
    int	        code;
    std::string	url;

}	t_returnData;

typedef struct locationData {
    std::string				uri;
    std::vector<t_listenData>	listen;
    std::string					root;
    std::vector<std::string>	index;
    std::string					autoIdx;
    unsigned long 				cliMax;
    std::vector<t_errorPageData>	errPage;
    t_returnData            redir;
    std::set<std::string>	limExcept;
    std::string				cgiPass;
    bool                    isRootOvr; // is root overridden

    locationData()
    {
        isRootOvr = true;
    }
    /*locationData()
    {
        uri = "/";
        limExcept.insert("GET");
        cgiPass = "php-cgi";
    }
    locationData(const t_serverData &s): serverData(s)
    {
        uri = "/";
        limExcept.insert("GET");
        cgiPass = "php-cgi";
    }

    inline bool operator==(const locationData& rhs) const {
        return uri == rhs.uri
            && listen == rhs.listen
            && root == rhs.root
            && index == rhs.index
            && autoIdx == rhs.autoIdx
            && cliMax == rhs.cliMax
            && errPage == rhs.errPage
            && limExcept == rhs.limExcept
            && cgiPass == rhs.cgiPass;
    }
    inline bool operator<(const locationData& rhs) const {
        return uri < rhs.uri
            && listen < rhs.listen
            && root < rhs.root
            && index < rhs.index
            && autoIdx < rhs.autoIdx
            && cliMax < rhs.cliMax
            && errPage < rhs.errPage
            && limExcept < rhs.limExcept
            && cgiPass < rhs.cgiPass;
    }*/
}   t_locationData;

typedef struct serverData {
    std::vector<std::string>	name;
    std::vector<t_listenData>	listen;
    std::string					root;
    std::vector<std::string>	index;
    std::string					autoIdx;
    unsigned long				cliMax;
    std::vector<t_errorPageData>	errPage;
    t_returnData            redir;
    std::vector<t_locationData>	location;

    /*serverData()
    {
        std::cerr << "default\n";
        name.push_back("");
        t_listenData    lsn = { "*", 8000 };
        listen.push_back(lsn);
        root = "html";
        index.push_back("index.html");
        autoIdx = "off";
        cliMax = 1000000;
    }
    serverData(const serverData &s)
    {
        std::cerr << "copy\n";
        name = s.name;
        listen = s.listen;
        root = s.root;
        index = s.index;
        autoIdx = s.autoIdx;
        cliMax = s.cliMax;
        errPage = s.errPage;
        location = s.location;
    }*/

    /*inline bool operator==(const serverData& rhs) const {
        return name == rhs.name
            && listen == rhs.listen
            && root == rhs.root
            && index == rhs.index
            && autoIdx == rhs.autoIdx
            && cliMax == rhs.cliMax
            && errPage == rhs.errPage
            && location == rhs.location;
    }*/
    inline bool operator<(const serverData& rhs) const
    {
        if (name == rhs.name)
        {
            return listen < rhs.listen;
            // if (root == rhs.root)
            //     return listen < rhs.listen;
            // return root < rhs.root;
        }
        return name < rhs.name;
    }
}	t_serverData;

#endif
