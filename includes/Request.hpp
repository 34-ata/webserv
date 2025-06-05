#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "HttpMethods.hpp"
#include <string>

class Request
{
  public:
	Request();
	Request(const Request&);
	Request& operator=(const Request&);
	~Request();

	void fillRequest(const std::string& buffer);
	void checkIntegrity();

	HttpMethods getMethod() const;
	std::string getPath() const;
	std::string getVersion() const;
	const std::string& getData() const;
	std::string getBody() const;
	size_t getBodyLenght() const;
	bool getBadRequest() const;
	void setBadRequest();

  private:
	std::string data;
	bool badRequest;
};

#endif
