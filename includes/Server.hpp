#ifndef SERVER_HPP
#define SERVER_HPP

#include "HttpMethods.hpp"
#include "Request.hpp"
#include "Response.hpp"
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

	void start();

	void handleEvent(int fd);
	void handleRequestTypes(Request* req);
	void handleGetRequest(Request* req);
	void handlePostRequest(Request* req);
	void handleDeleteRequest(Request* req);
	void handleInvalidRequest();

	bool ownsFd(int fd) const;
	std::vector< struct pollfd >& getPollFds();
	void connectIfNotConnected(int fd);
	void fillCache(std::stringstream& cache, int fd);
	Request* deserializeRequest(std::stringstream& cache);
	void emptyCache(std::stringstream& cache);

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
	std::string m_response;
};

Server* findMatchingServer(const std::string& ip, int port,
						   const std::vector< Server* >& servers);

#endif
