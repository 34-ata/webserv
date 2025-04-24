#include "WebServer.hpp"
#include "Server.hpp"
#include "Tokenizer.hpp"
#include <fstream>
#include <iostream>
#include <list>
#include <string>

WebServer::WebServer() {}

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

	fileIn.open(configFile.c_str());
	if (!fileIn.is_open())
		return false;

	try
	{
		Parse(fileIn);
	}
	catch (const Tokenizer::SyntaxException& e)
	{
		std::cerr << e.what() << std::endl;
		fileIn.close();
		return false;
	}
	fileIn.close();
	return true;
}

#define UNEXPECTED(get, expect)                                                \
	"Unexpected Token" + Tokenizer::TypeToString(get->type) + " expect "       \
		+ Tokenizer::TypeToString(expect)

void WebServer::Parse(std::ifstream& fileIn)
{
	Tokenizer tokenizer(fileIn);
	const std::list<Token>& tokens = tokenizer.GetTokens();
	Server::ServerConfig conf;
	for (std::list<Token>::const_iterator it = tokens.begin();
		 it != tokens.end(); it++)
	{
		if (it->type == Token::SERVER)
		{
			it++;
			if (it->type != Token::LBRACE)
				throw Tokenizer::SyntaxException(*it,
												 UNEXPECTED(it, Token::LBRACE));
			while (it->type != Token::RBRACE && it != tokens.end())
			{
				it++;
				switch (it->type)
				{
				case Token::LISTEN:
					it++;
					if (it->type != Token::VALUE)
					{
						throw Tokenizer::SyntaxException(
							*it, UNEXPECTED(it, Token::VALUE));
					}
				case Token::SERVER_NAME:
					it++;
					if (it->type != Token::VALUE) {
						throw Tokenizer::SyntaxException(
							*it, UNEXPECTED(it, Token::VALUE));
					}
				default:
					return;
				}
			}
		}
	}
	m_servers.push_back(conf);
}
