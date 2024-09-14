#pragma once

#include <chrono>
#include <optional>
#include <mutex>
#include <unordered_map>
#include <string>

class DataStore
{
public:
    void set(const std::string &key, const std::string &value,
             std::optional<std::chrono::milliseconds> expire_time = std::nullopt);
    std::string get(const std::string &key);
    bool exists(const std::string &key);
    bool del(const std::string &key);

private:
    struct ValueEntry
    {
        std::string value;
        std::optional<std::chrono::steady_clock::time_point> expiry;
    };

    std::unordered_map<std::string, ValueEntry> m_store;
    std::mutex m_store_mutex;

    bool is_expired_entry(const ValueEntry &entry) const;
};
