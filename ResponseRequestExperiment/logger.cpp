#include "logger.hpp"
#include <ctime>
#include <fstream>
#include <sstream>

static std::string getCurrentTime()
{
	time_t now = time(0);
	char buf[32];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
	return std::string(buf);
}

static std::string levelToString(LogLevel level)
{
	if (level == INFO)
		return "INFO";
	if (level == ERROR)
		return "ERROR";
	if (level == WARNING)
		return "WARN";
	return "LOG";
}

void log(LogLevel level, const std::string& message)
{
	std::ofstream logFile("server.log", std::ios::app);
	logFile << "[" << getCurrentTime() << "] "
			<< "[" << levelToString(level) << "] " << message << "\n";
}

void logRequest(const std::string& method, const std::string& path)
{
	log(INFO, "Request received: " + method + " " + path);
}

void logResponse(int client_fd, const std::string& status, size_t size)
{
	LogLevel level = (status.find("404") != std::string::npos
					  || status.find("500") != std::string::npos)
						 ? ERROR
						 : INFO;

	std::ostringstream oss;
	oss << "Response to fd=" << client_fd << " | " << status << " | " << size
		<< " bytes";

	log(level, oss.str());
}
