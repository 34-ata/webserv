#include "SyntaxException.hpp"
#include <sstream>

SyntaxException::SyntaxException(const ConfigBlock& block,
								 const ConfigDirective& directive,
								 const std::string& errorMsg)
{
	std::stringstream str;
	str << "Syntax Error: " << errorMsg << " at: " << block.name
		<< ", directive " << directive.directiveName << std::endl;
}

SyntaxException::SyntaxException(const std::string& errorMsg)
{
	std::stringstream str;
	str << "Syntax Error: " << errorMsg;
	m_errorMsg = str.str();
}

SyntaxException::~SyntaxException() throw() {}

const char* SyntaxException::what() const throw() { return m_errorMsg.c_str(); }
