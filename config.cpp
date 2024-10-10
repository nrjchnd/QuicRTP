/*
 * Copyright 2024 nrjchnd@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an **"AS IS" BASIS,**
 * **WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.**
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

bool Config::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Unable to open configuration file: " + filename);

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
            } else {
                throw std::runtime_error("Invalid line in config file: " + line);
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
    try {
        return std::stoi(val);
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Invalid integer value for " + key + " in section " + section);
    }
}

std::vector<std::string> Config::getList(const std::string& section, const std::string& key) const {
    std::string val = get(section, key);
    std::vector<std::string> list;
    std::stringstream ss(val);
    std::string item;
    while (std::getline(ss, item, ',')) {
        list.push_back(item);
    }
    return list;
}
