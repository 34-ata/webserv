#include "WebServer.hpp"
#include "Server.hpp"
#include "SyntaxException.hpp"
#include "Tokenizer.hpp"
#include <bits/types/locale_t.h>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

WebServer::WebServer()
{
	m_root.parent = NULL;
	m_root.name	  = "root";
}

WebServer::WebServer(const WebServer& other) { *this = other; }

WebServer::~WebServer() {}

WebServer& WebServer::operator=(const WebServer& other)
{
	(void)other;
	return *this;
}

bool WebServer::Init(const std::string& configFile)
{
	std::ifstream fileIn;
	Server::ServerConfig conf;

	fileIn.open(configFile.c_str());
	if (!fileIn.is_open())
		return false;
	try
	{
		Parse(fileIn);
	}
	catch (const SyntaxException& e)
	{
		std::cerr << e.what() << std::endl;
		fileIn.close();
		return false;
	}
	fileIn.close();

	conf.m_listens.clear();
	conf.m_serverName.clear();
	conf.m_listens.push_back(8080);
	conf.m_listens.push_back(4242);
	conf.m_listens.push_back(3000);
	conf.m_serverName = "boo.com";
	conf.m_isRunning = false;

	Server serv(conf);
	serv.Start();
	serv.Run();
	return true;
}

void ParseLocation(ConfigBlock* server, std::string locationPath,
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

void WebServer::ParseServer(std::list<std::string>::const_iterator start,
							std::list<std::string>::const_iterator end)
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

void CheckScopes(const Tokenizer& tokenizer)
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
