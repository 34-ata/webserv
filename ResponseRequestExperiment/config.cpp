#include "config.hpp"

Config::Config() {
    port = 8080;
    server_name = "localhost";
    root = "./";
    error_pages[404] = "404.html";

    Location loc;
    loc.path = "/";
    loc.root = "./";
    loc.autoindex = true;

    locations["/"] = loc;
}
