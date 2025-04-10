#ifndef WEBSERVER_HPP

#include "Server.hpp"
#include <string>
#include <vector>

class WebServer
{
  public:
	WebServer();
	WebServer(const WebServer& other);
	WebServer& operator=(const WebServer& other);
	~WebServer();

  public:
	bool Init(const std::string& config);

  private:
	void ParseServers(const std::string& fileContent);

  private:
	std::vector<Server> servers;
};

#endif // !WEBSERVER_HPP
