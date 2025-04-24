#include "response.hpp"
#include "utils.hpp"
#include <sstream>
#include <dirent.h>
#include <iostream>

std::map<int, ClientContext> clientContexts;

std::string getContentType(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos)
        return "application/octet-stream";
    std::string ext = path.substr(dot + 1);
    if (ext == "html") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "png") return "image/png";
    if (ext == "gif") return "image/gif";
    if (ext == "ico") return "image/x-icon";
    if (ext == "txt") return "text/plain";
    return "application/octet-stream";
}

std::string generateAutoIndex(const std::string& dirPath) {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir)
        return "";

    std::stringstream html;
    html << "<html><body><h1>Index of /" << dirPath << "</h1><ul>";
    struct dirent* entry;
    while ((entry = readdir(dir))) {
        std::string name = entry->d_name;
        if (name == ".") continue;
        html << "<li><a href='/" << dirPath << "/" << name << "'>" << name << "</a></li>";
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

    if (isDirectory(fullPath)) {
        content = generateAutoIndex(fullPath);
        status = "200 OK";
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " +
                   static_cast<std::ostringstream&>(std::ostringstream() << content.length()).str() +
                   "\r\n\r\n" + content;
    } else {
        content = readFileContent(fullPath);
        if (content.empty()) {
            std::map<int, std::string>::const_iterator it = conf.error_pages.find(404);
            if (it != conf.error_pages.end())
                content = readFileContent(it->second);
            else
                content = "<h1>404 - Not Found</h1>";

            status = "404 Not Found";
            response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " +
                       static_cast<std::ostringstream&>(std::ostringstream() << content.length()).str() +
                       "\r\n\r\n" + content;
        } else {
            status = "200 OK";
            std::string contentType = getContentType(req.path);
            response = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType +
                       "\r\nContent-Length: " + static_cast<std::ostringstream&>(std::ostringstream() << content.length()).str() +
                       "\r\n\r\n" + content;
        }
    }

    clientContexts[client_fd] = ClientContext(response, status);
}


