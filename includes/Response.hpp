#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include "HttpMethods.hpp"

class Response
{
  public:
	Response();
	Response(const Response& other);
	Response& operator=(const Response& other);
	~Response();

	static void buildAndSend(int fd, HttpMethods method, const std::string& path);

  private:
	static std::string getFileContent(const std::string& filepath);
	static std::string getContentType(const std::string& path);
};

#endif
