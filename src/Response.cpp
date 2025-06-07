#include "Response.hpp"
#include "HttpMethods.hpp"
#include "HttpVersion.hpp"
#include "ResponseCodes.hpp"
#include <fcntl.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

Response::Response() {}

Response::Response(const Response& other) { (void)other; }

Response& Response::operator=(const Response& other)
{
	(void)other;
	return *this;
}

Response::~Response() {}

Response& Response::status(ResponseCodes status)
{
	m_status = status;
	return *this;
}

Response& Response::htppVersion(std::string htppVersion)
{
	m_htppVersion = htppVersion;
	return *this;
}

Response& Response::header(std::string key, std::string value)
{
	m_headers[key] = value;
	return *this;
}

Response& Response::body(std::string body)
{
	m_body = body;
	return *this;
}

static std::string mapCodeToStr(ResponseCodes code)
{
	switch (code)
	{
	case OK:
		return "OK";
	case CREATED:
		return "Created";
	case NOT_FOUND:
		return "Not Found";
	case METH_NOT_ALLOW:
		return "Method Not Allowed";
	case INT_SERV_ERR:
		return "Internal Server Error";
	default:
		return "INVALID";
	}
}

std::string Response::build()
{
	std::stringstream response;
	std::stringstream size;
	if (m_body.length() != 0)
	{
		size << m_body.length();
		m_headers["Content-Length"] = size.str();
	}
	response << m_htppVersion << " " << m_status << " "
			 << mapCodeToStr(m_status) << "\r\n";
	for (std::map< std::string, std::string >::iterator it = m_headers.begin();
		 it != m_headers.end(); it++)
		response << it->first << ": " << it->second << "\r\n";
	response << "\r\n";
	response << m_body;
	return response.str();
}

std::string getFileContent(std::string filePath)
{
	std::string fileContent;
	int file = open(filePath.c_str(), O_RDONLY);
	if (file < 0)
	{
		fileContent = "<h1>File Not Found</h1>";
		return fileContent;
	}
	char buf[1024];
	ssize_t bytes;
	while ((bytes = read(file, buf, sizeof(buf))) > 0)
		fileContent.append(buf, bytes);
	close(file);
	return fileContent;
}

std::string getContentType(const std::string& path)
{
	if (path.find(".html") != std::string::npos)
		return "text/html";
	if (path.find(".css") != std::string::npos)
		return "text/css";
	if (path.find(".js") != std::string::npos)
		return "application/javascript";
	if (path.find(".png") != std::string::npos)
		return "image/png";
	if (path.find(".jpg") != std::string::npos
		|| path.find(".jpeg") != std::string::npos)
		return "image/jpeg";
	return "text/plain";
}
