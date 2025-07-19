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

struct ConnectionState
{
    Request* req;
    std::string cache;
    std::string response;
    size_t responseOffset;
    time_t timeStamp;
    int listenerFd;
    
    // Constructor ekleyin
    ConnectionState() 
        : req(NULL)
        , cache("")
        , response("")
        , responseOffset(0)
        , timeStamp(0)
        , listenerFd(-1)
    {
    }
};

	Server();
	Server(const ServerConfig& config);
	~Server();

	void start();

	void handleReadEvent(int fd);
	void handleWriteEvent(int fd);
	void handleRequestTypes(int fd);
	void handleGetRequest(const Location& loc, int fd);
	void handlePostRequest(const Location& loc, int fd);
	void handleDeleteRequest(const Location& loc, int fd);
	void handleInvalidRequest(int fd);
	void handleCgiOutput(int fd, std::string cgiOutput);
	void handleDirectory(int fd, const Location& loc, std::string uri, std::string filePath);

	void getHeader(ConnectionState& state);
	bool ownsFd(int fd) const;
	std::vector< struct pollfd >& getPollFds();
	bool connectIfNotConnected(int fd);
	bool fillCache(int fd);
	void closeConnection(int fd);
	void setPollout(int fd, bool enable);

	const Server::Location* matchLocation(const std::string& uri) const;
	std::string generateDirectoryListing(const std::string& path,
										 const std::string& uri);
	std::string executeCgi(const std::string& scriptPath,
								const std::string& interpreter,
								const Request& req);
	std::vector< std::pair< std::string, std::string > > getListens() const;
	std::string getServerName() const;
	std::map<int, ConnectionState>& getConnections();
	std::string getErrorPageContent(ResponseCodes code);
	void removePollFd(int fd);

  private:
	std::map<int, ConnectionState> m_connections;
	std::map<int, time_t> m_lastActivity;
	std::string m_serverName;
	std::string m_rootPath;
	size_t m_clientMaxBodySize;
	std::map< ResponseCodes, std::string > m_errorPages;
	std::vector< Location > m_locations;
	std::vector< std::pair< std::string, std::string > > m_listens;
	bool m_isRunning;
	std::vector< struct pollfd > pollFds;
	std::vector<int> listenerFds;
	std::queue< Request* > requestQueue;
	std::string m_response;
};

Server* findMatchingServer(const std::string& ip, int port,
						   const std::vector< Server* >& servers,
						   const std::string& hostHeader);

#endif
