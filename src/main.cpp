#include "WebServer.hpp"
#include <iostream>

int main(int argc, char** argv)
{
	if (argc > 2)
	{
		std::cerr << "Usage: ./webserv <config file>" << std::endl;
		return 1;
	}

	std::string configFile;
	if (argc == 1)
		configFile = "./config";
	else
		configFile = argv[1];
	

	WebServer webserv;
	if (!webserv.Init(configFile.c_str()))
	{
		std::cerr << "Failed to init server." << std::endl;
		return 1;
	}

	webserv.Run();
	webserv.Shutdown();
	return 0;
}
