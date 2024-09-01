#include <stdexcept>

#include "RESPString.hpp"

std::string RESPString::serialize() const { return "+" + value + "\r\n"; }

RESPString RESPString::deserialize(const std::string &data)
{
    if (data.size() < 3 || data[0] != '+' || data.back() != '\n')
    {
        throw std::invalid_argument("Invalid RESP string format!");
    }
    return RESPString(data.substr(1, data.size() - 3));
}