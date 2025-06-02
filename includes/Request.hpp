#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Server.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
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

  public:
	HttpMethods getMethod() const;
	const std::string& getData() const;
	const std::string& getBody() const;
	size_t getBodyLenght() const;

  private:
	std::string data;
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

const std::string& Request::getData() const { return data; }

size_t Request::getBodyLenght() const
{
	return std::atoi(data.c_str() + data.find("Content-Length") + 15);
}

void Request::fillRequest(const std::string& buffer) { data.append(buffer); }

Request& Request::operator=(const Request& other)
{
	if (this == &other)
		return *this;
	this->data = other.data;
	return *this;
}

Request::Request() {}

Request::~Request() {}

#endif // !REQUEST_HPP
