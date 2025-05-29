#ifndef VIRTUALSERVER_HPP
#define VIRTUALSERVER_HPP

#include <string>
#include <vector>
#include <map>

class VirtualServer
{
    private:
        std::string host;
        int port;
        std::string serverName;
        std::string root;
        std::map<int, std::string> errorPages;
        size_t clientMaxBodySize;
        std::vector<std::string> allowedMethods;

    public:
        VirtualServer();
        VirtualServer(const std::string& host, int port);
        ~VirtualServer();

        void setServerName(const std::string& name);
        void setRoot(const std::string& path);
        void setErrorPage(int code, const std::string& path);
        void setClientMaxBodySize(size_t size);
        void addAllowedMethod(const std::string& method);

        const std::string& getHost() const;
        int getPort() const;
        const std::string& getRoot() const;
        const std::string& getServerName() const;
        const std::vector<std::string>& getAllowedMethods() const;
        const std::map<int, std::string>& getErrorPages() const;
        size_t getClientMaxBodySize() const;

        bool isMethodAllowed(const std::string& method) const;
};

#endif
