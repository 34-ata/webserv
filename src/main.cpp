#include "Tokenizer.hpp"
#include "WebServer.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/ip.h>
#include <sys/socket.h>

#define IP "127.0.0.1"

int main(int argc, char** argv)
{
	struct sockaddr_in int_sock;

	if (argc != 2)
		std::cerr << "Usage: ./webserv <config file>" << std::endl;

	memset(&int_sock, 0, sizeof(struct sockaddr_in));
	int_sock.sin_family = AF_INET;
	int_sock.sin_port	= htons(6666);
	inet_pton(AF_INET, IP, &int_sock.sin_addr);

	std::cout << "Hello, World!" << std::endl;
	WebServer ws;
	if (!ws.Init(argv[1]))
		std::cerr << "Failed to init server." << std::endl;
	return (0);
}
