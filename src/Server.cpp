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
	this->m_listens			  = config.m_listens;
	this->m_isRunning		  = false;
}

Server::~Server() {}

Server::ServerConfig::ServerConfig()
{
	this->m_serverName		  = "";
	this->m_clientMaxBodySize = "1M";
	this->m_errorPages		  = std::map<int, std::string>();
	this->m_locations		  = std::vector<Server::Location>();
	this->m_listens			  = std::vector<std::string>(1,"80");
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
