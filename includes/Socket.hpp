#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>

class Socket
{
  public:
	Socket();
	Socket(const Socket&);
	Socket& operator=(const Socket&);
	~Socket();

  private:
	unsigned short m_Port;
	const std::string link;
};

Socket::Socket() {}

Socket::~Socket() {}

#endif // !SOCKET_HPP
