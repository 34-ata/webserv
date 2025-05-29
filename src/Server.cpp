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

Server::~Server()
{
	for (std::vector<struct pollfd>::iterator it = pollFds.begin(); it != pollFds.end(); ++it)
	{
        close(it->fd);
	}
}

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
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd == -1)
	{
		fprintf(stderr, "Error while trying to create socket!\n");
		return (-1);
	}
	int opt = 1;
	if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		fprintf(stderr, "Error while setting socket option!\n");
		return (-1);
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverFd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		fprintf(stderr, "Error while binding socket!\n");
		return (-1);
	}

	return (EXIT_SUCCESS);
}

void Server::Run()
{
	if (listen(serverFd, SOMAXCONN) < 0)
	{
		fprintf(stderr, "Error while starting to listen socket!\n");
		return ;
	}

	struct pollfd pfd;
    pfd.fd = serverFd;
    pfd.events = POLLIN;
    pollFds.push_back(pfd);

    while (true)
	{
        int ret = poll(&pollFds[0], pollFds.size(), -1);
        if (ret < 0)
		{
            perror("poll");
            break;
        }

        for (size_t i = 0; i < pollFds.size(); ++i)
		{
            if (pollFds[i].revents & POLLIN)
			{
                if (pollFds[i].fd == serverFd)
				{
                    struct sockaddr_in clientAddr;
                    socklen_t clientLen = sizeof(clientAddr);
                    int clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &clientLen);
                    if (clientFd < 0)
					{
                        perror("accept");
                        continue;
                	}
                	struct pollfd clientPoll;
                	clientPoll.fd = clientFd;
                	clientPoll.events = POLLIN;
                	pollFds.push_back(clientPoll);

                	std::cout << "New client connected: fd=" << clientFd << std::endl;
                }
				else
				{
                	handleClient(pollFds[i].fd);
    				close(pollFds[i].fd);
    				pollFds.erase(pollFds.begin() + i);
    				--i;
                }
            }
        }
    }
}

void Server::handleClient(int clientFd)
{
	printf("New connection: %d\n", clientFd);
	char buffer[1024];
    int bytes = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytes <= 0)
	{
        std::cerr << "Disconnected: fd=" << clientFd << std::endl;
		return ;
	}
	buffer[bytes] = '\0';
	std::cout << "Received complete request from fd=" << clientFd << ":\n"
              << buffer << std::endl;

	int fd = open("sample.html", O_RDONLY);
	char response[1024];
	read(fd, response, 1023);
	response[1024] = '\0';
	char header[512];
	int body_length = 1024;
	sprintf(header,
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Content-Length: %d\r\n"
    "Connection: close\r\n"
    "\r\n",
    body_length);
	send(clientFd, header, strlen(header), 0);
	send(clientFd, response, 1024, 0);
	printf("Message sent to the client!\n");
}

void Server::Stop()
{
	// Burada Socketler kapatılıp temizlenecek.
	this->m_isRunning = false;
}
