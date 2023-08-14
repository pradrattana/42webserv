#ifndef WEBSERVSTRUCT_HPP
# define WEBSERVSTRUCT_HPP

# include <string>
# include <vector>
# include <set>

typedef struct listenData {
    std::string	addr;
    int			port;

    inline bool operator==(const listenData& rhs) const {
        return addr == rhs.addr
            && port == rhs.port;
    }
    inline bool operator<(const listenData& rhs) const {
        if (port == rhs.port)
            return addr < rhs.addr;
        return port < rhs.port;
    }
}	t_listenData;

typedef struct errorPageData {
    std::vector<int>	code;
    std::string			uri;

    inline bool operator==(const errorPageData& rhs) const {
        return code == rhs.code
            && uri == rhs.uri;
    }
    inline bool operator<(const errorPageData& rhs) const {
        return code < rhs.code
            && uri < rhs.uri;
    }
}	t_errorPageData;

typedef struct locationData {
    std::string					uri;

    std::vector<t_listenData>	listen;
    std::string					root;
    std::vector<std::string>	index;
    std::string					autoIdx;
    std::string 				cliMax;
    std::vector<t_errorPageData>	errPage;

    std::set<std::string>	limExcept;
    std::string					cgiPass;

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
    }
}	t_locationData;

typedef struct serverData {
    std::vector<std::string>	name;

    std::vector<t_listenData>	listen;
    std::string					root;
    std::vector<std::string>	index;
    std::string					autoIdx;
    std::string					cliMax;
    std::vector<t_errorPageData>	errPage;

    std::vector<t_locationData>	location;

    inline bool operator==(const serverData& rhs) const {
        return name == rhs.name
            && listen == rhs.listen
            && root == rhs.root
            && index == rhs.index
            && autoIdx == rhs.autoIdx
            && cliMax == rhs.cliMax
            && errPage == rhs.errPage
            && location == rhs.location;
    }
    inline bool operator<(const serverData& rhs) const {
        if (name == rhs.name) {
            if (root == rhs.root)
                return listen < rhs.listen;
            return root < rhs.root;
        }
        return name < rhs.name;
        // return name < rhs.name
        //     && listen < rhs.listen
        //     && root < rhs.root
        //     && index < rhs.index
        //     && autoIdx < rhs.autoIdx
        //     && cliMax < rhs.cliMax 
        //     && errPage < rhs.errPage
        //     && location < rhs.location;
    }
}	t_serverData;

#endif
