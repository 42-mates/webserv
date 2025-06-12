#include "Webserv.hpp"

int main(int argc, char** argv) {
    std::string configPath;

    if (argc == 2) {
        configPath = argv[1];
    } else if (argc == 1) {
        configPath = DEFAULT_CONFIG;
        std::cout << "No config provided. Using default: " << configPath << std::endl;
    } else {
        std::cerr << "Usage: ./webserv [config_file]" << std::endl;
        return 1;
    }

    std::cout << "Webserv starting with config: " << configPath << std::endl;
    
    Config config(configPath);
    config.readFile();

    return 0;
}
