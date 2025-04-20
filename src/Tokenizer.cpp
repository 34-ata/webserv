#include "Tokenizer.hpp"
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
		else
		{
			print_indent(indent_level, 4);
			std::cout << i->text << '\n';
		}
	}
}

Tokenizer::Tokenizer(std::istream& input)
	: m_input(input), m_row(0), m_col(0), m_char(input.get())
{
	Token nextToken;

	if (m_char == EOF)
		throw Tokenizer::SyntaxException(*this, "Empty File");
	nextToken = Next();
	while (nextToken.type != Token::END)
	{
		m_tokens.push_back(nextToken);
		nextToken = Next();
		PrintTokens();
	}
	PrintTokens();
}

Tokenizer::~Tokenizer() {}

Token Tokenizer::Next()
{
	SkipWhiteSpaces();

	if (m_input.eof())
		return ((Token){
			.type = Token::END, .text = "", .row = m_row, .col = m_col});

	std::string word = "";

	while (!std::isspace(m_char) && m_char != '{' && m_char != '}'
		   && m_char != ';' && !m_input.eof())
	{
		word += m_char;
		GetChar();
	}

	return (Token){StringToType(word), word, m_row, m_col};
}

Token::Type Tokenizer::StringToType(const std::string& string)
{
	if (string == "server")
		return Token::SERVER;
	if (string == "location")
		return Token::LOCATION;
	if (string == "host")
		return Token::HOST;
	if (string == "server_name")
		return Token::SERVER_NAME;
	if (string == "port")
		return Token::PORT;
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

std::size_t Tokenizer::GetCol() const { return m_col; }

std::size_t Tokenizer::GetRow() const { return m_row; }

Tokenizer::SyntaxException::SyntaxException(const Tokenizer& tokenizer,
											const std::string& errorMsg)
{
	std::stringstream str;
	str << "Syntax Error: " << errorMsg << " at: Line: " << tokenizer.GetRow()
		<< " Row: " << tokenizer.GetCol();
	m_errorMsg = str.str();
}

Tokenizer::SyntaxException::~SyntaxException() throw() {}

const char* Tokenizer::SyntaxException::what() const throw()
{
	return m_errorMsg.c_str();
}
