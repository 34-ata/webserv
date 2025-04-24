#include "request.hpp"

Request::Request()
{
	method			= "GET";
	path			= "about.html";
	headers["Host"] = "localhost";
}
