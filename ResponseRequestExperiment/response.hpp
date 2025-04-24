#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include "request.hpp"
#include "config.hpp"

struct ClientContext {
    std::string buffer;
    std::string status;
    size_t bytes_sent;

    ClientContext() : buffer(""), status("200 OK"), bytes_sent(0) {}
    ClientContext(const std::string& data, const std::string& st) : buffer(data), status(st), bytes_sent(0) {}
};

extern std::map<int, ClientContext> clientContexts;

void prepareHttpResponse(int client_fd, const Request& req, const Config& conf);

std::string getContentType(const std::string& path);

#endif
