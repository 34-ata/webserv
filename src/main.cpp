#include "WebServer.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/ip.h>
#include <sys/socket.h>

#define IP "127.0.0.1"

int main(void)
{
	struct sockaddr_in int_sock;
	memset(&int_sock, 0, sizeof(struct sockaddr_in));
	int_sock.sin_family = AF_INET;
	int_sock.sin_port	= htons(6666);
	inet_pton(AF_INET, IP, &int_sock.sin_addr);

	std::cout << "Hello, World!" << std::endl;
	WebServer ws;
	ws.Init("./config");
	return (0);
}
