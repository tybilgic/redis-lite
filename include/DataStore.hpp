#pragma once

#include <unordered_map>
#include <string>

class DataStore
{
public:
    void set(const std::string &key, const std::string &value);
    std::string get(const std::string &key) const;

private:
    std::unordered_map<std::string, std::string> m_store;
};
