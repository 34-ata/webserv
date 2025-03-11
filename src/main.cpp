#include "Socket.hpp"
#include <iostream>
#include <string>
#include <vector>

int main(void)
{
	std::pair<std::string, std::vector<int> > a;
	a.first = "Key";
	a.second = std::vector<int>();
	a.second.push_back(5);
	std::cout << "Hello, World! TEST for better results and creating more "
				 "complex and long line of text to make formatter to split the "
				 "main text as a multiple line for easy reading."
			  << std::endl;
}
