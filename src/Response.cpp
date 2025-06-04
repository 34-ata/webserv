#include "../includes/Response.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sstream>
#include <iostream>
#include <sys/socket.h>

Response::Response() {}
Response::Response(const Response& other) { (void)other; }
Response& Response::operator=(const Response& other) { (void)other; return *this; }
Response::~Response() {}

void Response::buildAndSend(int fd, HttpMethods method, const std::string& path)
{
	std::string filepath;
	std::string status;
	std::string body;
	std::string contentType = "text/html";

	if (method != GET)
	{
		filepath = "./www/405.html";
		status = "HTTP/1.1 405 Method Not Allowed";
	}
	else
	{
		filepath = "./www" + path;
		int file = open(filepath.c_str(), O_RDONLY);
		if (file == -1)
		{
			filepath = "./www/404.html";
			status = "HTTP/1.1 404 Not Found";
		}
		else
		{
			status = "HTTP/1.1 200 OK";
			close(file);
		}
	}

	body = getFileContent(filepath);

	std::ostringstream oss;
	oss << body.size();
	std::string response =
		status + "\r\n" +
		"Content-Type: " + contentType + "\r\n" +
		"Content-Length: " + oss.str() + "\r\n" +
		"Connection: close\r\n\r\n" +
		body;

	send(fd, response.c_str(), response.size(), 0);
	close(fd);
}

std::string Response::getFileContent(const std::string& filepath)
{
	std::string content;
	int file = open(filepath.c_str(), O_RDONLY);
	if (file < 0)
		return "<h1>File Not Found</h1>";

	char buf[1024];
	ssize_t bytes;
	while ((bytes = read(file, buf, sizeof(buf))) > 0)
		content.append(buf, bytes);
	close(file);
	return content;
}

std::string Response::getContentType(const std::string& path)
{
	if (path.find(".html") != std::string::npos)
		return "text/html";
	if (path.find(".css") != std::string::npos)
		return "text/css";
	if (path.find(".js") != std::string::npos)
		return "application/javascript";
	if (path.find(".png") != std::string::npos)
		return "image/png";
	if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos)
		return "image/jpeg";
	return "text/plain";
}
