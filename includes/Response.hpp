#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <cstddef>
#include <map>
#include <string>
#include "HttpMethods.hpp"
#include "Request.hpp"
#include "ResponseCodes.hpp"

class Response
{
  public:
	Response();
	Response(const Response& other);
	Response& operator=(const Response& other);
	~Response();

	void buildAndSend(int fd, const std::string& path, Request* req);

  public:
	Response& status(ResponseCodes status);
	Response& htppVersion(std::string htppVersion);
	Response& header(std::string key, std::string value);
	Response& body(std::string body);
	std::string build();

	private:
		ResponseCodes m_status;
		std::string m_htppVersion;
		std::map<std::string, std::string> m_headers;
		std::string m_body;

  private:
	std::string getFileContent(std::string filePath);
	std::string getContentType(const std::string& path);
};

#endif
