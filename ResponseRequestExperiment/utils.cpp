#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>

std::string readFileContent(const std::string& path) {
    std::ifstream file(path.c_str());
    if (!file.is_open())
        return "";
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

bool isDirectory(const std::string& path) {
    struct stat st;
    return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}
