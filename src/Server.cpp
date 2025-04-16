#include "Server.hpp"
#include "Tokenizer.hpp"
#include <limits>

Server::Server(Tokenizer& tokenizer)
{
	Location temp;
	bool innerBrace = false;
	Token currToken = tokenizer.Next();

	if (currToken.text == "location")
	{
		currToken = tokenizer.Next();
		if (currToken.type == Token::TOKEN_STRING)
			temp.locUrl = currToken.text;
		currToken = tokenizer.Next();
		if (currToken.type == Token::TOKEN_LBRACE)
			innerBrace = true;
		currToken = tokenizer.Next();
		if (currToken.type == Token::TOKEN_STRING)
			m_locations.push_back(temp);
	}
	(void)innerBrace;
}

Server::Server()
	: m_host("127.0.0.1"), m_serverName("Test"), m_port(8080),
	  m_clientMaxBodySize(std::numeric_limits<int>::max())
{
	m_errorPages[404] = "Custom-404-url";
}

Server::~Server() {}
