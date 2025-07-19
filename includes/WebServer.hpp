#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "Server.hpp"
#include "Tokenizer.hpp"
#include <string>
#include <vector>

class WebServer
{
  public:
	WebServer();
	WebServer(const WebServer& other);
	WebServer& operator=(const WebServer& other);
	~WebServer();

	bool Init(const std::string& configFile);
	void Run();
	void Shutdown();

  private:
	void Parse(std::ifstream& fileIn);
	void ParseServer(std::list< std::string >::const_iterator start,
		std::list< std::string >::const_iterator end);
	std::string parseHostHeader(int fd);
	void checkTimeouts();
	Server::ServerConfig createServerConfig(const ConfigBlock& server);
	Server::Location createLocation(const ConfigBlock& location);

  private:
	ConfigBlock m_root;
	std::vector< Server* > m_servers;
};

#endif
