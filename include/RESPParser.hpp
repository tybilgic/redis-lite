#pragma once

#include <string>
#include <vector>

class RESPParser
{
public:
    explicit RESPParser(const std::string &data);

    bool has_next();
    std::vector<std::string> next_command();
    std::string get_remaining_data() const;

private:
    std::string m_data;
    size_t m_pos;
};