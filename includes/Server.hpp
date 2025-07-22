#ifndef SERVER_HPP
#define SERVER_HPP

#include "HttpMethods.hpp"
#include "ResponseCodes.hpp"
#include <map>
#include <poll.h>
#include <string>
#include <vector>

class Request;
class Response;

class Server
{
  public:
	struct ServerConfig;

  public:
	struct Location
	{
		Location(const ServerConfig& config);
		std::vector< HttpMethods > allowedMethods;
		std::string locUrl;
		std::string rootPath;
		std::string serverRoot;
		std::string indexPath;
		std::string redirectTo;
		std::string uploadPath;
		std::string cgiExtension;
		std::string cgiExecutablePath;
		int redirectCode;
		bool autoIndex;
		bool hasRedirect;
	};

	struct ServerConfig
	{
		ServerConfig();
		std::map< ResponseCodes, std::string > errorPages;
		std::vector< Location > locations;
		std::vector< std::string > listens;
		std::string serverName;
		std::string rootPath;
		std::string indexPath;
		size_t clientMaxBodySize;
		bool autoIndex;
	};

	struct ConnectionState
	{
		ConnectionState();
		Request* req;
		std::string cache;
		std::string response;
		size_t responseOffset;
		time_t timeStamp;
		int listenerFd;
	};

  public:
	Server(const ServerConfig& config);
	~Server();

  public:
	void start();
	void handleReadEvent(int fd);
	void handleWriteEvent(int fd);
	void handleRequestTypes(int fd);
	void handleGetRequest(const Location& loc, int fd);
	void handlePostRequest(const Location& loc, int fd);
	void handleDeleteRequest(const Location& loc, int fd);
	void handleInvalidRequest(int fd);
	void handleCgiOutput(int fd, std::string cgiOutput);
	void handleDirectory(int fd, const Location& loc, std::string uri,
						 std::string filePath);
	std::vector< struct pollfd >& getPollFds();
	bool connectIfNotConnected(int fd);
	bool fillCache(int fd);
	void closeConnection(int fd);
	void setPollout(int fd, bool enable);
	const Server::Location* matchLocation(const std::string& uri) const;
	std::string generateDirectoryListing(const std::string& path,
										 const std::string& uri);
	std::string executeCgi(const std::string& scriptPath,
						   const std::string& interpreter, const Request& req);
	std::vector< std::pair< std::string, std::string > > getListens() const;
	std::string getServerName() const;
	std::map< int, ConnectionState >& getConnections();
	std::string getErrorPageContent(ResponseCodes code);
	void removePollFd(int fd);

  private:
	std::map< int, ConnectionState > m_connections;
	std::map< ResponseCodes, std::string > m_errorPages;
	std::vector< std::pair< std::string, std::string > > m_listens;
	std::vector< Location > m_locations;
	std::vector< struct pollfd > pollFds;
	std::vector< int > listenerFds;
	std::string m_serverName;
	std::string m_rootPath;
	std::string m_response;
	size_t m_clientMaxBodySize;
};

Server* findMatchingServer(const std::string& ip, int port,
						   const std::vector< Server* >& servers,
						   const std::string& hostHeader);

#endif
