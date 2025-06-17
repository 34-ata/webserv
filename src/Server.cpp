#include "Server.hpp"
#include "../includes/Response.hpp"
#include "HttpMethods.hpp"
#include "HttpVersion.hpp"
#include "Log.hpp"
#include "Request.hpp"
#include "ResponseCodes.hpp"
#include "SyntaxException.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

Server::Server(const Server::ServerConfig& config)
{
	this->m_serverName		  = config.serverName;
	this->m_clientMaxBodySize = config.clientMaxBodySize;
	this->m_errorPages		  = config.errorPages;
	this->m_locations		  = config.locations;
	for (size_t i = 0; i < config.listens.size(); i++)
	{
		size_t seperaterPos = config.listens[i].find(':');
		if (seperaterPos != std::string::npos)
		{
			std::string ip	 = config.listens[i].substr(0, seperaterPos);
			std::string port = config.listens[i].substr(seperaterPos + 1);
			m_listens.push_back(std::make_pair(ip, port));
		}
		else
			m_listens.push_back(std::make_pair("0.0.0.0", config.listens[i]));
	}
	this->m_isRunning = false;
}

Server::ServerConfig::ServerConfig()
{
	serverName			  = "";
	clientMaxBodySize	  = 1000000;
	errorPages[NOT_FOUND] = "404.html";
	listens.push_back("0.0.0.0:8080");
}

Server::~Server()
{
	for (std::vector< struct pollfd >::iterator it = pollFds.begin();
		 it != pollFds.end(); ++it)
	{
		close(it->fd);
	}
	while (0 < requestQueue.size())
	{
		delete requestQueue.front();
		requestQueue.pop();
	}
}

int getPortFromSocket(int fd)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	if (getsockname(fd, (struct sockaddr*)&addr, &len) == -1)
		return -1;
	return ntohs(addr.sin_port);
}

Server* findMatchingServer(const std::string& ip, int port,
						   const std::vector< Server* >& servers)
{
	for (size_t i = 0; i < servers.size(); ++i)
	{
		const std::vector< std::pair< std::string, std::string > >& listens =
			servers[i]->getListens();
		for (size_t j = 0; j < listens.size(); ++j)
		{
			if (listens[j].first == ip
				&& std::atoi(listens[j].second.c_str()) == port)
				return servers[i];
		}
	}

	for (size_t i = 0; i < servers.size(); ++i)
	{
		const std::vector< std::pair< std::string, std::string > >& listens =
			servers[i]->getListens();
		for (size_t j = 0; j < listens.size(); ++j)
		{
			if (std::atoi(listens[j].second.c_str()) == port)
				return servers[i];
		}
	}

	return servers[0];
}

void Server::start()
{
	for (size_t i = 0; i < this->m_listens.size(); ++i)
	{
		std::string ip		= this->m_listens[i].first;
		std::string portStr = this->m_listens[i].second;
		int port			= std::atoi(portStr.c_str());

		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd == -1)
		{
			perror("socket");
			continue;
		}

		int opt = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port	= htons(port);

		if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0)
		{
			std::cerr << "Invalid IP address: " << ip << std::endl;
			close(fd);
			continue;
		}

		if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		{
			perror("bind");
			close(fd);
			continue;
		}

		if (listen(fd, SOMAXCONN) < 0)
		{
			perror("listen");
			close(fd);
			continue;
		}

		fcntl(fd, F_SETFL, O_NONBLOCK);

		struct pollfd pfd;
		pfd.fd	   = fd;
		pfd.events = POLLIN;

		pollFds.push_back(pfd);
		listenerFds.push_back(fd);

		std::cout << "Listening on " << ip << ":" << port << " (fd=" << fd
				  << ")" << std::endl;
	}
}

bool Server::ownsFd(int fd) const
{
	for (size_t i = 0; i < pollFds.size(); ++i)
	{
		if (pollFds[i].fd == fd)
			return true;
	}
	return false;
}

void Server::removePollFd(int fd)
{
	for (size_t i = 0; i < pollFds.size(); ++i)
	{
		if (pollFds[i].fd == fd)
		{
			pollFds.erase(pollFds.begin() + i);
			break;
		}
	}
}

void Server::connectIfNotConnected(int fd)
{
	if (std::find(listenerFds.begin(), listenerFds.end(), fd)
		!= listenerFds.end())
	{
		LOG("Added Socket to listenerFds.");
		struct sockaddr_in clientAddr;
		socklen_t len = sizeof(clientAddr);
		int clientFd  = accept(fd, (struct sockaddr*)&clientAddr, &len);
		if (clientFd >= 0)
		{
			fcntl(fd, F_SETFL, O_NONBLOCK);
			struct pollfd clientPoll;
			clientPoll.fd	  = clientFd;
			clientPoll.events = POLLIN;
			pollFds.push_back(clientPoll);
		}
	}
}

void Server::fillCache(int fd)
{
	char buffer[1024];
	memset(&buffer, 0, 1024);
	int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes > 0)
	{
		buffer[bytes] = 0;
		cache << buffer;
	}
}

Request* Server::deserializeRequest(Request* req)
{
	req->fillRequest(cache.str());
	req->checkIntegrity();
	return req;
}

void Server::getHeader(Request* req)
{
	bool headerFound = false;
	std::string res;
	while (std::getline(cache, res))
	{
		res.append("\n");
		req->fillRequest(res);
		if (req->getData().find("\r\n\r\n") != std::string::npos)
		{
			headerFound = true;
			break;
		}
	}
	if (!headerFound)
		req->setBadRequest();
	req->checkIntegrity();
	res.clear();
}

void Server::handleEvent(int fd)
{
	cache.str(std::string());
	static Request* req = NULL;
	connectIfNotConnected(fd);
	fillCache(fd);
	if (cache.str().find("\r\n\r\n") != std::string::npos || req != NULL)
	{
		if (req == NULL)
		{
			req = new Request();
		}
		deserializeRequest(req);

		if (req->getBodyLenght() == req->getBody().size())
		{
			handleRequestTypes(req);
			send(fd, m_response.c_str(), m_response.size(), 0);
			close(fd);
			delete req;
			req = NULL;
			removePollFd(fd);
		}
	}
}

bool isDirectory(const std::string& path)
{
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return false;
	return S_ISDIR(statbuf.st_mode);
}

std::string Server::generateDirectoryListing(const std::string& path, const std::string& uri)
{
	std::stringstream html;
	DIR* dir = opendir(path.c_str());

	if (!dir)
		return "<html><body><h1>403 Forbidden</h1></body></html>";

	html << "<html><head><title>Index of " << uri << "</title></head><body>";
	html << "<h1>Index of " << uri << "</h1><ul>";

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == "." || name == "..")
			continue;

		html << "<li><a href=\"" << uri;
		if (!uri.empty() && uri[uri.length() - 1] != '/')
			html << "/";
		html << name << "\">" << name << "</a></li>";
	}

	html << "</ul></body></html>";
	closedir(dir);
	return html.str();
}

std::string Server::executeCgi(const std::string& scriptPath,
							   const std::string& interpreter)
{
	int pipefd[2];
	if (pipe(pipefd) == -1)
		return "CGI error: pipe() failed\n";

	pid_t pid = fork();
	if (pid == -1)
		return "CGI error: fork() failed\n";

	if (pid == 0)
	{
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		char* args[] = {const_cast< char* >(interpreter.c_str()),
						const_cast< char* >(scriptPath.c_str()), NULL};

		char* env[] = {const_cast< char* >("GATEWAY_INTERFACE=CGI/1.1"),
					   const_cast< char* >("SERVER_PROTOCOL=HTTP/1.1"), NULL};

		execve(args[0], args, env);
		perror("execve");
		exit(1);
	}

	close(pipefd[1]);
	std::string output;
	char buffer[1024];
	ssize_t bytesRead;

	while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
		output.append(buffer, bytesRead);

	close(pipefd[0]);
	waitpid(pid, NULL, 0);

	return output;
}

void Server::handleCgiOutput(std::string cgiOutput)
{
	size_t headerEnd = cgiOutput.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		headerEnd = cgiOutput.find("\n\n");

	std::string headers, body;
	if (headerEnd != std::string::npos)
	{
		headers = cgiOutput.substr(0, headerEnd);
		body	= cgiOutput.substr(headerEnd
								   + (cgiOutput[headerEnd] == '\r' ? 4 : 2));
	}
	else
	{
		body = cgiOutput;
	}

	Response response;
	response.status(OK).htppVersion(HTTP_VERSION).body(body);

	size_t ctPos = headers.find("Content-Type:");
	if (ctPos != std::string::npos)
	{
		size_t endLine	   = headers.find('\n', ctPos);
		std::string ctLine = headers.substr(ctPos, endLine - ctPos);
		size_t sep		   = ctLine.find(":");
		if (sep != std::string::npos)
		{
			std::string value = ctLine.substr(sep + 1);
			while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
				value.erase(0, 1);
			response.header("Content-Type", value);
		}
	}
	m_response = response.build();
}

std::string joinPaths(const std::string& root, const std::string& sub)
{
	if (root.empty()) return sub;
	if (sub.empty()) return root;

	char lastRootChar = root[root.length() - 1];
	char firstSubChar = sub[0];

	if (lastRootChar == '/' && firstSubChar == '/')
		return root + sub.substr(1);
	else if (lastRootChar != '/' && firstSubChar != '/')
		return root + "/" + sub;
	else
		return root + sub;
}

void Server::handleDirectory(const Location& loc, std::string uri, std::string filePath)
{
	Response response;


	if (!loc.indexPath.empty())
	{
		std::string indexFilePath = joinPaths(filePath, loc.indexPath);
		if (access(indexFilePath.c_str(), F_OK) == 0)
		{
			if (access(indexFilePath.c_str(), R_OK) == 0)
			{
				m_response = response.status(OK)
									.htppVersion(HTTP_VERSION)
									.header("Content-Type", getContentType(indexFilePath))
									.body(getFileContent(indexFilePath))
									.build();
			}
			else
			{
				std::string errorBody = getErrorPageContent(FORBIDDEN);
				m_response = response.status(FORBIDDEN)
									.htppVersion(HTTP_VERSION)
									.header("Content-Type", "text/html")
									.body(errorBody)
									.build();
			}
			return;
		}
	}

	if (loc.autoIndex)
	{
		std::string listing = generateDirectoryListing(filePath, uri);
		m_response = response.status(OK)
							 .htppVersion(HTTP_VERSION)
							 .header("Content-Type", "text/html")
							 .body(listing)
							 .build();
		return;
	}

	std::string errorBody = getErrorPageContent(FORBIDDEN);
	m_response = response.status(FORBIDDEN)
						.htppVersion(HTTP_VERSION)
						.header("Content-Type", "text/html")
						.body(errorBody)
						.build();

}

std::string Server::getErrorPageContent(ResponseCodes code)
{
	std::map<ResponseCodes, std::string>::const_iterator it = m_errorPages.find(code);
	if (it != m_errorPages.end())
	{
		std::string fullPath = "." + it->second;
		if (access(fullPath.c_str(), F_OK) == 0)
			return getFileContent(fullPath);
		else
			return "<html><body><h1>" + Response::mapCodeToStr(code) + " (error page missing)</h1></body></html>";
	}

	std::stringstream ss;
	ss << "<html><body><h1>" << static_cast<int>(code) << " " << Response::mapCodeToStr(code) << "</h1></body></html>";
	return ss.str();
}

void Server::handleGetRequest(Request* req, const Location& loc)
{
	LOG("Started Handling GET Request");
	Response response;
	std::string uri = req->getPath();

	std::string relativePath = uri.substr(loc.locUrl.length());
	std::string filePath     = joinPaths("." + loc.rootPath, relativePath);

	if (!loc.cgiExtension.empty() && filePath.size() >= loc.cgiExtension.size()
		&& filePath.compare(filePath.size() - loc.cgiExtension.size(),
							loc.cgiExtension.size(), loc.cgiExtension)
			   == 0)
	{
		handleCgiOutput(executeCgi(filePath, loc.cgiExecutablePath));
		return;
	} 
	if (isDirectory(filePath))
	{
		handleDirectory(loc, uri, filePath);
		return;
	}

	if (access(filePath.c_str(), F_OK) != 0)
	{
		std::string errorBody = getErrorPageContent(NOT_FOUND);
		m_response = response.status(NOT_FOUND)
							.htppVersion(HTTP_VERSION)
							.header("Content-Type", "text/html")
							.body(errorBody)
							.build();
		return;
	}
	else if (access(filePath.c_str(), R_OK) != 0)
	{
		std::string errorBody = getErrorPageContent(FORBIDDEN);
		m_response = response.status(FORBIDDEN)
							.htppVersion(HTTP_VERSION)
							.header("Content-Type", "text/html")
							.body(errorBody)
							.build();
		return;
	}

	m_response = response.status(OK)
					 .htppVersion(HTTP_VERSION)
					 .body(getFileContent(filePath))
					 .header("Content-Type", getContentType(filePath))
					 .build();
}

void Server::handlePostRequest(Request* req, const Location& loc)
{
	LOG("Started Handling POST Request");
	Response response;

	if (std::find(loc.allowedMethods.begin(), loc.allowedMethods.end(), POST) == loc.allowedMethods.end())
	{
		std::string errorBody = getErrorPageContent(FORBIDDEN);
		m_response = response.status(FORBIDDEN)
							 .htppVersion(HTTP_VERSION)
							 .header("Content-Type", "text/html")
							 .body(errorBody)
							 .build();
		return;
	}

	std::string path = joinPaths("." + loc.rootPath, req->getPath().substr(loc.locUrl.length()));
	if (!loc.cgiExtension.empty()
		&& path.size() >= loc.cgiExtension.size()
		&& path.compare(path.size() - loc.cgiExtension.size(), loc.cgiExtension.size(), loc.cgiExtension) == 0)
	{
		handleCgiOutput(executeCgi(path, loc.cgiExecutablePath));
		return;
	}

	LOG("uploadPath: " << loc.uploadPath);
	if (loc.uploadPath.empty())
	{
		std::string errorBody = getErrorPageContent(SERVER_ERROR);
		m_response = response.status(SERVER_ERROR)
							 .htppVersion(HTTP_VERSION)
							 .header("Content-Type", "text/html")
							 .body(errorBody)
							 .build();
		return;
	}

	std::string body = req->getBody();
	if (body.empty())
	{
		std::string errorBody = getErrorPageContent(BAD_REQ);
		m_response = response.status(BAD_REQ)
							 .htppVersion(HTTP_VERSION)
							 .header("Content-Type", "text/html")
							 .body(errorBody)
							 .build();
		return;
	}

	std::stringstream ss;
	ss << std::time(NULL);
	std::string filename = loc.uploadPath + "/upload_" + ss.str();

	std::ofstream file(filename.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		std::string errorBody = getErrorPageContent(SERVER_ERROR);
		m_response = response.status(SERVER_ERROR)
							 .htppVersion(HTTP_VERSION)
							 .header("Content-Type", "text/html")
							 .body(errorBody)
							 .build();
		return;
	}

	file << body;
	file.close();

	m_response = response.status(CREATED)
						 .htppVersion(HTTP_VERSION)
						 .body("201 Created: File uploaded successfully.\n")
						 .build();
}

void Server::handleDeleteRequest(Request* req, const Location& loc)
{
	LOG("Started Handling DELETE Request");
	Response response;

	std::string uri			 = req->getPath();
	std::string relativePath = uri.substr(loc.locUrl.length());
	std::string filePath = joinPaths("." + loc.rootPath, relativePath);

	if (access(filePath.c_str(), F_OK) != 0)
	{
		std::string errorBody = getErrorPageContent(NOT_FOUND);
		m_response = response.status(NOT_FOUND)
							.htppVersion(HTTP_VERSION)
							.header("Content-Type", "text/html")
							.body(errorBody)
							.build();
		return;
	}
	if (std::remove(filePath.c_str()) != 0)
	{
		std::string errorBody = getErrorPageContent(FORBIDDEN);
		m_response = response.status(FORBIDDEN)
							.htppVersion(HTTP_VERSION)
							.header("Content-Type", "text/html")
							.body(errorBody)
							.build();
		return;
	}

	m_response = response.status(OK)
					 .htppVersion(HTTP_VERSION)
					 .body("200 OK: File deleted successfully.\n")
					 .build();
}

void Server::handleInvalidRequest()
{
	Response response;
	std::string errorBody = getErrorPageContent(METH_NOT_ALLOW);
	m_response = response.htppVersion(HTTP_VERSION)
						 .status(METH_NOT_ALLOW)
						 .header("Content-Type", "text/html")
						 .body(errorBody)
						 .build();
}

const Server::Location* Server::matchLocation(const std::string& uri) const
{
	const Location* bestMatch = NULL;
	size_t bestLength = 0;

	for (std::vector<Location>::const_iterator it = m_locations.begin();
		 it != m_locations.end(); ++it)
	{
		const std::string& locPath = it->locUrl;
		if (uri.find(locPath) == 0)
		{
			if (uri.length() == locPath.length()
				|| uri[locPath.length()] == '/')
			{
				if (locPath.length() > bestLength)
				{
					bestMatch = &(*it);
					bestLength = locPath.length();
				}
			}
		}
	}
	return bestMatch;
}

void Server::handleRequestTypes(Request* req)
{
	if (req->getBadRequest())
	{
		std::string errorBody = getErrorPageContent(BAD_REQ);
		m_response = Response().htppVersion(HTTP_VERSION)
							.status(BAD_REQ)
							.header("Content-Type", "text/html")
							.body(errorBody)
							.build();
		return;
	}

	const Location* loc = matchLocation(req->getPath());

	if (!loc)
	{
		std::string errorBody = getErrorPageContent(NOT_FOUND);
		m_response = Response().htppVersion(HTTP_VERSION)
							.status(NOT_FOUND)
							.header("Content-Type", "text/html")
							.body(errorBody)
							.build();
		return;
	}

	if (std::find(loc->allowedMethods.begin(), loc->allowedMethods.end(),
				  req->getMethod())
		== loc->allowedMethods.end())
	{
		handleInvalidRequest();
		return;
	}

	if (loc->hasRedirect)
	{
		std::stringstream response;
		response << "HTTP/1.1 " << loc->redirectCode << " Moved\r\n";
		response << "Location: " << loc->redirectTo << "\r\n";
		response << "Content-Length: 0\r\n\r\n";
		m_response = response.str();
		return;
	}

	switch (req->getMethod())
	{
	case GET:
		handleGetRequest(req, *loc);
		break;
	case POST:
		handlePostRequest(req, *loc);
		break;
	case DELETE:
		handleDeleteRequest(req, *loc);
		break;
	default:
		handleInvalidRequest();
		break;
	}
}

std::string Server::getServerName() const { return m_serverName; }

std::vector< std::pair< std::string, std::string > > Server::getListens() const
{
	return m_listens;
}

std::vector< struct pollfd >& Server::getPollFds() { return pollFds; }

Server::Location::Location()
{
	locUrl	  = "/";
	rootPath  = "";
	indexPath = "index.html";
	autoIndex = true;
	allowedMethods.push_back(GET);
	allowedMethods.push_back(POST);
	allowedMethods.push_back(DELETE);
	hasRedirect	  = false;
	uploadEnabled = false;
}
