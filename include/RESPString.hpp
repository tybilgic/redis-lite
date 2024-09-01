#pragma once
#include <string>

class RESPString
{
public:
    explicit RESPString(const std::string &value) : value(value) {}
    std::string serialize() const; // Converts to RESP format "+value\r\n"
    static RESPString deserialize(const std::string &data);

private:
    std::string value;
};