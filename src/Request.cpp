#include "Request.hpp"
#include <cstdarg>
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <string>
#include <sys/types.h>

HttpMethods Request::getMethod() const { return m_method; }

std::string Request::getPath() const { return m_path; }

std::string Request::getVersion() const { return m_httpVersion; }

const std::string& Request::getData() const { return m_data; }

std::string Request::getBody() const
{
	size_t headerEnd = m_data.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return std::string();
	return m_data.substr(headerEnd + 4, m_data.size() - headerEnd);
}

std::size_t Request::getBodyLenght() const
{
	std::string contentLength = "Content-Length";
	if (m_headers.find(contentLength) != m_headers.end())
		return std::atoi(m_headers.at(contentLength).c_str());
	return 0;
}

bool Request::getBadRequest() const { return this->m_badRequest; }

void Request::setBadRequest() { this->m_badRequest = true; }

void Request::fillRequest(const std::string& buffer) { m_data.append(buffer); }

void Request::requestLineIntegrity(std::stringstream& dataStream)
{
	std::string line;
	std::getline(dataStream, line);
	if (m_badRequest)
		return;
	size_t firstSpace = line.find(' ');
	if (firstSpace == std::string::npos)
	{
		m_badRequest = true;
		return;
	}
	size_t secondSpace = line.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
	{
		m_badRequest = true;
		return;
	}
	if (line.find(' ', secondSpace + 1) != std::string::npos)
	{
		m_badRequest = true;
		return;
	}
	m_method = strToMethod(line.substr(0, firstSpace));
	if (m_method == INVALID)
	{
		m_badRequest = true;
		return;
	}
	m_path = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
	if (m_path.empty())
	{
		m_badRequest = true;
		return;
	}
	m_httpVersion = line.substr(secondSpace + 1, line.size() - secondSpace - 2);
	if (m_httpVersion != "HTTP/1.1")
	{
		m_badRequest = true;
		return;
	}
}

void Request::checkHeaders(std::stringstream& dataStream)
{
	std::string line;
	while (std::getline(dataStream, line))
	{
		std::string key, value;
		size_t halfPos = line.find(':');
		if (line == "\r")
			continue;
		if (halfPos == std::string::npos)
		{
			m_badRequest = true;
			return;
		}
		key = line.substr(0, halfPos);
		if (key.find(' ') != std::string::npos)
		{
			m_badRequest = true;
			return;
		}
		value = line.substr(halfPos + 2, line.size() - halfPos - 3);
		if (value.find(' ') != std::string::npos)
		{
			m_badRequest = true;
			return;
		}
		m_headers[key] = value;
	}
}

void Request::checkIntegrity()
{
	std::stringstream dataStream(m_data);
	requestLineIntegrity(dataStream);
	checkHeaders(dataStream);
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
