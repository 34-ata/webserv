#ifndef WEBSERVER_HPP

#include "Server.hpp"
#include <string>
#include <vector>
#include <fstream>

class WebServer
{
  public:
	WebServer();
	WebServer(const WebServer& other);
	WebServer& operator=(const WebServer& other);
	~WebServer();

  public:
	bool Init(const std::string& configFile);

  private:
	void Parse(std::ifstream& fileIn);

  private:
	std::vector<Server> m_servers;
};

#endif // !WEBSERVER_HPP
