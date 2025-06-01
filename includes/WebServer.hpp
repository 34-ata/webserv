#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "Server.hpp"
#include <string>
#include <vector>
#include <fstream>
#include "Tokenizer.hpp"

class WebServer
{
  public:
	WebServer();
	WebServer(const WebServer& other);
	WebServer& operator=(const WebServer& other);
	~WebServer();

	bool Init(const std::string& configFile);
	void Run();

  private:
	void Parse(std::ifstream& fileIn);
	void ParseServer(std::list<std::string>::const_iterator start,
					 std::list<std::string>::const_iterator end);

  private:
	ConfigBlock m_root;
	std::vector<Server*> m_servers;
};

#endif
