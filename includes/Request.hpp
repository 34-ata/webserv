#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "HttpMethods.hpp"
#include <ctime>
#include <map>
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
	HttpMethods getMethod() const;
	std::string getPath() const;
	std::string getContentType() const;
	std::string getVersion() const;
	const std::string& getData() const;
	std::string getBody() const;
	size_t getBodyLenght() const;
	bool getBadRequest() const;
	void setBadRequest();
	bool shouldClose() const;
	time_t getTimeStamp() const;
	void setTimeStamp(time_t t);

  private:
	void requestLineIntegrity(std::stringstream& dataStream);
	void checkHeaders(std::stringstream& dataStream);

  private:
	time_t m_timestamp;
	bool m_shouldClose;
	std::string m_data;
	HttpMethods m_method;
	std::string m_path;
	std::string m_httpVersion;
	std::size_t m_bodyOffset;
	std::map< std::string, std::string > m_headers;
	bool m_badRequest;
};

#endif
