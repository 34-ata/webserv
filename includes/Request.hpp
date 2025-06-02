#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Server.hpp"
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

  private:
	std::string data;
};

HttpMethods Request::getMethod() const
{
	std::string method = data.substr(0 ,data.find(" "));
	if (method == "GET")
		return GET;
	if (method == "POST")
		return POST;
	if (method == "DELETE")
		return DELETE;
	return INVALID;
}

void Request::fillRequest(const std::string& buffer)
{
	data.append(buffer);
}

Request::Request() {}

Request::~Request() {}

#endif // !REQUEST_HPP
