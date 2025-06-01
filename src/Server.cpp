#include "Server.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <sys/types.h>
#include <vector>

Server::Server(const Server::ServerConfig& config)
{
	this->m_serverName		  = config.serverName;
	this->m_clientMaxBodySize = config.clientMaxBodySize;
	this->m_errorPages		  = config.errorPages;
	this->m_locations		  = config.locations;
	this->m_listens			  = config.listens;
	this->m_isRunning		  = false;
}

Server::ServerConfig::ServerConfig()
{
	serverName		  = "";
	clientMaxBodySize = "1M";
	errorPages[404]	  = "404.html";
}

Server::~Server()
{
	for (std::vector<struct pollfd>::iterator it = pollFds.begin();
		 it != pollFds.end(); ++it)
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

Server* findMatchingServer(const std::string& host, int port,
						   const std::vector<Server*>& servers)
{
	for (size_t i = 0; i < servers.size(); ++i)
	{
		const std::vector<std::string>& listens = servers[i]->getListens();
		for (size_t j = 0; j < listens.size(); ++j)
		{
			if (std::atoi(listens[j].c_str()) == port
				&& servers[i]->getServerName() == host)
				return servers[i];
		}
	}
	for (size_t i = 0; i < servers.size(); ++i)
	{
		const std::vector<std::string>& listens = servers[i]->getListens();
		for (size_t j = 0; j < listens.size(); ++j)
		{
			if (std::atoi(listens[j].c_str()) == port)
				return servers[i];
		}
	}
	return NULL;
}

void Server::Start()
{
	for (size_t i = 0; i < this->m_listens.size(); ++i)
	{
		int port;
		int ipAddr;

		ipAddr = INADDR_ANY;
		port   = std::atoi(this->m_listens[i].c_str());
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
		addr.sin_family		 = AF_INET;
		addr.sin_addr.s_addr = ipAddr;
		addr.sin_port		 = htons(port);

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
		pfd.fd	   = fd;
		pfd.events = POLLIN;

		pollFds.push_back(pfd);
		listenerFds.push_back(fd);
		std::cout << "Listening on port " << port << " (fd=" << fd << ")"
				  << std::endl;
	}
}

bool Server::ownsFd(int fd) const
{
	for (size_t i = 0; i < pollFds.size(); ++i)
	{
		if (pollFds[i].fd == fd)
			return true;
	}
	return false;
}

void Server::handleEvent(int fd)
{
	if (std::find(listenerFds.begin(), listenerFds.end(), fd) != listenerFds.end())
	{
		struct sockaddr_in clientAddr;
		socklen_t len = sizeof(clientAddr);
		int clientFd = accept(fd, (struct sockaddr*)&clientAddr, &len);
		if (clientFd >= 0)
		{
			int flags = fcntl(clientFd, F_GETFL, 0);
			fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
			struct pollfd clientPoll;
			clientPoll.fd = clientFd;
			clientPoll.events = POLLIN;
			pollFds.push_back(clientPoll);
		}
	}
	else
	{
		char buffer[1024];
		int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
		if (bytes > 0)
		{
			buffer[bytes] = '\0';
			std::cout << "Recv: " << std::endl << buffer << std::endl;
		}
		else
		{
			close(fd);
		}
	}
	
}

std::vector<struct pollfd>& Server::getPollFds()
{
	return pollFds;
}

std::vector<std::string> Server::getListens() const
{
	return m_listens;
}

std::string Server::getServerName() const
{
	return m_serverName;
}
