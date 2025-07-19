#include "Tokenizer.hpp"
#include "SyntaxException.hpp"
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <istream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

void print_indent(int count, int size)
{
	for (int i = 0; i < count * size; i++)
		std::cout << ' ';
}

void Tokenizer::PrintTokens() const
{
	int indent_level = 0;

	for (std::list<std::string>::const_iterator i = m_tokens.begin();
		 i != m_tokens.end(); i++)
	{
		if (*i == "{")
			indent_level++;
		else if (*i == "}")
			indent_level--;
		print_indent(indent_level, 4);
		std::cout << "Token: " << *i << '\n';
	}
}

Tokenizer::Tokenizer(std::istream& input) : m_input(input), m_char(input.get())
{
	std::string nextToken;

	if (m_input.eof())
		throw SyntaxException("Empty File");
	while (!m_input.eof())
	{
		SkipWhiteSpaces();

		if (m_input.eof())
		{
			nextToken = "";
			break;
		}

		switch (m_char)
		{
		case ';':
			m_tokens.push_back(";");
			GetChar();
			break;
		case '{':
			m_tokens.push_back("{");
			GetChar();
			break;
		case '}':
			m_tokens.push_back("}");
			GetChar();
			break;
		default:
			std::string nextText;
			while (!m_input.eof() && !std::isspace(m_char) && m_char != ';'
				   && m_char != '{' && m_char != '}' && m_char != '#')
			{
				nextText += m_char;
				GetChar();
			}
			if (m_char == '#')
			{
				while (m_char != '\n')
					GetChar();
				continue;
			}
			m_tokens.push_back(nextText);
		}
	}
}

Tokenizer::~Tokenizer() {}

void Tokenizer::SkipWhiteSpaces()
{
	while (std::isspace(m_char))
		GetChar();
}

void Tokenizer::GetChar() { m_char = m_input.get(); }

const std::list<std::string>& Tokenizer::GetTokens() const { return m_tokens; }
