#ifndef SYNTAXEXCEPTION_HPP
#define SYNTAXEXCEPTION_HPP

#include "Tokenizer.hpp"
#include <exception>
#include <iostream>
#include <string>

#define UNEXPECTED(strGet, strExpect)                                                \
	"Unexpected Token [" + strGet + "] expect [" + strExpect + "]"

class SyntaxException : public std::exception
{
  public:
	SyntaxException(const std::string& token);
	SyntaxException(const ConfigBlock& block, const ConfigDirective& directive,
					const std::string& errorMsg);
	virtual ~SyntaxException() throw();
	virtual const char* what() const throw();

  private:
	std::string m_errorMsg;
};

#endif
