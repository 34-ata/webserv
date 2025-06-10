#include "Server.hpp"
#include "../includes/Response.hpp"
#include "HttpMethods.hpp"
#include "HttpVersion.hpp"
#include "Log.hpp"
#include "Request.hpp"
#include "ResponseCodes.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
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
	for (std::vector< struct pollfd >::iterator it = pollFds.begin();
		 it != pollFds.end(); ++it)
	{
		close(it->fd);
	}
	while (0 < requestQueue.size())
	{
		delete requestQueue.front();
		requestQueue.pop();
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

Server* findMatchingServer(const std::string& ip, int port,
						   const std::vector< Server* >& servers)
{
	for (size_t i = 0; i < servers.size(); ++i)
	{
		const std::vector< std::pair< std::string, std::string > >& listens =
			servers[i]->getListens();
		for (size_t j = 0; j < listens.size(); ++j)
		{
			if (listens[j].first == ip
				&& std::atoi(listens[j].second.c_str()) == port)
				return servers[i];
		}
	}

	for (size_t i = 0; i < servers.size(); ++i)
	{
		const std::vector< std::pair< std::string, std::string > >& listens =
			servers[i]->getListens();
		for (size_t j = 0; j < listens.size(); ++j)
		{
			if (std::atoi(listens[j].second.c_str()) == port)
				return servers[i];
		}
	}

	return servers[0];
}

void Server::start()
{
	for (size_t i = 0; i < this->m_listens.size(); ++i)
	{
		std::string ip		= this->m_listens[i].first;
		std::string portStr = this->m_listens[i].second;
		int port			= std::atoi(portStr.c_str());

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
		addr.sin_port	= htons(port);

		if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0)
		{
			std::cerr << "Invalid IP address: " << ip << std::endl;
			close(fd);
			continue;
		}

		if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		{
			perror("bind");
			close(fd);
			continue;
		}

		if (listen(fd, SOMAXCONN) < 0)
		{
			perror("listen");
			close(fd);
			continue;
		}

		fcntl(fd, F_SETFL, O_NONBLOCK);

		struct pollfd pfd;
		pfd.fd	   = fd;
		pfd.events = POLLIN;

		pollFds.push_back(pfd);
		listenerFds.push_back(fd);

		std::cout << "Listening on " << ip << ":" << port << " (fd=" << fd
				  << ")" << std::endl;
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

void Server::removePollFd(int fd)
{
	for (size_t i = 0; i < pollFds.size(); ++i)
	{
		if (pollFds[i].fd == fd)
		{
			pollFds.erase(pollFds.begin() + i);
			break;
		}
	}
}

void Server::connectIfNotConnected(int fd)
{
	if (std::find(listenerFds.begin(), listenerFds.end(), fd)
		!= listenerFds.end())
	{
		LOG("Added Socket to listenerFds.");
		struct sockaddr_in clientAddr;
		socklen_t len = sizeof(clientAddr);
		int clientFd  = accept(fd, (struct sockaddr*)&clientAddr, &len);
		if (clientFd >= 0)
		{
			fcntl(fd, F_SETFL, O_NONBLOCK);
			struct pollfd clientPoll;
			clientPoll.fd	  = clientFd;
			clientPoll.events = POLLIN;
			pollFds.push_back(clientPoll);
		}
	}
}

void Server::fillCache(int fd)
{
	char buffer[1024];
	memset(&buffer, 0, 1024);
	int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes > 0)
	{
		buffer[bytes] = 0;
		cache << buffer;
	}
}

Request* Server::deserializeRequest(Request* req)
{
	std::string res;
	char bodyChar;
	for (size_t i = 0; i < req->getBodyLenght(); i++)
	{
		cache.get(bodyChar);
		res.append(1, bodyChar);
	}
	req->fillRequest(res);
	return req;
}

void Server::getHeader(Request* req)
{
	bool headerFound = false;
	std::string res;
	while (std::getline(cache, res))
	{
		res.append("\n");
		req->fillRequest(res);
		if (req->getData().find("\r\n\r\n") != std::string::npos)
		{
			headerFound = true;
			break;
		}
	}
	if (!headerFound)
		req->setBadRequest();
	req->checkIntegrity();
	res.clear();

}

void Server::handleEvent(int fd)
{
	cache.str(std::string());
	static Request* req = NULL;
	connectIfNotConnected(fd);
	fillCache(fd);
	if (cache.str().find("\r\n\r\n") != std::string::npos || req != NULL)
	{
		if (req == NULL)
		{
			req = new Request();
			getHeader(req);
		}
		deserializeRequest(req);
		if (req->getBodyLenght() == req->getBody().size())
		{
			handleRequestTypes(req);
			send(fd, m_response.c_str(), m_response.size(), 0);
			LOG("FD: " << fd);
			close(fd);
			delete req;
			req = NULL;
			removePollFd(fd);
		}
	}
}

void Server::handleGetRequest(Request* req)
{
	Response response;
	std::string filePath;
	LOG("Started Handling GET Request");
	char cwd[200];
	getcwd(cwd, 200);
	filePath = std::string(cwd) + req->getPath();
	if (access(filePath.c_str(), F_OK) != 0)
	{
		filePath   = "./www/404.html";
		m_response = response.status(NOT_FOUND)
						 .htppVersion(HTTP_VERSION)
						 .body(getFileContent(filePath))
						 .header("Content-Type", getContentType(filePath))
						 .build();
		return;
	}
	m_response = response.status(OK)
					 .htppVersion(HTTP_VERSION)
					 .body(getFileContent(filePath))
					 .header("Content-Type", getContentType(filePath))
					 .build();
}

void Server::handlePostRequest(Request* req)
{
	Response response;
	std::string filePath;
	filePath = req->getPath();

	int file = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (file == -1)
	{
		filePath = "./www/500.html";
		m_response =
			response.htppVersion(HTTP_VERSION).status(INT_SERV_ERR).build();
		return;
	}
	write(file, req->getBody().c_str(), req->getBody().size());
	close(file);
	m_response = response.status(CREATED)
					 .htppVersion(HTTP_VERSION)
					 .header("Content-Type", "text/plain")
					 .body("File uploaded successfully.\n")
					 .build();
}

void Server::handleDeleteRequest(Request* req)
{
	Response response;
	std::string filePath;
	filePath = req->getPath();
	int res	 = std::remove(filePath.c_str());
	if (res == 0)
		m_response = response.htppVersion(HTTP_VERSION).status(OK).build();
	else
		m_response =
			response.htppVersion(HTTP_VERSION).status(NOT_FOUND).build();
}

void Server::handleInvalidRequest()
{
	Response response;
	std::string filePath;
	filePath   = "./www/405.html";
	m_response = response.htppVersion(HTTP_VERSION)
					 .status(METH_NOT_ALLOW)
					 .header("Content-Type", getContentType(filePath))
					 .body(getFileContent(filePath))
					 .build();
}

void Server::handleRequestTypes(Request* req)
{
	if (req->getBadRequest())
		m_response =
			Response().htppVersion(HTTP_VERSION).status(BAD_REQ).build();
	switch (req->getMethod())
	{
	case GET:
		handleGetRequest(req);
		break;
	case POST:
		handlePostRequest(req);
		break;
	case DELETE:
		handleDeleteRequest(req);
		break;
	case INVALID:
		handleInvalidRequest();
	}
}

std::string Server::getServerName() const { return m_serverName; }

std::vector< std::pair< std::string, std::string > > Server::getListens() const
{
	return m_listens;
}

std::vector< struct pollfd >& Server::getPollFds() { return pollFds; }
