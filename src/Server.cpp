#include "Server.hpp"

Server::Server(const Server::ServerConfig& config)
{
    this->m_serverName = config.m_serverName;
    this->m_clientMaxBodySize = config.m_clientMaxBodySize;
    this->m_errorPages = config.m_errorPages;
    this->m_locations = config.m_locations;
    this->m_listens = config.m_listens;
    this->m_isRunning = false;
}

Server::ServerConfig::ServerConfig()
{
    m_serverName = "";
    m_clientMaxBodySize = "1M";
    m_errorPages[404] = "404.html";
    m_isRunning = false;
}

Server::~Server()
{
    for (std::vector<struct pollfd>::iterator it = pollFds.begin(); it != pollFds.end(); ++it)
    {
        close(it->fd);
    }
}
int getPortFromSocket(int fd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    if (getsockname(fd, (struct sockaddr*)&addr, &len) == -1)
        return -1;
    return ntohs(addr.sin_port);
}

Server* findMatchingServer(const std::string& host, int port, const std::vector<Server*>& servers)
{
    for (size_t i = 0; i < servers.size(); ++i)
    {
        const std::vector<int>& listens = servers[i]->getListens();
        for (size_t j = 0; j < listens.size(); ++j)
        {
            if (listens[j] == port && servers[i]->getServerName() == host)
                return servers[i];
        }
    }
    for (size_t i = 0; i < servers.size(); ++i)
    {
        const std::vector<int>& listens = servers[i]->getListens();
        for (size_t j = 0; j < listens.size(); ++j)
        {
            if (listens[j] == port)
                return servers[i];
        }
    }
    return NULL;
}

void Server::Start()
{
    for (size_t i = 0; i < this->m_listens.size(); ++i)
    {
        int port = this->m_listens[i];
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
        {
            perror("socket");
            continue;
        }
        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        {
            perror("bind");
            close(fd);
            continue;
        }
        listen(fd, SOMAXCONN);

        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;

        pollFds.push_back(pfd);
        listenerFds.push_back(fd);
        std::cout << "Listening on port " << port << " (fd=" << fd << ")" << std::endl;

    }
}

void Server::Run(const std::vector<Server*>& servers)
{
    while (true)
    {
        if (poll(&pollFds[0], pollFds.size(), -1) < 0)
        {
            perror("poll");
            return;
        }

        for (size_t i = 0; i < pollFds.size(); ++i)
        {
            int fd = pollFds[i].fd;
            if (pollFds[i].revents & POLLIN)
            {
                if (std::find(listenerFds.begin(), listenerFds.end(), fd) != listenerFds.end())
                {
                    struct sockaddr_in clientAddr;
                    socklen_t len = sizeof(clientAddr);
                    int clientFd = accept(fd, (struct sockaddr*)&clientAddr, &len);
                    if (clientFd < 0)
                    {
                        perror("accept");
                        continue;
                    }
                    int flags = fcntl(clientFd, F_GETFL, 0);
                    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

                    struct pollfd clientPoll;
                    clientPoll.fd = clientFd;
                    clientPoll.events = POLLIN;
                    pollFds.push_back(clientPoll);
                }
                else
                {
                    if (handleClient(fd, servers))
                    {
                        close(fd);
                        pollFds.erase(pollFds.begin() + i);
                        clientBuffers.erase(fd);
                        --i;
                    }
                }
            }
        }
    }
}

bool Server::handleClient(int clientFd, const std::vector<Server*>& servers)
{
    char buffer[1024];
    int bytes = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    std::cout << "recv: " << std::endl << buffer << std::endl;
    if (bytes <= 0)
        return true;

    buffer[bytes] = '\0';
    clientBuffers[clientFd] += buffer;

    std::string& request = clientBuffers[clientFd];
    if (request.find("\r\n\r\n") == std::string::npos)
        return false;

    std::string hostHeader;
    size_t hostPos = request.find("Host:");
    if (hostPos != std::string::npos)
    {
        size_t end = request.find("\r\n", hostPos);
        if (end != std::string::npos)
            hostHeader = request.substr(hostPos + 5, end - hostPos - 5);
        while (!hostHeader.empty() && (hostHeader[0] == ' ' || hostHeader[0] == '\t'))
            hostHeader.erase(0, 1);
    }

    int port = getPortFromSocket(clientFd);
    Server* matched = findMatchingServer(hostHeader, port, servers);
    if (matched)
    {
        std::cout << "Matched Server: " << matched->getServerName() << " on port " << port << std::endl;
    }
    else
    {
        std::cerr << "[ERROR] No matching server found for host: " << hostHeader << " port: " << port << std::endl;
        return true;
    }

    std::cout << "Sending response to fd: " << clientFd << std::endl;
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello world\n";
    send(clientFd, response.c_str(), response.size(), 0);
    return true;
}
