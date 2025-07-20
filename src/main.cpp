#include "WebServer.hpp"
#include <cstdlib>
#include <iostream>

WebServer* g_server = NULL;

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config file>" << std::endl;
		return 1;
	}

	g_server = new WebServer();
	if (!g_server->Init(argv[1]))
	{
		std::cerr << "Failed to init server." << std::endl;
		delete g_server;
		return 1;
	}

	g_server->Run();

	g_server->Shutdown();
	delete g_server;
	return 0;
}
