#include <fstream>

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

bool DataStore::save(const std::string &filename)
{
    std::lock_guard<std::mutex> lock(m_store_mutex);

    std::ofstream ofs(filename, std::ios::binary);

    if (!ofs)
    {
        return false;
    }

    size_t store_size = m_store.size();

    ofs.write(reinterpret_cast<const char *>(&store_size), sizeof(store_size));

    for (const auto &[key, entry] : m_store)
    {
        size_t key_size = key.size();
        ofs.write(reinterpret_cast<const char *>(&key_size), sizeof(key_size));
        ofs.write(key.data(), key_size);

        if (std::holds_alternative<std::string>(entry.value))
        {
            char type = 0; // type 0 for string
            ofs.write(&type, sizeof(type));

            auto &value = std::get<std::string>(entry.value);
            size_t value_size = value.size();
            ofs.write(reinterpret_cast<const char *>(&value_size), sizeof(value_size));
            ofs.write(value.data(), value_size);
        }

        bool has_expiry = entry.expiry.has_value();
        ofs.write(reinterpret_cast<const char *>(&has_expiry), sizeof(has_expiry));

        if (has_expiry)
        {
            auto expiry_time = entry.expiry.value().time_since_epoch().count();
            ofs.write(reinterpret_cast<const char *>(&expiry_time), sizeof(expiry_time));
        }
    }
    return true;
}

bool DataStore::load(const std::string &filename)
{
    std::lock_guard<std::mutex> lock(m_store_mutex);
    std::ifstream ifs(filename, std::ios::binary);

    if (!ifs)
    {
        return false;
    }

    m_store.clear();

    size_t store_size;

    ifs.read(reinterpret_cast<char *>(&store_size), sizeof(store_size));

    for (size_t i = 0; i < store_size; i++)
    {
        size_t key_size;
        ifs.read(reinterpret_cast<char *>(&key_size), sizeof(key_size));
        std::string key(key_size, '\0');
        ifs.read(&key[0], key_size);

        char type;
        ifs.read(&type, sizeof(type));
        ValueEntry entry;
        if (type == 0) // String
        {
            size_t value_size;
            ifs.read(reinterpret_cast<char *>(&value_size), sizeof(value_size));
            std::string value(value_size, '\0');
            ifs.read(&value[0], value_size);
            entry.value = value;
        }

        bool has_expiry;
        ifs.read(reinterpret_cast<char *>(&has_expiry), sizeof(has_expiry));
        if (has_expiry)
        {
            int64_t expiry_time_count;
            ifs.read(reinterpret_cast<char *>(&expiry_time_count), sizeof(expiry_time_count));
            entry.expiry = std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration(expiry_time_count));
        }

        m_store[key] = std::move(entry);
    }
    return true;
}

bool DataStore::is_expired_entry(const ValueEntry &entry) const
{
    if (!entry.expiry.has_value())
    {
        return false;
    }
    return std::chrono::steady_clock::now() > entry.expiry.value();
}
