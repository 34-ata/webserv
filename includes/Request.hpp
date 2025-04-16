#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>

class Request
{
  public:
	Request();
	Request(const Request&);
	Request& operator=(const Request&);
	~Request();

  private:
	std::string data;
};

Request::Request() {}

Request::~Request() {}

#endif // !REQUEST_HPP
