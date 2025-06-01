#include "Tokenizer.hpp"
#include "WebServer.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/ip.h>
#include <sys/socket.h>

int main(int argc, char** argv)
{
	if (argc != 2)
	std::cerr << "Usage: ./webserv <config file>" << std::endl;
	
	WebServer ws;
	if (!ws.Init(argv[1]))
		std::cerr << "Failed to init server." << std::endl;
	
	ws.Run();
	return (0);
}
