#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

enum LogLevel
{
	INFO,
	ERROR,
	WARNING
};

void log(LogLevel level, const std::string& message);
void logRequest(const std::string& method, const std::string& path);
void logResponse(int client_fd, const std::string& status, size_t size);

#endif
