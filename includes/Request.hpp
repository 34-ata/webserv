#ifndef DEBUG

class Request
{
  public:
	Request();
	Request(const Request&);
	Request& operator=(const Request&);
	~Request();

  private:
};

Request::Request() {}

Request::~Request() {}

#endif // !DEBUG
