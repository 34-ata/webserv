#include "request.hpp"

Request::Request()
{
	method			= "POST";
	path			= "upload";
	body			= "denemedenemedeneme";
	headers["Host"] = "localhost";
}
