#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <string>

struct Location
{
	std::string path;
	std::string root;
	bool autoindex;
};

struct Config
{
	int port;
	std::string server_name;
	std::string root;
	std::map<int, std::string> error_pages;
	std::map<std::string, Location> locations;

	Config();
};

#endif
