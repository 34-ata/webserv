#include "Tokenizer.hpp"
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
	void ParseServer(std::list<std::string>::const_iterator start,
				 std::list<std::string>::const_iterator end);
  private:
	ConfigBlock m_root;
	std::vector<Server> m_servers;
};

#endif // !WEBSERVER_HPP
