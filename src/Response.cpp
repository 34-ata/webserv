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

void Response::buildAndSend(int fd, const std::string& path, Request* req)
{
	getContentType(path);

	if (req->getMethod() == GET)
	{
		filePath = "./www" + path;
		int file = open(filePath.c_str(), O_RDONLY);
		if (file == -1)
		{
			filePath = "./www/404.html";
			status = "HTTP/1.1 404 Not Found";
		}
		else
		{
			status = "HTTP/1.1 200 OK";
			close(file);
		}
	}
	else if (req->getMethod() == POST)
	{
		filePath = "./www" + path;

		int file = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (file == -1)
		{
			filePath = "./www/500.html";
			status = "HTTP/1.1 500 Internal Server Error";
		}
		else
		{
			write(file, req->getBody().c_str(), req->getBody().size());
			close(file);
			filePath = filePath; // hedef dosyanın path'i body yerine gönderilsin istemiyoruz
			status = "HTTP/1.1 201 Created";
			body = "File uploaded successfully.\n";
			contentType = "text/plain";
			generateResponse();
			send(fd, response.c_str(), response.size(), 0);
			close(fd);
			return;
		}
	}

	if (req->getMethod() != GET && req->getMethod() != POST && req->getMethod() != DELETE)
	{
		filePath = "./www/405.html";
		status = "HTTP/1.1 405 Method Not Allowed";
	}

	getFileContent();
	generateResponse();
	send(fd, response.c_str(), response.size(), 0);
	close(fd);
}

void Response::generateResponse()
{
	std::ostringstream oss;
	oss << body.size();
	response =
		status + "\r\n" +
		"Content-Type: " + contentType + "\r\n" +
		"Content-Length: " + oss.str() + "\r\n" +
		"Connection: close\r\n\r\n" +
		body;
}

void Response::getFileContent()
{
	int file = open(filePath.c_str(), O_RDONLY);
	if (file < 0)
		body = "<h1>File Not Found</h1>";

	char buf[1024];
	ssize_t bytes;
	while ((bytes = read(file, buf, sizeof(buf))) > 0)
		body.append(buf, bytes);
	close(file);
}

void Response::getContentType(const std::string& path)
{
	if (path.find(".html") != std::string::npos)
		contentType = "text/html";
	if (path.find(".css") != std::string::npos)
		contentType = "text/css";
	if (path.find(".js") != std::string::npos)
		contentType = "application/javascript";
	if (path.find(".png") != std::string::npos)
		contentType = "image/png";
	if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos)
		contentType = "image/jpeg";
	if (contentType.empty())
		contentType = "text/plain";
}
