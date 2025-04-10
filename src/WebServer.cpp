#include "WebServer.hpp"
#include <fstream>
#include <iostream>
#include <string>

WebServer::WebServer() {}

WebServer::WebServer(const WebServer& other) { *this = other; }

WebServer::~WebServer() {}

WebServer& WebServer::operator=(const WebServer& other)
{
	(void)other;
	return *this;
}

bool WebServer::Init(const std::string& config)
{
	std::ifstream confStream;
	std::string buffer;
	std::string fileContent;

	confStream.open(config.c_str());
	if (!confStream.is_open())
		return false;

	while (!confStream.eof())
	{
		std::getline(confStream, buffer);
		fileContent += buffer;
		fileContent += "\n";
	}

	ParseServers(fileContent);

	std::cout << fileContent << std::endl;
	return true;
}

void WebServer::ParseServers(const std::string& fileContent) {

	std::string tempContent(fileContent);
	std::string::iterator startPos = tempContent.begin();

	for (; startPos != tempContent.end(); startPos++)
		if (std::string(" \t\n\r\v").find(*startPos) == std::string::npos)
			break;

	
}
