#include "Server.hpp"

Server::Server() {}

Server::Server(const Server& other) { *this = other; }

Server& Server::operator=(const Server& other)
{
	this->m_serverName		  = other.m_serverName;
	this->m_clientMaxBodySize = other.m_clientMaxBodySize;
	this->m_errorPages		  = other.m_errorPages;
	this->m_host			  = other.m_host;
	this->m_locations		  = other.m_locations;
	this->m_port			  = other.m_port;
	return *this;
}

Server::~Server() {}

bool Server::Start() { return false; }

void Server::Stop() {}
