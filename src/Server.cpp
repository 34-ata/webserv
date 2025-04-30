#include "Server.hpp"

#define PORT 8080

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
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd == -1) {perror("Socket"); exit(EXIT_FAILURE);}

	int opt = 1;
	setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	addr.sin_family		 = AF_INET;
	addr.sin_port		 = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverFd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(serverFd, 128) == -1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	std::cout << "serverFd: " << serverFd << std::endl;
	this->m_isRunning = true;
	return this->m_isRunning;
}

void handleNewConnection(int serverFd, std::vector<struct pollfd>& fds)
{
	int clientFd = accept(serverFd, NULL, NULL);
	std::cout << "Accepted new connection: " << clientFd << std::endl;
	if (clientFd < 0)
	{
		perror("accept");
		return;
	}
	struct pollfd clientPoll = {clientFd, POLLIN, 0};
	fds.push_back(clientPoll);
}

void handleClientRequest(void)
{
	return ;
}

void handleClientWrite(void)
{
	return ;
}

void Server::Run()
{
	std::vector<struct pollfd> fds(1);
	fds[0].fd	  = serverFd;
	fds[0].events = POLLIN;

	while (true)
	{
		int ret = poll(&fds[0], fds.size(), -1);
		if (ret < 0)
		{
			perror("poll");
			break;
		}

		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == serverFd)
					handleNewConnection(serverFd, fds);
				else
					handleClientRequest();
			}
			else if (fds[i].revents & POLLOUT)
			{
				handleClientWrite();
			}
		}
	}
}

void Server::Stop()
{
	// Burada Socketler kapatılıp temizlenecek.
	this->m_isRunning = false;
}
