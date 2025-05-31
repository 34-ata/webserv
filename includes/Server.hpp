#ifndef SERVER_HPP
#define SERVER_HPP

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

enum HttpMethods
{
	GET,
	POST,
	HEAD,
	PUT,
	DELETE,
	CONNECT,
	OPTIONS,
	TRACE,
	PATCH
};

class Server
{
  public:
	struct Location
	{
		std::string locUrl;
		std::string rootPath;
		std::string indexFile;
		bool autoIndex;
		std::vector<HttpMethods> allowedMethods;
	};

	struct ServerConfig
	{
		ServerConfig();

		std::map<int, std::string> errorPages;
		std::vector<Location> locations;
		std::string serverName;
		std::vector<std::string> listens;
		std::string clientMaxBodySize;
		std::string rootPath;
	};

  public:
	Server();
	Server(const ServerConfig& config);
	~Server();

	void Start();
	void Run(const std::vector<Server*>& servers);
	bool handleClient(int clientFd, const std::vector<Server*>& servers);
	void Stop();

	const std::string& getServerName() const;
	const std::vector<std::string>& getListens() const;
	const std::vector<Location>& getLocations() const;
	const std::map<int, std::string>& getErrorPages() const;
	const std::string& getClientMaxBodySize() const;
	const std::string& getRootPath() const;

  private:
	std::map<int, std::string> m_errorPages;
	std::vector<Location> m_locations;
	std::string m_serverName;
	std::vector<std::string> m_listens;
	std::string m_clientMaxBodySize;
	std::string m_rootPath;
	bool m_isRunning;
	int serverFd;
	std::vector<int> listenerFds;
	std::vector<struct pollfd> pollFds;
	std::map<int, std::string> clientBuffers;
};

int getPortFromSocket(int fd);

Server* findMatchingServer(const std::string& host, int port,
						   const std::vector<Server*>& servers);

#endif
