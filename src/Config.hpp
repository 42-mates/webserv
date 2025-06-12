/**
 * config-file parser
 */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

class Config {
public:
    Config(const std::string& path);
    void readFile() const;

private:
    std::string _configPath;
};

#endif // CONFIG_HPP