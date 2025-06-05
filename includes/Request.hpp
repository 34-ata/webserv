#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "HttpMethods.hpp"
#include "Log.hpp"
#include "Server.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>
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

  public:
	HttpMethods getMethod() const;
	std::string getPath() const;
	std::string getVersion() const;
	const std::string& getData() const;
	const std::string getBody() const;
	std::size_t getBodyLenght() const;
	bool getBadRequest();
	void setBadRequest();

  private:
	static HttpMethods strToMethod(const std::string& str);

  private:
	std::string m_data;
	HttpMethods m_method;
	std::string m_path;
	std::string m_httpVersion;
	std::size_t m_bodyOffset;
	bool m_badRequest;
};

HttpMethods Request::getMethod() const { return m_method; }

std::string Request::getPath() const { return m_path; }

std::string Request::getVersion() const { return m_httpVersion; }

const std::string& Request::getData() const { return m_data; }

const std::string Request::getBody() const
{
	return m_data.substr(m_bodyOffset);
}

std::size_t Request::getBodyLenght() const
{
	std::string contentLenght = "Content-Length";
	std::size_t pos			  = m_data.find(contentLenght);
	if (pos != std::string::npos)
		return std::atoi(m_data.c_str() + pos + contentLenght.size());
	return 0;
}

bool Request::getBadRequest() { return this->m_badRequest; }

void Request::setBadRequest() { this->m_badRequest = true; }

void Request::fillRequest(const std::string& buffer) { m_data.append(buffer); }

void Request::checkIntegrity()
{
	std::stringstream sstream(m_data);
	std::string line;
	std::getline(sstream, line);
	if (m_badRequest)
		return;
	size_t firstSpace = line.find(' ');
	if (firstSpace == std::string::npos)
	{
		m_badRequest = true;
		return;
	}
	LOG("Passed first Space");
	size_t secondSpace = line.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
	{
		m_badRequest = true;
		return;
	}
	LOG("Passed second Space");
	if (line.find(' ', secondSpace + 1) != std::string::npos)
	{
		m_badRequest = true;
		return;
	}
	LOG("Passed more Space");
	LOG(line.substr(0, firstSpace))
	m_method = strToMethod(line.substr(0, firstSpace));
	LOG(m_method);
	if (m_method == INVALID)
	{
		m_badRequest = true;
		return;
	}
	m_path = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
	LOG(std::string("-") + m_path + std::string("-"));
	if (m_path.empty())
	{
		m_badRequest = true;
		return;
	}
	m_httpVersion = line.substr(secondSpace + 1, line.size() - secondSpace - 2);
	LOG(m_httpVersion + std::string("--"));
	if (m_httpVersion != "HTTP/1.1")
	{
		m_badRequest = true;
		return;
	}
}

Request& Request::operator=(const Request& other)
{
	if (this == &other)
		return *this;
	this->m_data = other.m_data;
	return *this;
}

Request::Request() : m_badRequest(false) {}

Request::~Request() {}

HttpMethods Request::strToMethod(const std::string& str)
{
	if (str == "GET")
		return GET;
	if (str == "POST")
		return POST;
	if (str == "DELETE")
		return DELETE;
	return INVALID;
}

#endif // !REQUEST_HPP
