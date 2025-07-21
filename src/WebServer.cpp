#include "WebServer.hpp"
#include "HttpMethods.hpp"
#include "Log.hpp"
#include "ResponseCodes.hpp"
#include "Server.hpp"
#include "SyntaxException.hpp"
#include "Tokenizer.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <utility>
#include <vector>
#define TIMEOUT 15

volatile sig_atomic_t g_shutdown = 0;

void signalHandler(int signum)
{
	(void)signum;
	g_shutdown = 1;
}

WebServer::WebServer()
{
	m_root.parent = NULL;
	m_root.name	  = "root";
}

WebServer::WebServer(const WebServer& other) { *this = other; }

void deleteConfigBlock(ConfigBlock* block)
{
	for (size_t i = 0; i < block->childs.size(); ++i)
		deleteConfigBlock(block->childs[i]);

	delete block;
}

WebServer::~WebServer()
{
	if (!m_servers.empty() || !m_root.childs.empty())
	{
		for (size_t i = 0; i < m_servers.size(); ++i)
			delete m_servers[i];
		
		for (size_t i = 0; i < m_root.childs.size(); ++i)
			deleteConfigBlock(m_root.childs[i]);
	}
}

WebServer& WebServer::operator=(const WebServer& other)
{
	(void)other;
	return *this;
}

bool isErrorCode(std::string code)
{
	ResponseCodes errors[] = {NOT_FOUND, INT_SERV_ERR};
	for (size_t i = 0; i < 2; i++)
	{
		if (std::atoi(code.c_str()) == errors[i])
			return true;
	}
	return false;
}

Server::Location WebServer::createLocation(const ConfigBlock& location)
{
	Server::Location loc;
	if (location.name == "location" && !location.args.empty())
		loc.locUrl = location.args[0];
	std::vector< ConfigDirective > directives = location.directives;
	for (size_t i = 0; i < directives.size(); i++)
	{
		std::string directive_name				  = directives[i].directiveName;
		std::vector< std::string > directive_args = directives[i].args;
		if (directive_name == "methods")
		{
			if (directive_args.size() < 1)
				throw SyntaxException(location, directives[i],
									  "Invalid usage of given directive.");
			for (size_t j = 0; j < directive_args.size(); j++)
			{
				loc.allowedMethods.push_back(
					(HttpMethods)strToMethod(directive_args[j].c_str()));
			}
		}
		else if (directive_name == "root")
		{
			if (directive_args.size() != 1)
				throw SyntaxException(location, directives[i],
									  "Invalid usage of given directive.");
			loc.rootPath = directive_args[0];
		}
		else if (directive_name == "index")
		{
			if (directive_args.size() != 1)
				throw SyntaxException(location, directives[i],
									  "Invalid usage of given directive.");
			loc.indexPath = directive_args[0];
		}
		else if (directive_name == "autoindex")
		{
			if (directive_args.size() != 1)
				throw SyntaxException(location, directives[i],
									  "Invalid usage of given directive.");
			if (directive_args[0] != "on" && directive_args[0] != "off")
				throw SyntaxException(location, directives[i],
									  "Invalid value of autoindex.");
			if (directive_args[0] == "on")
				loc.autoIndex = true;
			else if (directive_args[0] == "off")
				loc.autoIndex = false;
		}
		else if (directive_name == "return")
		{
			if (directive_args.size() != 2)
				throw SyntaxException(location, directives[i],
									  "Invalid usage of return directive.");

			loc.hasRedirect	 = true;
			loc.redirectCode = std::atoi(directive_args[0].c_str());
			loc.redirectTo	 = directive_args[1];

			if (loc.redirectCode < 300 || loc.redirectCode >= 400)
				throw SyntaxException(
					location, directives[i],
					"Return code must be a 3xx redirect code.");
		}
		else if (directive_name == "cgi_extension")
		{
			if (directive_args.size() != 1)
				throw SyntaxException(location, directives[i],
									  "Invalid usage of given directive.");
			loc.cgiExtension = directive_args[0];
		}
		else if (directive_name == "cgi_path")
		{
			if (directive_args.size() != 1)
				throw SyntaxException(location, directives[i],
									  "Invalid usage of given directive.");
			loc.cgiExecutablePath = directive_args[0];
		}
		else if (directive_name == "upload_store")
		{
			if (directive_args.size() != 1)
				throw SyntaxException(
					location, directives[i],
					"Invalid usage of upload_store directive.");
			loc.uploadEnabled = true;
			loc.uploadPath	  = directive_args[0];
		}
	}
	if (loc.allowedMethods.size() == 0)
		loc.allowedMethods.push_back(GET);
	return loc;
}

Server::ServerConfig WebServer::createServerConfig(const ConfigBlock& server)
{
	Server::ServerConfig config;
	std::vector< ConfigDirective > directives = server.directives;
	for (size_t i = 0; i < directives.size(); i++)
	{
		std::string directive_name				  = directives[i].directiveName;
		std::vector< std::string > directive_args = directives[i].args;
		if (directive_name == "server_name")
			config.serverName =
				directive_args[0];
		else if (directive_name == "error_page")
		{
			if (directive_args.size() != 2)
				throw SyntaxException(server, directives[i],
									  "Invalid usage of given directive.");
			if (!isErrorCode(directive_args[0]))
				throw SyntaxException(server, directives[i],
									  "Invalid error code");
			config.errorPages[(ResponseCodes)std::atoi(
				directive_args[0].c_str())] = directive_args[1];
		}
		else if (directive_name == "root")
		{
			if (directive_args.size() != 1)
				throw SyntaxException(server, directives[i],
									  "Invalid usage of given directive.");
			config.rootPath = directive_args[0];
		}
		else if (directive_name == "index")
		{
			if (directive_args.size() != 1)
				throw SyntaxException(server, directives[i],
									  "Invalid usage of given directive.");
			config.indexPath = directive_args[0];
		}
		else if (directive_name == "autoindex")
		{
			if (directive_args.size() != 1)
				throw SyntaxException(server, directives[i],
									  "Invalid usage of given directive.");
			if (directive_args[0] != "on" || directive_args[0] != "off")
				throw SyntaxException(server, directives[i],
									  "Invalid value of autoindex.");
			if (directive_args[0] == "on")
				config.autoIndex = true;
			else if (directive_args[0] == "off")
				config.autoIndex = false;
		}
		else if (directive_name == "client_max_body_size")
		{
			if (directive_args.size() != 1)
				throw SyntaxException(server, directives[i],
									  "Invalid usage of given directive.");
			config.clientMaxBodySize = std::atoi(directive_args[0].c_str());
			if (config.clientMaxBodySize == 0)
				throw SyntaxException(server, directives[i],
									  "Invalid size for body.");
		}
		else if (directive_name == "listen")
		{
			if (directive_args.size() < 1)
				throw SyntaxException(server, directives[i],
									  "Invalid usage of given directive.");
			config.listens.clear();
			for (size_t j = 0; j < directive_args.size(); j++)
				config.listens.push_back(directive_args[j]);
		}
	}

	std::vector< ConfigBlock* > locations = server.childs;
	for (size_t i = 0; i < locations.size(); i++)
		config.locations.push_back(createLocation(*locations[i]));

	return config;
}

bool WebServer::Init(const std::string& configFile)
{
	std::ifstream fileIn;
	fileIn.open(configFile.c_str());
	if (!fileIn.is_open())
		return false;

	try
	{
		Parse(fileIn);
		for (size_t i = 0; i < m_root.childs.size(); i++)
			m_servers.push_back(
				new Server(createServerConfig(*m_root.childs[i])));
	}
	catch (const SyntaxException& e)
	{
		std::cerr << e.what() << std::endl;
		fileIn.close();
		return false;
	}
	fileIn.close();

	for (size_t i = 0; i < m_servers.size(); ++i)
		m_servers[i]->start();

	return true;
}

std::string WebServer::parseHostHeader(int fd)
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	recv(fd, buffer, sizeof(buffer) - 1, MSG_PEEK);

	std::string data(buffer);
	size_t hostPos = data.find("Host:");
	if (hostPos == std::string::npos)
		return "";

	size_t start = hostPos + 5;
	while (start < data.size() && (data[start] == ' ' || data[start] == '\t'))
		++start;

	size_t end = data.find("\r\n", start);
	if (end == std::string::npos)
		return "";

	std::string host = data.substr(start, end - start);
	size_t colon = host.find(":");
	if (colon != std::string::npos)
		host = host.substr(0, colon);

	return host;
}

void WebServer::checkTimeouts()
{
	time_t now = time(NULL);

	for (size_t i = 0; i < m_servers.size(); ++i)
	{
		Server* server = m_servers[i];
		std::map<int, Server::ConnectionState>& conns = server->getConnections();
		std::vector<int> toClose;

		for (std::map<int, Server::ConnectionState>::iterator it = conns.begin(); it != conns.end(); ++it)
		{
			if (difftime(now, it->second.timeStamp) > TIMEOUT)
				toClose.push_back(it->first);
		}

		for (size_t j = 0; j < toClose.size(); ++j)
		{
			int fd = toClose[j];
			LOG("Timeout: closing fd " << fd);
			close(fd);
			server->removePollFd(fd);
			conns.erase(fd);
		}
	}
}

void WebServer::Run()
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGPIPE, SIG_IGN);
	
	while (!g_shutdown)
	{
		std::vector<struct pollfd> fds;

		for (size_t i = 0; i < m_servers.size(); ++i)
		{
			const std::vector<struct pollfd>& serverFds =
				m_servers[i]->getPollFds();
			fds.insert(fds.end(), serverFds.begin(), serverFds.end());
		}

		if (poll(&fds[0], fds.size(), 0) < 0)
		{
			perror("poll");
			break;
		}

		for (size_t i = 0; i < fds.size(); ++i)
		{
			struct sockaddr_in addr;
			socklen_t len = sizeof(addr);
			if (getsockname(fds[i].fd, (struct sockaddr*)&addr, &len) == -1)
			{
				perror("getsockname");
				continue;
			}

			char ip[INET_ADDRSTRLEN];
			if (!inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip)))
			{
				perror("inet_ntop");
				continue;
			}

			int port = ntohs(addr.sin_port);
			std::string host = parseHostHeader(fds[i].fd);
			Server* matched = findMatchingServer(ip, port, m_servers, host);
			if (!matched)
				continue;

			if (fds[i].revents & POLLIN)
				matched->handleReadEvent(fds[i].fd);

			if (fds[i].revents & POLLOUT)
				matched->handleWriteEvent(fds[i].fd);
		}

		checkTimeouts();
	}
}

void WebServer::Shutdown()
{
	for (size_t i = 0; i < m_servers.size(); ++i)
	{
		if (m_servers[i])
		{
			delete m_servers[i];
			m_servers[i] = NULL;
		}
	}
	m_servers.clear();
	
	for (size_t i = 0; i < m_root.childs.size(); ++i)
		deleteConfigBlock(m_root.childs[i]);
	m_root.childs.clear();
}

static void ParseLocation(ConfigBlock* server, std::string locationPath,
						  Tokenizer::const_iterator start,
						  Tokenizer::const_iterator end)
{
	ConfigBlock* location = new ConfigBlock();
	server->childs.push_back(location);

	location->name = "location";
	location->args.push_back(locationPath);
	for (; start != end; start++)
	{
		ConfigDirective directive;
		directive.directiveName = *start;
		start++;
		while (*start != ";")
		{
			directive.args.push_back(*start);
			start++;
		}
		location->directives.push_back(directive);
	}
}

void WebServer::ParseServer(std::list< std::string >::const_iterator start,
							std::list< std::string >::const_iterator end)
{
	ConfigBlock* server = new ConfigBlock();
	std::string locationPath;
	m_root.childs.push_back(server);
	server->name = "server";

	for (; start != end; start++)
	{
		if (*start == "location")
		{
			start++;
			if (*start == ";" || *start == "{" || *start == "}")
				throw SyntaxException(UNEXPECTED(*start, "VALUE"));
			locationPath = *start;
			start++;
			if (*start != "{")
				throw SyntaxException(UNEXPECTED(*start, "{"));
			start++;
			Tokenizer::const_iterator locationStart = start;
			for (; *start != "}"; start++)
				;
			ParseLocation(server, locationPath, locationStart, start);
		}
		else
		{
			ConfigDirective directive;
			directive.directiveName = *start;
			start++;
			while (*start != ";")
			{
				directive.args.push_back(*start);
				start++;
			}
			server->directives.push_back(directive);
		}
	}
}

static void CheckScopes(const Tokenizer& tokenizer)
{
	int indentLevel				 = 0;
	Tokenizer::const_iterator it = tokenizer.GetTokens().begin();
	for (; it != tokenizer.GetTokens().end(); it++)
	{
		if (*it == "{")
			indentLevel++;
		if (*it == "}")
			indentLevel--;
	}
	if (indentLevel != 0)
		throw SyntaxException("Unclosed Scope");
}

void WebServer::Parse(std::ifstream& fileIn)
{
	Tokenizer tokenizer(fileIn);
	CheckScopes(tokenizer);
	Tokenizer::const_iterator it = tokenizer.GetTokens().begin();
	while (it != tokenizer.GetTokens().end())
	{
		if (*it == "server")
		{
			int indentLevel = 1;
			it++;
			if (*it != "{")
				throw SyntaxException(UNEXPECTED(*it, "{"));
			it++;
			Tokenizer::const_iterator start = it;
			while (it != tokenizer.GetTokens().end())
			{
				if (*it == "{")
					indentLevel++;
				if (*it == "}")
					indentLevel--;
				if (indentLevel == 0)
					break;
				it++;
			}
			ParseServer(start, it);
		}
		else
			throw SyntaxException(UNEXPECTED(*it, "server"));
		if (it == tokenizer.GetTokens().end())
			throw SyntaxException("Unexpected EOF");
		it++;
	}
}
