#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <istream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

struct ConfigDirective
{
	std::string directiveName;
	std::vector< std::string > args;
};

struct ConfigBlock
{
	std::string name;
	std::vector< std::string > args;
	std::vector< ConfigBlock* > childs;
	std::vector< ConfigDirective > directives;
	ConfigBlock* parent;

	~ConfigBlock()
	{
		for (std::size_t i = 0; i < this->childs.size(); i++)
			delete this->childs[i];
	}
};

class Tokenizer
{
  public:
	typedef std::list< std::string >::const_iterator const_iterator;

  public:
	Tokenizer(std::istream& input);
	~Tokenizer();

  public:
	std::string Next();
	void PrintTokens() const;
	const std::list< std::string >& GetTokens() const;

  private:
	std::istream& m_input;
	std::list< std::string > m_tokens;
	char m_char;

  private:
	Tokenizer();
	Tokenizer(const Tokenizer& other);

  private:
	void SkipWhiteSpaces();
	void GetChar();
};

#endif // !TOKENIZER_HPP
