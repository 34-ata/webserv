#include "Tokenizer.hpp"
#include <cctype>
#include <iostream>
#include <istream>
#include <iterator>
#include <list>
#include <sstream>
#include <string>

void print_indent(int count, int size)
{
	for (int i = 0; i < count * size; i++)
		std::cout << ' ';
}

void Tokenizer::PrintTokens() const
{
	int indent_level = 0;

	for (std::list<Token>::const_iterator i = m_tokens.begin();
		 i != m_tokens.end(); i++)
	{
		if (i->type == Token::LBRACE)
			indent_level++;
		else if (i->type == Token::RBRACE)
			indent_level--;
		print_indent(indent_level, 4);
		std::cout << TypeToString(i->type) << ": " << i->text << '\n';
	}
}

Tokenizer::Tokenizer(std::istream& input)
	: m_input(input), m_row(0), m_col(0), m_char(input.get())
{
	Token nextToken;

	if (m_input.eof())
		throw Tokenizer::SyntaxException(
			(Token){.type = Token::END, .text = "", .row = m_row, .col = m_col},
			"Empty File");
	while (!m_input.eof())
	{
		SkipWhiteSpaces();

		if (m_input.eof())
		{
			nextToken = ((Token){
				.type = Token::END, .text = "", .row = m_row, .col = m_col});
			break;
		}

		switch (m_char)
		{
		case ';':
			m_tokens.push_back((Token){.type = Token::SEMICOLON,
									   .text = ";",
									   .row	 = m_row,
									   .col	 = m_col});
			GetChar();
			break;
		case '{':
			m_tokens.push_back((Token){.type = Token::LBRACE,
									   .text = "{",
									   .row	 = m_row,
									   .col	 = m_col});
			GetChar();
			break;
		case '}':
			m_tokens.push_back((Token){.type = Token::RBRACE,
									   .text = "}",
									   .row	 = m_row,
									   .col	 = m_col});
			GetChar();
			break;
		default:
			std::string nextText;
			while (!m_input.eof() && !std::isspace(m_char) && m_char != ';'
				   && m_char != '{' && m_char != '}')
			{
				nextText += m_char;
				GetChar();
			}
			m_tokens.push_back((Token){.type = StringToType(nextText),
									   .text = nextText,
									   .row	 = m_row,
									   .col	 = m_col - nextText.length()});
		}
	}
	//PrintTokens();
}

Tokenizer::~Tokenizer() {}

std::string Tokenizer::TypeToString(Token::Type type)
{
	if (type == Token::SERVER)
		return "Token::SERVER";
	if (type == Token::LOCATION)
		return "Token::LOCATION";
	if (type == Token::SERVER_NAME)
		return "Token::SERVER_NAME";
	if (type == Token::LISTEN)
		return "Token::LISTEN";
	if (type == Token::ERROR_PAGE)
		return "Token::ERROR_PAGE";
	if (type == Token::CLIENT_MAX_BODY)
		return "Token::CLIENT_MAX_BODY";
	if (type == Token::ROOT)
		return "Token::ROOT";
	if (type == Token::INDEX)
		return "Token::INDEX";
	if (type == Token::AUTO_INDEX)
		return "Token::AUTO_INDEX";
	if (type == Token::METHODS)
		return "Token::METHODS";
	if (type == Token::RETURN)
		return "Token::RETURN";
	if (type == Token::UPLOAD_STORE)
		return "Token::UPLOAD_STORE";
	if (type == Token::CGI_EXTENSION)
		return "Token::CGI_EXTENSION";
	if (type == Token::CGI_PATH)
		return "Token::CGI_PATH";
	if (type == Token::SEMICOLON)
		return "Token::SEMICOLON";
	if (type == Token::LBRACE)
		return "Token::LBRACE";
	if (type == Token::RBRACE)
		return "Token::RBRACE";
	return "Token::VALUE";
}

Token::Type Tokenizer::StringToType(const std::string& string)
{
	if (string == "server")
		return Token::SERVER;
	if (string == "location")
		return Token::LOCATION;
	if (string == "server_name")
		return Token::SERVER_NAME;
	if (string == "listen")
		return Token::LISTEN;
	if (string == "error_page")
		return Token::ERROR_PAGE;
	if (string == "client_max_body_size")
		return Token::CLIENT_MAX_BODY;
	if (string == "root")
		return Token::ROOT;
	if (string == "index")
		return Token::INDEX;
	if (string == "auto_index")
		return Token::AUTO_INDEX;
	if (string == "methods")
		return Token::METHODS;
	if (string == "return")
		return Token::RETURN;
	if (string == "upload_store")
		return Token::UPLOAD_STORE;
	if (string == "cgi_extension")
		return Token::CGI_EXTENSION;
	if (string == "cgi_path")
		return Token::CGI_PATH;
	return Token::VALUE;
}

void Tokenizer::SkipWhiteSpaces()
{
	while (std::isspace(m_char))
	{
		if (m_char == '\n')
		{
			m_col = 0;
			m_row++;
		}
		GetChar();
	}
}

void Tokenizer::GetChar()
{
	m_char = m_input.get();
	m_col++;
}

const std::list<Token>& Tokenizer::GetTokens() const { return m_tokens; }

std::size_t Tokenizer::GetCol() const { return m_col; }

std::size_t Tokenizer::GetRow() const { return m_row; }

Tokenizer::SyntaxException::SyntaxException(const Token& token,
											const std::string& errorMsg)
{
	std::stringstream str;
	str << "Syntax Error: " << errorMsg << " at: Line: " << token.row
		<< " Row: " << token.col;
	m_errorMsg = str.str();
}

Tokenizer::SyntaxException::~SyntaxException() throw() {}

const char* Tokenizer::SyntaxException::what() const throw()
{
	return m_errorMsg.c_str();
}
