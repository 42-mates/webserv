#include "Config.hpp"
#include <fstream>
#include <iostream>

Config::Config(const std::string& path) : _configPath(path) {}

void Config::readFile() const {
    std::ifstream file(_configPath.c_str());
    if (!file) {
        std::cerr << "Error: Cannot open config file: " << _configPath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::cout << "LINE: " << line << std::endl;
    }

    file.close();
}
