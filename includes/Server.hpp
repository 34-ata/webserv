#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <arpa/inet.h>

class Request;

enum HttpMethods
{
	INVALID = -1,
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
		std::vector<std::pair<std::string, std::string> > listens;
		std::string clientMaxBodySize;
		std::string rootPath;
	};

	Server();
	Server(const ServerConfig& config);
	~Server();

	void Start();

	bool ownsFd(int fd) const;
	void handleEvent(int fd);
	std::vector<struct pollfd>& getPollFds();
	Request createRequest(const std::string& cache, std::size_t bodyIndex);

	std::vector<std::pair<std::string, std::string> > getListens() const;
	std::string getServerName() const;
	void removePollFd(int fd);

  private:
	std::string m_serverName;
	std::string m_clientMaxBodySize;
	std::map<int, std::string> m_errorPages;
	std::vector<Location> m_locations;
	std::vector<std::pair<std::string, std::string> > m_listens;
	bool m_isRunning;
	std::vector<struct pollfd> pollFds;
	std::vector<int> listenerFds;
};

Server* findMatchingServer(const std::string& ip, int port, const std::vector<Server*>& servers);

#endif
