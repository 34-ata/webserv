#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
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
	Server();
	Server(const Server& other);
	Server& operator=(const Server& other);
	~Server();

  public:
	// It returns true when starting failed.
	bool Start();
	void Stop();

  private:
	std::map<int, std::string> m_errorPages;
	std::vector<Location> m_locations;
	std::string m_host;
	std::string m_serverName;
	int m_port;
	int m_clientMaxBodySize;
	bool m_isRunning;
};

#endif // !SERVER_HPP
