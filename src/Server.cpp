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
#define TIMEOUT 5

Server::Server(const Server::ServerConfig& config)
{
	this->m_serverName		  = config.serverName;
	this->m_clientMaxBodySize = config.clientMaxBodySize;
	this->m_errorPages		  = config.errorPages;
	this->m_locations		  = config.locations;
    this->m_rootPath          = config.rootPath;
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
						   const std::vector< Server* >& servers,
						   const std::string& hostHeader)
{
	std::vector<Server*> portMatches;

	for (size_t i = 0; i < servers.size(); ++i)
	{
		const std::vector< std::pair< std::string, std::string > >& listens =
			servers[i]->getListens();

		for (size_t j = 0; j < listens.size(); ++j)
		{
			if (listens[j].first == ip
				&& std::atoi(listens[j].second.c_str()) == port)
			{
				portMatches.push_back(servers[i]);
				break;
			}
		}
	}

	if (!hostHeader.empty())
	{
		for (size_t i = 0; i < portMatches.size(); ++i)
		{
			if (portMatches[i]->getServerName() == hostHeader)
				return portMatches[i];
		}
	}

	if (!portMatches.empty())
		return portMatches[0];

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

	if (!servers.empty())
		return servers[0];

	return NULL;
}

void Server::start()
{
	for (size_t i = 0; i < this->m_listens.size(); ++i)
	{
		std::string ip		= this->m_listens[i].first;
		int port			= std::atoi(this->m_listens[i].second.c_str());

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
		listenerFds.push_back(std::make_pair(fd, false));

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

bool Server::connectIfNotConnected(int fd)
{
    for (size_t i = 0; i < listenerFds.size(); ++i)
    {
        if (listenerFds[i].first == fd)
        {
            struct sockaddr_in clientAddr;
            socklen_t len = sizeof(clientAddr);
            int clientFd = accept(fd, (struct sockaddr*)&clientAddr, &len);
            if (clientFd >= 0)
            {
                LOG("Accepted client connection");
                fcntl(clientFd, F_SETFL, O_NONBLOCK);
                struct pollfd clientPoll;
                clientPoll.fd = clientFd;
                clientPoll.events = POLLIN;
                pollFds.push_back(clientPoll);
                
                ConnectionState& state = m_connections[clientFd];
                state.listenerFd = fd;
        		state.timeStamp = time(NULL);
            }
            return true;
        }
    }
    return false;
}

void Server::fillCache(int fd)
{
    ConnectionState& state = m_connections[fd];
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0)
    {
        buffer[bytes] = 0;
        state.cache.clear();
        state.cache.append(buffer, bytes);
        state.timeStamp = time(NULL);
    }
	else if (bytes == 0)
	{
		close(fd);
		removePollFd(fd);
		m_connections.erase(fd);
	}
}

void Server::deserializeRequest(ConnectionState& state)
{
    if (!state.req)
        state.req = new Request();

    std::stringstream ss(state.cache);
    state.req->fillRequest(ss.str());
    state.req->checkIntegrity();
}

void Server::getHeader(ConnectionState& state)
{
    std::string& cache = state.cache;
    if (cache.find("\r\n\r\n") == std::string::npos)
    {
        state.req->setBadRequest();
        return;
    }
    state.req->fillRequest(cache);
    state.req->checkIntegrity();
}

void Server::closeConnection(int fd)
{
    ConnectionState& state = m_connections[fd];
    close(fd);
    removePollFd(fd);
    delete state.req;
    m_connections.erase(fd);
}

void Server::setPollout(int fd, bool enable)
{
	for (size_t i = 0; i < pollFds.size(); ++i)
	{
		if (pollFds[i].fd == fd)
		{
			if (enable)
				pollFds[i].events |= POLLOUT;
			else
				pollFds[i].events &= ~POLLOUT;
			break;
		}
	}
}

void Server::handleWriteEvent(int fd)
{
    ConnectionState& state = m_connections[fd];
    const std::string& resp = state.response;
    size_t& offset = state.responseOffset;

    ssize_t sent = send(fd, resp.c_str() + offset, resp.size() - offset, 0);
    if (sent <= 0)
    {
        closeConnection(fd);
        return;
    }

    offset += sent;
    if (offset >= resp.size())
    {
        if (state.req->shouldClose())
        {
            shutdown(fd, SHUT_WR);
            closeConnection(fd);
        }
        else
        {
            delete state.req;
            state.req = NULL;
            state.response.clear();
            offset = 0;

            setPollout(fd, false);
        }
    }
}

void Server::handleReadEvent(int fd)
{
    if (connectIfNotConnected(fd) || m_connections.find(fd) == m_connections.end())
        return;

    ConnectionState& state = m_connections[fd]; 

    fillCache(fd);
    if (state.cache.find("\r\n\r\n") != std::string::npos || state.req != NULL)
    {
        if (state.req == NULL)
        {
            state.req = new Request();
            state.timeStamp = time(NULL);
        }

        std::stringstream ss(state.cache);
        state.req->fillRequest(ss.str());
        state.req->checkIntegrity();

        if (state.req->getBodyLenght() == state.req->getBody().size())
        {
            handleRequestTypes(fd);
            state.response = m_response;
            state.responseOffset = 0;
            setPollout(fd, true);
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

std::string Server::generateDirectoryListing(const std::string& path,
											 const std::string& uri)
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
                               const std::string& interpreter,
                               const Request& req)
{
    int inputPipe[2];  // parent → child (stdin)
    int outputPipe[2]; // child → parent (stdout)

    if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1)
        return "CGI error: pipe() failed\n";

    pid_t pid = fork();
    if (pid == -1)
        return "CGI error: fork() failed\n";

    if (pid == 0)
    {
        // child process
        close(inputPipe[1]);
        close(outputPipe[0]);
        dup2(inputPipe[0], STDIN_FILENO);
        dup2(outputPipe[1], STDOUT_FILENO);
        close(inputPipe[0]);
        close(outputPipe[1]);

        // set working directory
        //size_t lastSlash = scriptPath.find_last_of('/');
        //if (lastSlash != std::string::npos)
        //    chdir(scriptPath.substr(0, lastSlash).c_str());

        std::string method = req.getMethod() == POST ? "POST" : "GET";
        std::stringstream ss;
        ss << req.getBody().size();
        std::string contentLength = ss.str();
        std::string contentType = req.getContentType();

        // prepare environment
        std::vector<std::string> env;
        env.push_back("GATEWAY_INTERFACE=CGI/1.1");
        env.push_back("SERVER_PROTOCOL=HTTP/1.1");
        env.push_back("REQUEST_METHOD=" + method);
        env.push_back("SCRIPT_NAME=" + scriptPath);
        env.push_back("PATH_INFO=" + scriptPath);
        env.push_back("CONTENT_LENGTH=" + contentLength);
        env.push_back("CONTENT_TYPE=" + contentType);

        std::vector<char*> envp;
        for (size_t i = 0; i < env.size(); ++i)
            envp.push_back(const_cast<char*>(env[i].c_str()));
        envp.push_back(NULL);

        char* args[] = {
            const_cast<char*>(interpreter.c_str()),
            const_cast<char*>(scriptPath.c_str()),
            NULL};
        
        execve(args[0], args, &envp[0]);
        perror("execve");
        exit(1);
    }

    // parent process
    close(inputPipe[0]);
    close(outputPipe[1]);

    if (!req.getBody().empty())
        write(inputPipe[1], req.getBody().c_str(), req.getBody().size());

    close(inputPipe[1]);

    std::string output;
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
        output.append(buffer, bytesRead);

    close(outputPipe[0]);
    waitpid(pid, NULL, 0);

    return output;
}

void Server::handleCgiOutput(int fd, std::string cgiOutput)
{
    ConnectionState& state = m_connections[fd];
    Request* req = state.req;

    std::string delimiter = "\r\n\r\n";
    size_t headerEnd = cgiOutput.find(delimiter);
    if (headerEnd == std::string::npos) {
        delimiter = "\n\n";
        headerEnd = cgiOutput.find(delimiter);
    }

    std::string headers, body;
    if (headerEnd != std::string::npos) {
        headers = cgiOutput.substr(0, headerEnd);
        body = cgiOutput.substr(headerEnd + delimiter.size());
    } else {
        body = cgiOutput;
    }

    Response response;
    response.status(OK)
            .httpVersion(HTTP_VERSION)
            .header("Connection", req->shouldClose() ? "close" : "keep-alive")
            .body(body);

    size_t ctPos = headers.find("Content-Type:");
    if (ctPos != std::string::npos) {
        size_t endLine = headers.find('\n', ctPos);
        std::string ctLine = headers.substr(ctPos, endLine - ctPos);
        size_t sep = ctLine.find(":");
        if (sep != std::string::npos) {
            std::string value = ctLine.substr(sep + 1);
            while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
                value.erase(0, 1);
            response.header("Content-Type", value);
        }
    } else {
        response.header("Content-Type", "text/html");
    }

    m_response = response.build();

    // Eğer buradan çağrılmıyorsa bu kısımlar handleRequestTypes içinde çağrılmalı
    state.response = m_response;
    state.responseOffset = 0;
    setPollout(fd, true);
}

std::string joinPaths(const std::string& root, const std::string& sub)
{
	if (root.empty())
		return sub;
	if (sub.empty())
		return root;

	char lastRootChar = root[root.length() - 1];
	char firstSubChar = sub[0];

	if (lastRootChar == '/' && firstSubChar == '/')
		return root + sub.substr(1);
	else if (lastRootChar != '/' && firstSubChar != '/')
		return root + "/" + sub;
	else
		return root + sub;
}

void Server::handleDirectory(int fd, const Location& loc, std::string uri, std::string filePath)
{
    Response response;
    Request* req = m_connections[fd].req;

    if (!loc.indexPath.empty())
    {
        std::string indexFilePath = joinPaths(filePath, loc.indexPath);
        if (access(indexFilePath.c_str(), F_OK) == 0)
        {
            if (access(indexFilePath.c_str(), R_OK) == 0)
            {
                m_response =
                    response.status(OK)
                        .httpVersion(HTTP_VERSION)
                        .header("Content-Type", getContentType(indexFilePath))
                        .header("Connection", req->shouldClose() ? "close" : "keep-alive")
                        .body(getFileContent(indexFilePath))
                        .build();
                return;
            }
            else
            {
                std::string errorBody = getErrorPageContent(FORBIDDEN);
                m_response = response.status(FORBIDDEN)
                             .httpVersion(HTTP_VERSION)
                             .header("Content-Type", "text/html")
                             .header("Connection", req->shouldClose() ? "close" : "keep-alive")
                             .body(errorBody)
                             .build();
                return;
            }
        }
        else if (!loc.autoIndex)
        {
            std::string errorBody = getErrorPageContent(FORBIDDEN);
            m_response = response.status(FORBIDDEN)
                            .httpVersion(HTTP_VERSION)
                            .header("Content-Type", "text/html")
                            .header("Connection", req->shouldClose() ? "close" : "keep-alive")
                            .body(errorBody)
                            .build();
            return;
        }
    }

    if (loc.autoIndex)
    {
        std::string listing = generateDirectoryListing(filePath, uri);
        m_response = response.status(OK)
                     .httpVersion(HTTP_VERSION)
                     .header("Content-Type", "text/html")
                     .header("Connection", req->shouldClose() ? "close" : "keep-alive")
                     .body(listing)
                     .build();
        return;
    }

    std::string errorBody = getErrorPageContent(NOT_FOUND);
    m_response = response.status(NOT_FOUND)
                 .httpVersion(HTTP_VERSION)
                 .header("Content-Type", "text/html")
                 .header("Connection", req->shouldClose() ? "close" : "keep-alive")
                 .body(errorBody)
                 .build();
}

std::string Server::getErrorPageContent(ResponseCodes code)
{
	std::map< ResponseCodes, std::string >::const_iterator it =
		m_errorPages.find(code);
	if (it != m_errorPages.end())
	{
		std::string fullPath = "." + m_rootPath + it->second;
		if (access(fullPath.c_str(), F_OK) == 0)
			return getFileContent(fullPath);
		else
			return "<html><body><h1>" + Response::mapCodeToStr(code)
				   + " (error page missing)</h1></body></html>";
	}

	std::stringstream ss;
	ss << "<html><body><h1>" << static_cast< int >(code) << " "
	   << Response::mapCodeToStr(code) << "</h1></body></html>";
	return ss.str();
}

void Server::handleGetRequest(const Location& loc, int fd)
{
    LOG("Started Handling GET Request");
    Response response;
    ConnectionState& state = m_connections[fd];
    Request* req = state.req;

    std::string uri = req->getPath();
    std::string relativePath = uri.substr(loc.locUrl.length());
    std::string filePath = joinPaths("." + loc.rootPath, relativePath);
    
    if (!loc.cgiExtension.empty() && filePath.size() >= loc.cgiExtension.size()
    && filePath.compare(filePath.size() - loc.cgiExtension.size(),
                            loc.cgiExtension.size(), loc.cgiExtension) == 0)
    {
        handleCgiOutput(fd, executeCgi(filePath, "/usr/bin/python3", *req));
        return;
    }
    if (isDirectory(filePath))
    {
        handleDirectory(fd, loc, uri, filePath);
        return;
    }
    
    if (access(filePath.c_str(), F_OK) != 0)
    {
        std::string errorBody = getErrorPageContent(NOT_FOUND);
        m_response = response.status(NOT_FOUND)
            .httpVersion(HTTP_VERSION)
            .header("Content-Type", "text/html")
            .header("Connection", req->shouldClose() ? "close" : "keep-alive")
            .body(errorBody)
            .build();
            return;
    }
    else if (access(filePath.c_str(), R_OK) != 0)
    {
        std::string errorBody = getErrorPageContent(FORBIDDEN);
        m_response = response.status(FORBIDDEN)
        .httpVersion(HTTP_VERSION)
            .header("Content-Type", "text/html")
            .header("Connection", req->shouldClose() ? "close" : "keep-alive")
            .body(errorBody)
            .build();
            return;
    }

    m_response = response.status(OK)
        .httpVersion(HTTP_VERSION)
        .body(getFileContent(filePath))
        .header("Content-Type", getContentType(filePath))
        .header("Connection", req->shouldClose() ? "close" : "keep-alive")
        .build();
}

void Server::handlePostRequest(const Location& loc, int fd)
{
    LOG("Started Handling POST Request");
    Response response;
    ConnectionState& state = m_connections[fd];
    Request* req = state.req;

    if (std::find(loc.allowedMethods.begin(), loc.allowedMethods.end(), POST)
        == loc.allowedMethods.end())
    {
        std::string errorBody = getErrorPageContent(FORBIDDEN);
        m_response = response.status(FORBIDDEN)
            .httpVersion(HTTP_VERSION)
            .header("Content-Type", "text/html")
            .header("Connection", req->shouldClose() ? "close" : "keep-alive")
            .body(errorBody)
            .build();
        return;
    }

    std::string path = joinPaths("." + loc.rootPath,
                                 req->getPath().substr(loc.locUrl.length()));

    if (!loc.cgiExtension.empty() && path.size() >= loc.cgiExtension.size()
        && path.compare(path.size() - loc.cgiExtension.size(),
                        loc.cgiExtension.size(), loc.cgiExtension) == 0)
    {
        handleCgiOutput(fd, executeCgi(path, "/usr/bin/python3", *req));
        return;
    }

    if (loc.uploadPath.empty())
    {
        std::string errorBody = getErrorPageContent(SERVER_ERROR);
        m_response = response.status(SERVER_ERROR)
            .httpVersion(HTTP_VERSION)
            .header("Content-Type", "text/html")
            .header("Connection", req->shouldClose() ? "close" : "keep-alive")
            .body(errorBody)
            .build();
        return;
    }

    std::string body = req->getBody();
    if (body.empty())
    {
        std::string errorBody = getErrorPageContent(BAD_REQ);
        m_response = response.status(BAD_REQ)
            .httpVersion(HTTP_VERSION)
            .header("Content-Type", "text/html")
            .header("Connection", req->shouldClose() ? "close" : "keep-alive")
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
            .httpVersion(HTTP_VERSION)
            .header("Content-Type", "text/html")
            .header("Connection", req->shouldClose() ? "close" : "keep-alive")
            .body(errorBody)
            .build();
        return;
    }

    file << body;
    file.close();

    m_response = response.status(CREATED)
        .httpVersion(HTTP_VERSION)
        .header("Connection", req->shouldClose() ? "close" : "keep-alive")
        .body("201 Created: File uploaded successfully.\n")
        .build();
}

void Server::handleDeleteRequest(const Location& loc, int fd)
{
    LOG("Started Handling DELETE Request");
    Response response;
    ConnectionState& state = m_connections[fd];
    Request* req = state.req;

    std::string uri = req->getPath();
    std::string relativePath = uri.substr(loc.locUrl.length());
    std::string filePath = joinPaths("." + loc.rootPath, relativePath);

    if (access(filePath.c_str(), F_OK) != 0)
    {
        std::string errorBody = getErrorPageContent(NOT_FOUND);
        m_response = response.status(NOT_FOUND)
            .httpVersion(HTTP_VERSION)
            .header("Content-Type", "text/html")
            .header("Connection", req->shouldClose() ? "close" : "keep-alive")
            .body(errorBody)
            .build();
        return;
    }

    if (access(filePath.c_str(), W_OK) != 0)
    {
        std::string errorBody = getErrorPageContent(FORBIDDEN);
        m_response = response.status(FORBIDDEN)
            .httpVersion(HTTP_VERSION)
            .header("Content-Type", "text/html")
            .header("Connection", req->shouldClose() ? "close" : "keep-alive")
            .body(errorBody)
            .build();
        return;
    }

    if (std::remove(filePath.c_str()) != 0)
    {
        m_response = response.status(INT_SERV_ERR)
            .httpVersion(HTTP_VERSION)
            .header("Connection", req->shouldClose() ? "close" : "keep-alive")
            .build();
        return;
    }

    m_response = response.status(OK)
        .httpVersion(HTTP_VERSION)
        .header("Connection", req->shouldClose() ? "close" : "keep-alive")
        .body("200 OK: File deleted successfully.\n")
        .build();
}

void Server::handleInvalidRequest(int fd)
{
    Response response;
    ConnectionState& state = m_connections[fd];
    Request* req = state.req;

    std::string errorBody = getErrorPageContent(METH_NOT_ALLOW);
    m_response = response.httpVersion(HTTP_VERSION)
                     .status(METH_NOT_ALLOW)
                     .header("Content-Type", "text/html")
                     .header("Connection", req->shouldClose() ? "close" : "keep-alive")
                     .body(errorBody)
                     .build();
}

const Server::Location* Server::matchLocation(const std::string& uri) const
{
	const Location* bestMatch = NULL;
	size_t bestLength		  = 0;

	for (std::vector< Location >::const_iterator it = m_locations.begin();
		 it != m_locations.end(); ++it)
	{
		const std::string& locPath = it->locUrl;
		if (uri.find(locPath) == 0)
        {
            if (locPath == "/" || uri.length() == locPath.length()
                || uri[locPath.length()] == '/')
            {
                if (locPath.length() > bestLength)
                {
                    bestMatch  = &(*it);
                    bestLength = locPath.length();
                }
            }
        }
	}
	return bestMatch;
}

void Server::handleRequestTypes(int fd)
{
    ConnectionState& state = m_connections[fd];
    Request* req = state.req;

	if (!req)
    {
        closeConnection(fd);
        return;
    }

    if (req->getBadRequest())
    {
        std::string errorBody = getErrorPageContent(BAD_REQ);
        m_response = Response()
                         .httpVersion(HTTP_VERSION)
                         .status(BAD_REQ)
                         .header("Content-Type", "text/html")
                         .header("Connection", req->shouldClose() ? "close" : "keep-alive")
                         .body(errorBody)
                         .build();
        return;
    }

    if (req->getBodyLenght() > m_clientMaxBodySize)
    {
        m_response = Response()
                         .httpVersion(HTTP_VERSION)
                         .status(ENTITY_TOO_LARGE)
                         .header("Connection", req->shouldClose() ? "close" : "keep-alive")
                         .build();
        return;
    }

    const Location* loc = matchLocation(req->getPath());

    if (!loc)
    {
        std::string errorBody = getErrorPageContent(NOT_FOUND);
        m_response = Response()
                         .httpVersion(HTTP_VERSION)
                         .status(NOT_FOUND)
                         .header("Content-Type", "text/html")
                         .header("Connection", req->shouldClose() ? "close" : "keep-alive")
                         .body(errorBody)
                         .build();
        return;
    }

    if (std::find(loc->allowedMethods.begin(), loc->allowedMethods.end(),
                  req->getMethod()) == loc->allowedMethods.end())
    {
        handleInvalidRequest(fd);
        return;
    }

    if (loc->hasRedirect)
    {
        std::stringstream response;
        response << "HTTP/1.1 " << loc->redirectCode << " Moved\r\n";
        response << "Location: " << loc->redirectTo << "\r\n";
        response << "Connection: " << (req->shouldClose() ? "close" : "keep-alive") << "\r\n";
        response << "Content-Length: 0\r\n\r\n";
        m_response = response.str();
        return;
    }

	switch (req->getMethod())
	{
		case GET:
			handleGetRequest(*loc, fd);
			break;
		case POST:
			handlePostRequest(*loc, fd);
			break;
		case DELETE:
			handleDeleteRequest(*loc, fd);
			break;
		default:
			handleInvalidRequest(fd);
			break;
	}

}

std::string Server::getServerName() const { return m_serverName; }

std::vector< std::pair< std::string, std::string > > Server::getListens() const { return m_listens; }
void Server::setListenerFds(int fd, bool value)
{
	for (int i = 0; i < (int)listenerFds.size(); i++)
	{
		if (listenerFds[i].first == fd)
		{
			listenerFds[i].second = value;
			break ;
		}
	}
}
std::map<int, Server::ConnectionState>& Server::getConnections()
{
	return m_connections;
}

std::vector< struct pollfd >& Server::getPollFds() { return pollFds; }

Server::Location::Location()
{
	locUrl		  = "/";
	rootPath	  = "";
	indexPath	  = "index.html";
	autoIndex	  = false;
	hasRedirect	  = false;
	uploadEnabled = false;
}
