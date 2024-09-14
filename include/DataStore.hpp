#pragma once

#include <chrono>
#include <optional>
#include <mutex>
#include <unordered_map>
#include <string>
#include <list>
#include <variant>
#include <vector>

class DataStore
{
public:
    void set(const std::string &key, const std::string &value,
             std::optional<std::chrono::milliseconds> expire_time = std::nullopt);
    std::string get(const std::string &key);
    bool exists(const std::string &key);
    bool del(const std::string &key);
    int incr(const std::string &key);
    int decr(const std::string &key);

    int lpush(const std::string &key, const std::string &value);
    int rpush(const std::string &key, const std::string &value);
    std::vector<std::string> lrange(const std::string &key, int start, int stop);

private:
    struct ValueEntry
    {
        using ValueType = std::variant<std::string, std::list<std::string>>;
        ValueType value;
        std::optional<std::chrono::steady_clock::time_point> expiry;
    };

    std::unordered_map<std::string, ValueEntry> m_store;
    std::mutex m_store_mutex;

    bool is_expired_entry(const ValueEntry &entry) const;
};
