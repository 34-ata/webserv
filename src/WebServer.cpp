#include "WebServer.hpp"
#include "Server.hpp"
#include "Tokenizer.hpp"
#include <fstream>
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

	Parse(fileIn);

	return true;
}

void WebServer::Parse(std::ifstream& fileIn)
{
	Tokenizer tokenizer(fileIn);
	Server temp;
	Token currToken;

	currToken = tokenizer.Next();
	while (currToken.type != Token::TOKEN_EOF)
	{
		currToken = tokenizer.Next();
		if (currToken.type == Token::TOKEN_SEMICOLON)
			throw Tokenizer::SyntaxException(tokenizer);
	}
}

