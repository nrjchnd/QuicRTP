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
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <vector>

class Config {
public:
    bool loadConfig();
    std::string get(const std::string& section, const std::string& key) const;
    bool getBool(const std::string& section, const std::string& key) const;
    int getInt(const std::string& section, const std::string& key) const;

    std::vector<std::string> getList(const std::string& section, const std::string& key) const;

private:
    std::map<std::string, std::map<std::string, std::string>> data;
    static std::string trim(const std::string& str);
};

#endif
