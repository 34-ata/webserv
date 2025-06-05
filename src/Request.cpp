#include "Request.hpp"
#include <sstream>
#include <cstdlib>

Request::Request() : badRequest(false) {}

Request::~Request() {}

Request::Request(const Request& other) : data(other.data), badRequest(other.badRequest) {}

Request& Request::operator=(const Request& other)
{
	if (this != &other)
	{
		data = other.data;
		badRequest = other.badRequest;
	}
	return *this;
}

void Request::fillRequest(const std::string& buffer)
{
	data.append(buffer);
}

void Request::checkIntegrity()
{
	if (getMethod() == INVALID || getPath().empty() || getVersion() != "HTTP/1.1")
		badRequest = true;
}

HttpMethods Request::getMethod() const
{
	std::string method = data.substr(0, data.find(" "));
	if (method == "GET") return GET;
	if (method == "POST") return POST;
	if (method == "DELETE") return DELETE;
	return INVALID;
}

std::string Request::getPath() const
{
	size_t first = data.find(" ");
	size_t second = data.find(" ", first + 1);
	return data.substr(first + 1, second - first - 1);
}

std::string Request::getVersion() const
{
	return data.substr(data.find_last_of(" "), data.find("\r\n") - data.find_last_of(" "));
}

const std::string& Request::getData() const
{
	return data;
}

std::string Request::getBody() const
{
	std::size_t endOfHeaders = data.find("\r\n\r\n");
	if (endOfHeaders != std::string::npos)
		return data.substr(endOfHeaders + 4);
	return "";
}


size_t Request::getBodyLenght() const
{
	std::string header = "Content-Length:";
	std::size_t pos = data.find(header);
	if (pos != std::string::npos)
		return std::atoi(data.c_str() + pos + header.length());
	return 0;
}

void Request::setBadRequest()
{
	badRequest = true;
}

bool Request::getBadRequest() const
{
	return badRequest;
}
