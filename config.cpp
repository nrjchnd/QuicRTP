#include "config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

bool Config::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        return false;

    std::string line;
    std::string currentSection;

    while (std::getline(file, line)) {
        // Remove comments
        auto commentPos = line.find('#');
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);

        // Trim whitespace
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        if (line.empty())
            continue;

        // Check for section
        if (line.front() == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
        } else {
            // Parse key=value
            auto equalPos = line.find('=');
            if (equalPos != std::string::npos) {
                std::string key = line.substr(0, equalPos);
                std::string value = line.substr(equalPos + 1);

                data[currentSection][key] = value;
            }
        }
    }

    file.close();
    return true;
}

std::string Config::get(const std::string& section, const std::string& key) const {
    auto secIt = data.find(section);
    if (secIt != data.end()) {
        auto keyIt = secIt->second.find(key);
        if (keyIt != secIt->second.end())
            return keyIt->second;
    }
    return "";
}

bool Config::getBool(const std::string& section, const std::string& key) const {
    std::string val = get(section, key);
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    return val == "true" || val == "1";
}

int Config::getInt(const std::string& section, const std::string& key) const {
    std::string val = get(section, key);
    return std::stoi(val);
}
