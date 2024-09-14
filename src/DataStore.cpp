#include "DataStore.hpp"

void DataStore::set(const std::string &key, const std::string &value, std::optional<std::chrono::milliseconds> expire_time)
{
    std::lock_guard<std::mutex> lock(m_store_mutex);
    ValueEntry entry;
    entry.value = value;
    if (expire_time.has_value())
    {
        entry.expiry = std::chrono::steady_clock::now() + expire_time.value();
    }
    m_store[key] = std::move(entry);
}

std::string DataStore::get(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_store_mutex);
    auto it = m_store.find(key);

    if (it != m_store.end())
    {
        if (is_expired_entry(it->second))
        {
            m_store.erase(it);
            return "";
        }
        return std::get<std::string>(it->second.value);
    }

    return "";
}

bool DataStore::exists(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_store_mutex);
    auto it = m_store.find(key);
    if (it != m_store.end() &&
        !is_expired_entry(it->second))
    {
        return true;
    }
    return false;
}

bool DataStore::del(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_store_mutex);
    auto it = m_store.find(key);
    return m_store.erase(key) > 0;
}

int DataStore::incr(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_store_mutex);
    auto it = m_store.find(key);
    int value = 0;
    if (it != m_store.end() &&
        !is_expired_entry(it->second))
    {
        // TODO: handle exceptions i.e not a number, out-of-bound
        value = std::stoi(std::get<std::string>(it->second.value));
    }
    value += 1;
    m_store[key].value = std::to_string(value);
    return value;
}

int DataStore::decr(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_store_mutex);
    auto it = m_store.find(key);
    int value = 0;
    if (it != m_store.end() &&
        !is_expired_entry(it->second))
    {
        // TODO: handle exceptions i.e not a number, out-of-bound
        value = std::stoi(std::get<std::string>(it->second.value));
    }
    value -= 1;
    m_store[key].value = std::to_string(value);
    return value;
}

int DataStore::lpush(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(m_store_mutex);
    auto &entry = m_store[key];

    if (!std::holds_alternative<std::list<std::string>>(entry.value))
    {
        entry.value = std::list<std::string>{};
    }
    auto &list = std::get<std::list<std::string>>(entry.value);
    list.push_front(value);
    return static_cast<int>(list.size());
}

int DataStore::rpush(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(m_store_mutex);
    auto &entry = m_store[key];

    if (!std::holds_alternative<std::list<std::string>>(entry.value))
    {
        entry.value = std::list<std::string>{};
    }
    auto &list = std::get<std::list<std::string>>(entry.value);
    list.push_back(value);
    return static_cast<int>(list.size());
}

std::vector<std::string> DataStore::lrange(const std::string &key, int start, int stop)
{
    std::lock_guard<std::mutex> lock(m_store_mutex);
    auto it = m_store.find(key);

    if (it != m_store.end() &&
        !is_expired_entry(it->second))
    {
        if (std::holds_alternative<std::list<std::string>>(it->second.value))
        {
            auto &list = std::get<std::list<std::string>>(it->second.value);
            std::vector<std::string> result;
            int list_size = static_cast<int>(list.size());

            if (start < 0)
                start = list_size + start;
            if (stop < 0)
                stop = list_size + stop;
            if (start < 0)
                start = 0;
            if (stop >= list_size)
                stop = list_size - 1;

            auto it_start = std::next(list.begin(), start);
            auto it_end = std::next(list.begin(), stop + 1);
            result.assign(it_start, it_end);
            return result;
        }
    }
    return {};
}

bool DataStore::is_expired_entry(const ValueEntry &entry) const
{
    if (!entry.expiry.has_value())
    {
        return false;
    }
    return std::chrono::steady_clock::now() > entry.expiry.value();
}
