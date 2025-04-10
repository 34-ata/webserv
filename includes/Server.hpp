#ifndef SERVER_HPP
#define SERVER_HPP

#include <limits>
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
	Server(const std::string& conf);
	Server(const Server& other);
	Server& operator=(const Server& other);
	~Server();

  private:
	std::map<int, std::string> errorPages;
	std::vector<Location> locations;
	const std::string host;
	const std::string serverName;
	const int port;
	const int clientMaxBodySize;
};

#endif // !SERVER_HPP
