#include "Server.hpp"

Server::Server()
	: host("127.0.0.1"), serverName("Test"), port(8080),
	  clientMaxBodySize(std::numeric_limits<int>::max())
{
	errorPages[404] = "Custom-404-url";
}

Server::Server(const std::string& conf)
	: host("127.0.0.1"), serverName("Test"), port(8080),
	  clientMaxBodySize(std::numeric_limits<int>::max())
{
	(void)conf;
}

Server::~Server() {}


