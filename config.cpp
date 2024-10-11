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

const std::string DEFAULT_CONFIG_PATH = "/etc/quicrtp/quicrtp.conf";

// Helper method to trim whitespace from both ends of a string
std::string Config::trim(const std::string& str) {
    const char* whitespace = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos)
        return ""; // All whitespace

    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

bool Config::loadConfig() {
    std::ifstream file(DEFAULT_CONFIG_PATH);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open configuration file: " + DEFAULT_CONFIG_PATH);
    }

    std::string line;
    std::string currentSection;

    while (std::getline(file, line)) {
        // Remove comments starting with '#'
        auto commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // Trim whitespace from both ends
        line = trim(line);

        if (line.empty()) {
            continue; // Skip empty lines
        }

        // Check for section headers [Section]
        if (line.front() == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            currentSection = trim(currentSection); // Trim in case of extra spaces
            continue;
        }

        // Parse key=value pairs
        auto equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = trim(line.substr(0, equalPos));
            std::string value = trim(line.substr(equalPos + 1));

            if (key.empty()) {
                throw std::runtime_error("Empty key found in section [" + currentSection + "]");
            }

            data[currentSection][key] = value;
        } else {
            throw std::runtime_error("Invalid line in config file: " + line);
        }
    }

    file.close();
    return true;
}

std::string Config::get(const std::string& section, const std::string& key) const {
    auto secIt = data.find(section);
    if (secIt != data.end()) {
        auto keyIt = secIt->second.find(key);
        if (keyIt != secIt->second.end()) {
            return keyIt->second;
        }
    }
    return "";
}

bool Config::getBool(const std::string& section, const std::string& key) const {
    std::string val = get(section, key);
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    return (val == "true" || val == "1" || val == "yes" || val == "on");
}

int Config::getInt(const std::string& section, const std::string& key) const {
    std::string val = get(section, key);
    try {
        return std::stoi(val);
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Invalid integer value for key '" + key + "' in section '" + section + "'");
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Integer value out of range for key '" + key + "' in section '" + section + "'");
    }
}

std::vector<std::string> Config::getList(const std::string& section, const std::string& key) const {
    std::string val = get(section, key);
    std::vector<std::string> list;
    std::stringstream ss(val);
    std::string item;

    while (std::getline(ss, item, ',')) {
        item = trim(item);
        if (!item.empty()) {
            list.push_back(item);
        }
    }

    return list;
}
