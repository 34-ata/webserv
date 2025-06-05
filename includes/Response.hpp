#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include "HttpMethods.hpp"
#include "Request.hpp"

class Response
{
  public:
	Response();
	Response(const Response& other);
	Response& operator=(const Response& other);
	~Response();

	void buildAndSend(int fd, const std::string& path, Request* req);

	private:
		std::string status;
		std::string body;
		std::string filePath;
		std::string contentType;
		std::string response;

  private:
	void getFileContent();
	void getContentType(const std::string& path);
	void generateResponse();
};

#endif
