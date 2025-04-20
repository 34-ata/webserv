#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <istream>
#include <list>
#include <string>

struct Token
{
	enum Type
	{
		LBRACE,
		RBRACE,
		SEMICOLON,
		SERVER,
		HOST,
		LOCATION,
		SERVER_NAME,
		PORT,
		ERROR_PAGE,
		CLIENT_MAX_BODY,
		ROOT,
		INDEX,
		AUTO_INDEX,
		METHODS,
		RETURN,
		UPLOAD_STORE,
		CGI_EXTENSION,
		CGI_PATH,
		VALUE,
		END
	};
	Type type;
	std::string text;
	std::size_t row;
	std::size_t col;
};

class Tokenizer
{
  public:
	class SyntaxException : std::exception
	{
	  public:
		SyntaxException(const Tokenizer& tokenizer, const std::string& errorMsg);
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
	std::size_t GetCol() const;
	std::size_t GetRow() const;
	void PrintTokens() const;

  private:
	std::istream& m_input;
	std::list<Token> m_tokens;
	std::size_t m_row;
	std::size_t m_col;
	char m_char;

  private:
	Tokenizer();
	Tokenizer(const Tokenizer& other);

  private:
	Token::Type StringToType(const std::string& string);
	void SkipWhiteSpaces();
	void GetChar();
};

#endif // !TOKENIZER_HPP
