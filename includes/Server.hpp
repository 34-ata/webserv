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
		std::vector<int> m_listens;
		std::string m_clientMaxBodySize;
		bool m_isRunning;
	};

  public:
	Server();
	Server(const ServerConfig& config);
	~Server();

  public:
	void Start();
    void Run();
    bool handleClient(int clientFd);

	void Stop();

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

#endif // !SERVER_HPP
