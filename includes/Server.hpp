#ifndef SERVER_HPP
#define SERVER_HPP

#include "HttpMethods.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ResponseCodes.hpp"
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
		Location();
		std::string locUrl;
		std::string rootPath;
		std::string indexPath;
		bool autoIndex;
		std::vector< HttpMethods > allowedMethods;

		bool hasRedirect;
		std::string redirectTo;
		int redirectCode;

		bool uploadEnabled;
		std::string uploadPath;

		std::string cgiExtension;
		std::string cgiExecutablePath;
	};

	struct ServerConfig
	{
		ServerConfig();

		std::map< ResponseCodes, std::string > errorPages;
		std::vector< Location > locations;
		std::string serverName;
		std::vector<std::string> listens;
		size_t clientMaxBodySize;
		std::string rootPath;
		std::string indexPath;
		bool autoIndex;
	};

	Server();
	Server(const ServerConfig& config);
	~Server();

	void start();

	void handleEvent(int fd);
	void handleRequestTypes(Request* req);
	void handleGetRequest(Request* req, const Location& loc);
	void handlePostRequest(Request* req, const Location& loc);
	void handleDeleteRequest(Request* req, const Location& loc);
	void handleInvalidRequest();
	void handleCgiOutput(std::string cgiOutput);
	void handleDirectory(const Location& loc, std::string uri,
						 std::string filePath);
	void getHeader(Request* req);

	bool ownsFd(int fd) const;
	std::vector< struct pollfd >& getPollFds();
	void connectIfNotConnected(int fd);
	void fillCache(int fd);
	Request* deserializeRequest(Request* req);

	const Server::Location* matchLocation(const std::string& uri) const;
	std::string generateDirectoryListing(const std::string& path,
										 const std::string& uri);
	std::string executeCgi(const std::string& scriptPath,
						   const std::string& interpreter);
	std::vector< std::pair< std::string, std::string > > getListens() const;
	std::string getServerName() const;
	void removePollFd(int fd);

  private:
	std::string m_serverName;
	std::string m_clientMaxBodySize;
	std::map< ResponseCodes, std::string > m_errorPages;
	std::vector< Location > m_locations;
	std::vector< std::pair< std::string, std::string > > m_listens;
	bool m_isRunning;
	std::vector< struct pollfd > pollFds;
	std::vector< int > listenerFds;
	std::queue< Request* > requestQueue;
	std::string m_response;
	std::stringstream cache;
};

Server* findMatchingServer(const std::string& ip, int port,
						   const std::vector< Server* >& servers);

#endif
