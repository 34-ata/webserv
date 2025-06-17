#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "HttpMethods.hpp"
#include "Request.hpp"
#include "ResponseCodes.hpp"
#include <cstddef>
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
	Response& htppVersion(std::string htppVersion);
	Response& header(std::string key, std::string value);
	Response& body(std::string body);
	static std::string mapCodeToStr(ResponseCodes code);
	std::string build();

  private:
	ResponseCodes m_status;
	std::string m_htppVersion;
	std::map< std::string, std::string > m_headers;
	std::string m_body;
};

std::string getContentType(const std::string& path);

std::string getFileContent(std::string filePath);

#endif
