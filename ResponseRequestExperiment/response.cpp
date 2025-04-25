#include "response.hpp"
#include "utils.hpp"
#include <dirent.h>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <fstream>

std::map<int, ClientContext> clientContexts;

std::string getContentType(const std::string& path)
{
	size_t dot = path.rfind('.');
	if (dot == std::string::npos)
		return "application/octet-stream";
	std::string ext = path.substr(dot + 1);
	if (ext == "html")
		return "text/html";
	if (ext == "css")
		return "text/css";
	if (ext == "js")
		return "application/javascript";
	if (ext == "jpg" || ext == "jpeg")
		return "image/jpeg";
	if (ext == "png")
		return "image/png";
	if (ext == "gif")
		return "image/gif";
	if (ext == "ico")
		return "image/x-icon";
	if (ext == "txt")
		return "text/plain";
	return "application/octet-stream";
}

std::string generateAutoIndex(const std::string& dirPath)
{
	DIR* dir = opendir(dirPath.c_str());
	if (!dir)
		return "";

	std::stringstream html;
	html << "<html><body><h1>Index of /" << dirPath << "</h1><ul>";
	struct dirent* entry;
	while ((entry = readdir(dir)))
	{
		std::string name = entry->d_name;
		if (name == ".")
			continue;
		html << "<li><a href='/" << dirPath << "/" << name << "'>" << name
			 << "</a></li>";
	}
	html << "</ul></body></html>";
	closedir(dir);
	return html.str();
}

void prepareHttpResponse(int client_fd, const Request& req, const Config& conf) {
    std::string response;
    std::string content;
    std::string status;
    std::string fullPath = conf.root + req.path;

    if (req.method == "GET") {
        if (isDirectory(fullPath)) {
            content = generateAutoIndex(fullPath);
            status = "200 OK";
        } else {
            content = readFileContent(fullPath);
            if (content.empty()) {
                std::map<int, std::string>::const_iterator it = conf.error_pages.find(404);
                content = (it != conf.error_pages.end()) ? readFileContent(it->second) : "<h1>404 - Not Found</h1>";
                status = "404 Not Found";
            } else {
                status = "200 OK";
            }
        }
    }
    else if (req.method == "POST") {
		std::ofstream uploaded("uploaded.txt");
		if (!uploaded) {
			content = "<h1>500 Internal Server Error</h1>";
			status = "500 Internal Server Error";
		} else {
			uploaded << req.body;
			uploaded.close();
			content = "<h1>POST Data Saved</h1>";
			status = "200 OK";
		}
	}
	
    else if (req.method == "DELETE") {
        if (remove(fullPath.c_str()) == 0) {
            content = "<h1>Deleted successfully</h1>";
            status = "200 OK";
        } else {
            content = "<h1>File not found</h1>";
            status = "404 Not Found";
        }
    }
    else if (req.method == "HEAD") {
        content = "";
        status = "200 OK";
    }
    else {
        content = "<h1>501 Not Implemented</h1>";
        status = "501 Not Implemented";
    }

    std::ostringstream ss;
    ss << "HTTP/1.1 " << status << "\r\n"
       << "Content-Type: text/html\r\n"
       << "Content-Length: " << content.size() << "\r\n"
       << "\r\n";

    if (req.method != "HEAD")
        ss << content;

    response = ss.str();
    clientContexts[client_fd] = ClientContext(response, status);
}
