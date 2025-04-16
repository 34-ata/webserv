#include "Tokenizer.hpp"
#include <string>
#include <sstream>

Tokenizer::Tokenizer(std::istream& input)
	: m_input(input), m_col(0), m_row(0), m_char(m_input.get())
{
}

Tokenizer::~Tokenizer() {}

Token Tokenizer::Next()
{
	SkipWhiteSpaces();

	if (m_input.eof())
		return ((Token){.type = Token::TOKEN_EOF, .text = ""});

	switch (m_char)
	{
	case '{':
		GetChar();
		return ((Token){.type = Token::TOKEN_LBRACE, .text = "{"});
		break;
	case '}':
		GetChar();
		return ((Token){.type = Token::TOKEN_RBRACE, .text = "}"});
		break;
	case ';':
		GetChar();
		return ((Token){.type = Token::TOKEN_SEMICOLON, .text = ";"});
		break;
	}

	std::string word = "";

	while (!std::isspace(m_char) && m_char != '{' && m_char != '}'
		   && m_char != ';' && !m_input.eof())
	{
		word += m_char;
		GetChar();
	}

	return ((Token){.type = Token::TOKEN_STRING, .text = word});
}

void Tokenizer::Expect(const Token::Type type)
{
	if (this->m_input.eof() || this->Next().type != type)
	{
		throw Tokenizer::SyntaxException(*this);
	}
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

Tokenizer::SyntaxException::SyntaxException(const Tokenizer& tokenizer)
{
	std::stringstream str;
	str << "Syntax Error at: Line: " << tokenizer.GetRow() << " Row: " << tokenizer.GetCol();
	m_errorMsg = str.str();
}

Tokenizer::SyntaxException::~SyntaxException() throw()
{}

const char* Tokenizer::SyntaxException::what() const throw()
{
	return m_errorMsg.c_str();
}

