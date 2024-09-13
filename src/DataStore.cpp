#include "DataStore.hpp"

void DataStore::set(const std::string &key, const std::string &value)
{
    m_store[key] = value;
}

std::string DataStore::get(const std::string &key) const
{
    auto it = m_store.find(key);

    if (it != m_store.end())
    {
        return it->second;
    }

    return "";
}