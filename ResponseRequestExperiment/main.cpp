#include "config.hpp"
#include "logger.hpp"
#include "request.hpp"
#include "response.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#define PORT 8080

int createServerSocket()
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	addr.sin_family		 = AF_INET;
	addr.sin_port		 = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 128) == -1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	return server_fd;
}

std::string extractRequestedPath(const std::string& request)
{
	std::istringstream iss(request);
	std::string method, path;
	iss >> method >> path;
	if (path == "/")
		path = "index.html";
	else if (path[0] == '/')
		path = path.substr(1);

	logRequest(method, "/" + path);
	return path;
}

void handleNewConnection(int server_fd, std::vector<struct pollfd>& fds)
{
	int client_fd = accept(server_fd, NULL, NULL);
	if (client_fd < 0)
	{
		perror("accept");
		return;
	}
	struct pollfd clientPoll = {client_fd, POLLIN, 0};
	fds.push_back(clientPoll);
}

void handleClientRequest(std::vector<struct pollfd>& fds, size_t i)
{
	Request req;
	Config conf;
	int client_fd	  = fds[i].fd;
	char buffer[2048] = {0};
	int bytes		  = read(client_fd, buffer, sizeof(buffer));
	if (bytes <= 0)
	{
		close(client_fd);
		fds.erase(fds.begin() + i);
		clientContexts.erase(client_fd);
		return;
	}

	std::string request(buffer, bytes);
	std::string path = extractRequestedPath(request);
	std::string status;
	prepareHttpResponse(client_fd, req, conf);
	fds[i].events |= POLLOUT;

	clientContexts[client_fd].status = status;
}

void handleClientWrite(std::vector<struct pollfd>& fds, size_t i)
{
	int client_fd = fds[i].fd;
	if (clientContexts.count(client_fd))
	{
		ClientContext& ctx = clientContexts[client_fd];
		const char* data   = ctx.buffer.c_str() + ctx.bytes_sent;
		size_t remaining   = ctx.buffer.size() - ctx.bytes_sent;

		ssize_t sent = send(client_fd, data, remaining, 0);
		if (sent < 0)
		{
			perror("send");
			close(client_fd);
			clientContexts.erase(client_fd);
			fds.erase(fds.begin() + i);
			return;
		}

		ctx.bytes_sent += sent;

		if (ctx.bytes_sent >= ctx.buffer.size())
		{
			logResponse(client_fd, ctx.status, ctx.buffer.size());
			close(client_fd);
			clientContexts.erase(client_fd);
			fds.erase(fds.begin() + i);
		}
	}
}

void runServer(int server_fd)
{
	std::vector<struct pollfd> fds(1);
	fds[0].fd	  = server_fd;
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
				if (fds[i].fd == server_fd)
					handleNewConnection(server_fd, fds);
				else
					handleClientRequest(fds, i);
			}
			else if (fds[i].revents & POLLOUT)
			{
				handleClientWrite(fds, i);
			}
		}
	}
}

int main()
{
	int server_fd = createServerSocket();
	std::cout << "Webserv is running. Port: " << PORT << "\n";
	runServer(server_fd);
	close(server_fd);
	return 0;
}
