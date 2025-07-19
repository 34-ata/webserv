#ifndef HTTPMETHODS_HPP
#define HTTPMETHODS_HPP

#include <string>

enum HttpMethods
{
	INVALID = -1,
	GET,
	POST,
	DELETE,
};
HttpMethods strToMethod(const std::string& str);
#endif
