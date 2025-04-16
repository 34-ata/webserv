#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <istream>
#include <string>

struct Token
{
	enum Type
	{
		TOKEN_LBRACE,
		TOKEN_RBRACE,
		TOKEN_SEMICOLON,
		TOKEN_STRING,
		TOKEN_EOF
	};
	Type type;
	std::string text;
};

class Tokenizer
{
  public:
	class SyntaxException : std::exception
	{
	  public:
		SyntaxException(const Tokenizer& tokenizer);
		virtual ~SyntaxException() throw();
		virtual const char* what() const throw();

	  private:
		std::string m_errorMsg;
	};

  public:
	Tokenizer(std::istream& input);
	~Tokenizer();

  public:
	Token Next();
	void Expect(const Token::Type type);
	std::size_t GetCol() const;
	std::size_t GetRow() const;

  private:
	std::istream& m_input;
	std::size_t m_col;
	std::size_t m_row;
	char m_char;

  private:
	Tokenizer();
	Tokenizer(const Tokenizer& other);

  private:
	void SkipWhiteSpaces();
	void GetChar();
};

#endif // !TOKENIZER_HPP
