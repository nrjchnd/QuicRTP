#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <vector>

class Config {
public:
    bool loadConfig(const std::string& filename);
    std::string get(const std::string& section, const std::string& key) const;
    bool getBool(const std::string& section, const std::string& key) const;
    int getInt(const std::string& section, const std::string& key) const;
    std::vector<std::string> getList(const std::string& section, const std::string& key) const;

private:
    std::map<std::string, std::map<std::string, std::string>> data;
};

#endif // CONFIG_H
