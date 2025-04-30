#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstdio>
#include <poll.h>

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
  private:
	struct Location
	{
		std::string locUrl;
		std::string rootPath;
		std::string indexFile;
		bool autoIndex;
		std::vector<HttpMethods> allowedMethods;
	};

  public:
	struct ServerConfig
	{
		ServerConfig();

		std::map<int, std::string> m_errorPages;
		std::vector<Location> m_locations;
		std::string m_serverName;
		std::vector<std::string> m_listens;
		std::string m_clientMaxBodySize;
		bool m_isRunning;
	};

  public:
	Server();
	Server(const ServerConfig& config);
	~Server();

  public:
	// It returns true when starting failed.
	bool Start();
	void Run();
	void Stop();
	int serverFd;

  private:
	std::map<int, std::string> m_errorPages;
	std::vector<Location> m_locations;
	std::string m_serverName;
	std::vector<std::string> m_listens;
	std::string m_clientMaxBodySize;
	bool m_isRunning;
};

#endif // !SERVER_HPP
