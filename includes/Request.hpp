#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Server.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

class Request
{
  public:
	Request();
	Request(const Request&);
	Request& operator=(const Request&);
	~Request();

  public:
	void fillRequest(const std::string& buffer);
	void checkIntegrity();

  public:
	HttpMethods getMethod() const;
	std::string getPath() const;
	std::string getVersion() const;
	const std::string& getData() const;
	const std::string& getBody() const;
	size_t getBodyLenght() const;
	bool getBadRequest();
	void setBadRequest();

  private:
	std::string data;
	bool badRequest;
};

HttpMethods Request::getMethod() const
{
	std::string method = data.substr(0, data.find(" "));
	if (method == "GET")
		return GET;
	if (method == "POST")
		return POST;
	if (method == "DELETE")
		return DELETE;
	return INVALID;
}

std::string Request::getPath() const
{
	return data.substr(data.find(" ") + 1,
					   data.find(" ", data.find(" ") + 1) - data.find(" "));
}

std::string Request::getVersion() const
{
	return data.substr(data.find_last_of(" "), data.find("\r\n"));
}

const std::string& Request::getData() const { return data; }

size_t Request::getBodyLenght() const
{
	std::string contentLenght = "Content-Length";
	std::size_t pos			  = data.find(contentLenght);
	if (pos != std::string::npos)
		return std::atoi(data.c_str() + pos + contentLenght.size());
	return 0;
}

void Request::setBadRequest() { this->badRequest = true; }
bool Request::getBadRequest() { return this->badRequest; }

void Request::fillRequest(const std::string& buffer) { data.append(buffer); }

void Request::checkIntegrity()
{
	std::stringstream sstream(data);
	std::string line;
	if (badRequest)
		return;
	if (getMethod() == INVALID || getPath().empty()
		|| getVersion() != "HTTP/1.1")
		badRequest = true;
	while (std::getline(sstream, line))
	{
		;
	}
}

Request& Request::operator=(const Request& other)
{
	if (this == &other)
		return *this;
	this->data = other.data;
	return *this;
}

Request::Request() : badRequest(false) {}

Request::~Request() {}

#endif // !REQUEST_HPP
