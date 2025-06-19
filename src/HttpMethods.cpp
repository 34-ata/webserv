#include "HttpMethods.hpp"

HttpMethods strToMethod(const std::string& str)
{
	if (str == "GET")
		return GET;
	if (str == "POST")
		return POST;
	if (str == "DELETE")
		return DELETE;
	return INVALID;
}
