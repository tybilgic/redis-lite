#include <stdexcept>
#include "RESPParser.hpp"

RESPParser::RESPParser(const std::string &data) : m_data(data), m_pos(0)
{
}

bool RESPParser::has_next()
{
    return m_pos < m_data.size();
}

std::vector<std::string> RESPParser::next_command()
{
    std::vector<std::string> command;

    if (m_pos > m_data.size())
    {
        throw std::runtime_error("No more data to parse");
    }

    if (m_data[m_pos] != '*')
    {
        throw std::runtime_error("Expected array");
    }

    m_pos += 1; // skip '*'

    size_t line_end = m_data.find("\r\n", m_pos);
    if (line_end == std::string::npos)
    {
        // Incomplete data
        m_pos -= 1;
        throw std::runtime_error("Incomplete data");
    }

    int array_length = std::stoi(m_data.substr(m_pos, line_end - m_pos));
    m_pos = line_end + 2; // move past "\r\n"

    for (int i = 0; i < array_length; i++)
    {
        // Expect bulk string
        if (m_pos >= m_data.size() || m_data[m_pos] != '$')
        {
            throw std::runtime_error("Expected bulk string");
        }

        m_pos++; // Skip '$'
        line_end = m_data.find("\r\n", m_pos);
        if (line_end == std::string::npos)
        {
            // incomplete
            m_pos -= 1;
            throw std::runtime_error("Incomplete data");
        }

        int str_length = std::stoi(m_data.substr(m_pos, line_end - m_pos));
        m_pos = line_end + 2; // Move past "\r\n"

        if (m_pos + str_length + 2 > m_data.size())
        {
            // incomplete
            m_pos -= (line_end - m_pos) + 3; // Rewind to '$'
            throw std::runtime_error("Incomplete data");
        }

        std::string arg = m_data.substr(m_pos, str_length);
        command.push_back(arg);
        m_pos += str_length + 2; // string + "\r\n"
    }

    return command;
}

std::string RESPParser::get_remaining_data() const
{
    return m_data.substr(m_pos);
}
