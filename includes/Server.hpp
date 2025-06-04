#ifndef SERVER_HPP
#define SERVER_HPP

#include <arpa/inet.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <queue>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "../includes/HttpMethods.hpp"

class Request;

class Server
{
  public:
	struct Location
	{
		std::string locUrl;
		std::string rootPath;
		std::string indexFile;
		bool autoIndex;
		std::vector< HttpMethods > allowedMethods;
	};

	struct ServerConfig
	{
		ServerConfig();

		std::map< int, std::string > errorPages;
		std::vector< Location > locations;
		std::string serverName;
		std::vector< std::pair< std::string, std::string > > listens;
		std::string clientMaxBodySize;
		std::string rootPath;
	};

	Server();
	Server(const ServerConfig& config);
	~Server();

	void Start();

	bool ownsFd(int fd) const;
	void handleEvent(int fd);
	std::vector< struct pollfd >& getPollFds();
	Request *emptyCache(std::stringstream& cache);

	std::vector< std::pair< std::string, std::string > > getListens() const;
	std::string getServerName() const;
	void removePollFd(int fd);

  private:
	std::string m_serverName;
	std::string m_clientMaxBodySize;
	std::map< int, std::string > m_errorPages;
	std::vector< Location > m_locations;
	std::vector< std::pair< std::string, std::string > > m_listens;
	bool m_isRunning;
	std::vector< struct pollfd > pollFds;
	std::vector< int > listenerFds;
	std::queue< Request* > requestQueue;
};

Server* findMatchingServer(const std::string& ip, int port,
						   const std::vector< Server* >& servers);

#endif
