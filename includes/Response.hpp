#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "ResponseCodes.hpp"
#include <map>
#include <string>

class Response
{
  public:
	Response();
	Response(const Response& other);
	Response& operator=(const Response& other);
	~Response();

  public:
	Response& status(ResponseCodes status);
	Response& httpVersion(std::string httpVersion);
	Response& header(std::string key, std::string value);
	Response& body(std::string body);
	std::string build();

  public:
	static std::string mapCodeToStr(ResponseCodes code);

  private:
	ResponseCodes m_status;
	std::string m_httpVersion;
	std::map< std::string, std::string > m_headers;
	std::string m_body;
};

std::string getContentType(const std::string& path);

std::string getFileContent(std::string filePath);

#endif
