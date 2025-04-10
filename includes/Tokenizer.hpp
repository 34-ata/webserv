#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <cstddef>
#include <list>
#include <string>

struct Token
{
	enum Type
	{
		TOKEN_LBRACE,
		TOKEN_RBRACE,
		TOKEN_SERVER,
		TOKEN_LOCATION,
		TOKEN_SEMICOLON,
		TOKEN_EOF
	};
	const Type type;
	const std::size_t pos;
	const std::string text;
};

class Tokenizer
{
  public:
	Tokenizer();
	Tokenizer(const Tokenizer&);
	Tokenizer& operator=(const Tokenizer&);
	~Tokenizer();

  private:
	std::string text;
	std::list<char> tok;
};

Tokenizer::Tokenizer() {}

Tokenizer::~Tokenizer() {}

#endif // !TOKENIZER_HPP
