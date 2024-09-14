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
        return it->second.value;
    }

    return "";
}

bool DataStore::is_expired_entry(const ValueEntry &entry) const
{
    if (!entry.expiry.has_value())
    {
        return false;
    }
    return std::chrono::steady_clock::now() > entry.expiry.value();
}
