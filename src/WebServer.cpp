#include "WebServer.hpp"
#include "HttpMethods.hpp"
#include "ResponseCodes.hpp"
#include "Server.hpp"
#include "SyntaxException.hpp"
#include "Tokenizer.hpp"
#include <bits/types/locale_t.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <utility>
#include <vector>

WebServer::WebServer()
{
	m_root.parent = NULL;
	m_root.name	  = "root";
}

WebServer::WebServer(const WebServer& other) { *this = other; }

WebServer::~WebServer()
{
	for (size_t i = 0; i < m_servers.size(); ++i)
		delete m_servers[i];
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
				loc.allowedMethods.push_back(
					(HttpMethods)std::atoi(directive_args[j].c_str()));
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
				throw SyntaxException(location, directives[i], "Invalid usage of return directive.");

			loc.hasRedirect = true;
			loc.redirectCode = std::atoi(directive_args[0].c_str());
			loc.redirectTo = directive_args[1];

			if (loc.redirectCode < 300 || loc.redirectCode >= 400)
				throw SyntaxException(location, directives[i], "Return code must be a 3xx redirect code.");
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
				throw SyntaxException(location, directives[i],
									"Invalid usage of upload_store directive.");
			loc.uploadEnabled = true;
			loc.uploadPath = directive_args[0];
		}
	}
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
				directive_args[0]; // 1 isim alıyor sonra değişecek.
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

	/*Server::Location loc1;
	loc1.locUrl	   = "/images";
	loc1.rootPath  = "./www/images";
	loc1.indexPath = "index.html";
	loc1.autoIndex = true;
	loc1.allowedMethods.push_back(GET);
	loc1.allowedMethods.push_back(POST);
	loc1.allowedMethods.push_back(DELETE);
	loc1.hasRedirect   = false;
	loc1.uploadEnabled = false;

	Server::Location loc2;
	loc2.locUrl	   = "/old-page";
	loc2.rootPath  = "";
	loc2.indexPath = "";
	loc2.autoIndex = false;
	loc2.allowedMethods.push_back(GET);
	loc2.hasRedirect   = true;
	loc2.redirectTo	   = "/new-page";
	loc2.redirectCode  = 301;
	loc2.uploadEnabled = false;

	Server::Location loc3;
	loc3.locUrl	   = "/upload";
	loc3.rootPath  = "./www/uploads";
	loc3.indexPath = "";
	loc3.autoIndex = false;
	loc3.allowedMethods.push_back(POST);
	loc3.uploadEnabled = true;
	loc3.uploadPath	   = "./www/uploads";
	loc3.hasRedirect   = false;

	Server::Location loc4;
	loc4.locUrl	   = "/cgi-bin";
	loc4.rootPath  = "./www/cgi-bin";
	loc4.indexPath = "";
	loc4.autoIndex = false;
	loc4.allowedMethods.push_back(GET);
	loc4.cgiExtension	   = ".py";
	loc4.cgiExecutablePath = "/usr/bin/python3";
	loc4.hasRedirect	   = false;
	loc4.uploadEnabled	   = false;

	Server::ServerConfig conf1;
	conf1.locations.push_back(loc1);
	conf1.locations.push_back(loc2);
	conf1.locations.push_back(loc3);
	conf1.locations.push_back(loc4);
	conf1.serverName = "loopback_server";
	conf1.listens.push_back("127.0.0.1:8080");

	Server::ServerConfig conf2;
	conf2.locations.push_back(loc1);
	conf2.locations.push_back(loc2);
	conf2.locations.push_back(loc3);
	conf2.locations.push_back(loc4);
	conf2.serverName = "anyip_server";
	conf2.listens.push_back("0.0.0.0:9080");

	Server::ServerConfig conf3;
	conf3.locations.push_back(loc1);
	conf3.locations.push_back(loc2);
	conf3.locations.push_back(loc3);
	conf3.locations.push_back(loc4);
	conf3.serverName = "localnet_server";
	conf3.listens.push_back("10.11.3.2:4242");*/

	for (size_t i = 0; i < m_servers.size(); ++i)
		m_servers[i]->start();

	return true;
}

void WebServer::Run()
{
	while (true)
	{
		std::vector< struct pollfd > fds;

		for (size_t i = 0; i < m_servers.size(); ++i)
		{
			const std::vector< struct pollfd >& serverFds =
				m_servers[i]->getPollFds();
			fds.insert(fds.end(), serverFds.begin(), serverFds.end());
		}

		if (poll(&fds[0], fds.size(), -1) < 0)
		{
			perror("poll");
			break;
		}

		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & POLLIN)
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

				Server* matched = findMatchingServer(ip, port, m_servers);
				if (matched)
					matched->handleEvent(fds[i].fd);
			}
		}
	}
}

static void ParseLocation(ConfigBlock* server, std::string locationPath,
						  Tokenizer::const_iterator start,
						  Tokenizer::const_iterator end)
{
	ConfigDirective temp;
	ConfigBlock* location = new ConfigBlock();
	server->childs.push_back(location);

	location->name = "location";
	location->args.push_back(locationPath);
	for (; start != end; start++)
	{
		temp.directiveName = *start;
		temp.args.clear();
		start++;
		while (*start != ";")
		{
			temp.args.push_back(*start);
			start++;
		}
		location->directives.push_back(temp);
	}
}

void WebServer::ParseServer(std::list< std::string >::const_iterator start,
							std::list< std::string >::const_iterator end)
{
	ConfigBlock* server = new ConfigBlock();
	ConfigDirective temp;
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
			temp.directiveName = *start;
			temp.args.clear();
			start++;
			while (*start != ";")
			{
				temp.args.push_back(*start);
				start++;
			}
			server->directives.push_back(temp);
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
