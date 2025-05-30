#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string.h>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstdio>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>

enum HttpMethods {
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

class Server {
  public:
	struct Location {
		std::string locUrl;
		std::string rootPath;
		std::string indexFile;
		bool autoIndex;
		std::vector<HttpMethods> allowedMethods;
	};

	struct ServerConfig {
		ServerConfig();

		std::map<int, std::string> m_errorPages;
		std::vector<Location> m_locations;
		std::string m_serverName;
		std::vector<int> m_listens;
		std::string m_clientMaxBodySize;
		bool m_isRunning;
	};

  public:
	Server();
	Server(const ServerConfig& config);
	~Server();

	void Start();
	void Run(const std::vector<Server*>& servers);
	bool handleClient(int clientFd, const std::vector<Server*>& servers);
	void Stop();

	const std::string& getServerName() const { return m_serverName; }
	const std::vector<int>& getListens() const { return m_listens; }
	const std::vector<Location>& getLocations() const { return m_locations; }
	const std::map<int, std::string>& getErrorPages() const { return m_errorPages; }
	const std::string& getClientMaxBodySize() const { return m_clientMaxBodySize; }

  private:
	std::map<int, std::string> m_errorPages;
	std::vector<Location> m_locations;
	std::string m_serverName;
	std::vector<int> m_listens;
	std::string m_clientMaxBodySize;
	bool m_isRunning;
	int serverFd;
	std::vector<int> listenerFds;
	std::vector<struct pollfd> pollFds;
	std::map<int, std::string> clientBuffers;
};

int getPortFromSocket(int fd);

Server* findMatchingServer(const std::string& host, int port, const std::vector<Server*>& servers);

#endif
