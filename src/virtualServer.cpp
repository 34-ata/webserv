#include "../includes/virtualServer.hpp"

VirtualServer::VirtualServer() : host("0.0.0.0"), port(80), clientMaxBodySize(1000000) {}

VirtualServer::VirtualServer(const std::string& h, int p)
    : host(h), port(p), clientMaxBodySize(1000000) {}

VirtualServer::~VirtualServer() {}

void VirtualServer::setServerName(const std::string& name)
{
    serverName = name;
}

void VirtualServer::setRoot(const std::string& path)
{
    root = path;
}

void VirtualServer::setErrorPage(int code, const std::string& path)
{
    errorPages[code] = path;
}

void VirtualServer::setClientMaxBodySize(size_t size)
{
    clientMaxBodySize = size;
}

void VirtualServer::addAllowedMethod(const std::string& method)
{
    allowedMethods.push_back(method);
}

const std::string& VirtualServer::getHost() const
{
    return host;
}

int VirtualServer::getPort() const
{
    return port;
}

const std::string& VirtualServer::getRoot() const
{
    return root;
}

const std::string& VirtualServer::getServerName() const
{
    return serverName;
}

const std::vector<std::string>& VirtualServer::getAllowedMethods() const
{
    return allowedMethods;
}

const std::map<int, std::string>& VirtualServer::getErrorPages() const
{
    return errorPages;
}

size_t VirtualServer::getClientMaxBodySize() const
{
    return clientMaxBodySize;
}

bool VirtualServer::isMethodAllowed(const std::string& method) const
{
    for (size_t i = 0; i < allowedMethods.size(); ++i)
    {
        if (allowedMethods[i] == method)
            return true;
    }
    return false;
}
