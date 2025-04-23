#include "Server.hpp"
#include <map>
#include <string>
#include <vector>

Server::Server(const Server::ServerConfig& config)
{
	this->m_serverName		  = config.m_serverName;
	this->m_clientMaxBodySize = config.m_clientMaxBodySize;
	this->m_errorPages		  = config.m_errorPages;
	this->m_locations		  = config.m_locations;
	this->m_listen			  = config.m_listen;
	this->m_isRunning		  = false;
}

Server::~Server() {}

Server::ServerConfig::ServerConfig()
{
	this->m_serverName		  = "";
	this->m_clientMaxBodySize = 1;
	this->m_errorPages		  = std::map<int, std::string>();
	this->m_locations		  = std::vector<Server::Location>();
	this->m_listen			  = "80";
	this->m_isRunning		  = false;
}

bool Server::Start()
{
	// Burada Socketler ve diğer sunucu başlatma kısımları olacak.
	this->m_isRunning = true;
	return this->m_isRunning;
}

void Server::Stop()
{
	// Burada Socketler kapatılıp temizlenecek.
	this->m_isRunning = false;
}
