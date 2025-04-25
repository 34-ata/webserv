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
	m_serverName		  = "";
	m_clientMaxBodySize   = "1M";
	m_errorPages		  = std::map<int, std::string>();
	m_errorPages[404]     = "404.html";
	m_locations		      = std::vector<Server::Location>();
	m_listens			  = std::vector<std::string>(1,"8080");
	m_isRunning		      = false;
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
