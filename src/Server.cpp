#include "Server.hpp"

#define PORT 8080

Server::Server(const Server::ServerConfig& config)
{
	this->m_serverName		  = config.m_serverName;
	this->m_clientMaxBodySize = config.m_clientMaxBodySize;
	this->m_errorPages		  = config.m_errorPages;
	this->m_locations		  = config.m_locations;
	this->m_listens			  = config.m_listens;
	this->m_isRunning		  = false;
}

Server::~Server()
{
	for (std::vector<struct pollfd>::iterator it = pollFds.begin(); it != pollFds.end(); ++it)
	{
        close(it->fd);
	}
}

Server::ServerConfig::ServerConfig()
{
	m_serverName		  = "";
	m_clientMaxBodySize   = "1M";
	m_errorPages		  = std::map<int, std::string>();
	m_errorPages[404]     = "404.html";
	m_locations		      = std::vector<Server::Location>();
	m_listens			  = std::vector<std::string>(1,"8080");
	m_isRunning		      = false;
}

void Server::addVirtualServer(const VirtualServer& vs) {
    int port = vs.getPort();
    vserverMap[port].push_back(vs);
}

const VirtualServer* Server::findMatchingVirtualServer(const std::string& hostHeader, int port) const {
    std::map<int, std::vector<VirtualServer> >::const_iterator it = vserverMap.find(port);
    if (it == vserverMap.end() || it->second.empty())
        return NULL;

    const std::vector<VirtualServer>& servers = it->second;

    for (size_t i = 0; i < servers.size(); ++i) {
        if (servers[i].getServerName() == hostHeader)
            return &servers[i];
    }

    return &servers[0];
}

static int setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::Start(const std::vector<int>& ports)
{
    for (size_t i = 0; i < ports.size(); ++i)
	{
        int port = ports[i];

        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
		{
            perror("socket");
            std::exit(1);
        }

        int opt = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
            perror("setsockopt");
            std::exit(1);
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		{
            perror("bind");
            std::exit(1);
        }

        if (listen(fd, SOMAXCONN) < 0)
		{
            perror("listen");
            std::exit(1);
        }

        if (setNonBlocking(fd) < 0)
		{
            perror("fcntl");
            std::exit(1);
        }

        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;

        pollFds.push_back(pfd);
        listenerFds.push_back(fd);

        std::cout << "Listening on port " << port << " (fd=" << fd << ")" << std::endl;
    }
}

int getPortFromSocket(int sockfd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    if (getsockname(sockfd, (struct sockaddr*)&addr, &len) == -1)
        return -1;
    return ntohs(addr.sin_port);
}

void Server::Run()
{
    while (true)
	{
        int ret = poll(&pollFds[0], pollFds.size(), -1);
        if (ret < 0)
		{
            perror("poll");
            break;
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
                    if (setNonBlocking(clientFd) < 0)
					{
                        perror("fcntl");
                        close(clientFd);
                        continue;
                    }

                    struct pollfd clientPoll;
                    clientPoll.fd = clientFd;
                    clientPoll.events = POLLIN;
                    pollFds.push_back(clientPoll);

                    std::cout << "New client on fd=" << clientFd << " (from port " << getPortFromSocket(fd) << ")\n";
                }
                else
				{
                    bool done = handleClient(fd);
                    if (done)
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

bool Server::handleClient(int clientFd)
{
    char buffer[1024];
    int bytes = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
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

    int port = 8080;

    const VirtualServer* vs = findMatchingVirtualServer(hostHeader, port);
    if (vs)
        std::cout << "Matched server: " << vs->getServerName() << std::endl;
    else
        std::cout << "No matching virtual server found!" << std::endl;

    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello world\n";
    send(clientFd, response.c_str(), response.size(), 0);

    return true;
}


void Server::Stop()
{
	this->m_isRunning = false;
}
