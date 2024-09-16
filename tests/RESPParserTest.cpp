#include <string>
#include <vector>
#include <stdexcept>
#include <gtest/gtest.h>

#include "../include/RESPParser.hpp"

TEST(RESPParserTest, ParseSimpleCommand)
{
    std::string data = "*1\r\n$4\r\nPING\r\n";
    RESPParser parser(data);

    EXPECT_TRUE(parser.has_next());
    auto command = parser.next_command();
    EXPECT_EQ(command.size(), 1);
    EXPECT_EQ(command[0], "PING");

    EXPECT_FALSE(parser.has_next());
    EXPECT_EQ(parser.get_remaining_data(), "");
}

TEST(RESPParserTest, ParseCommandWithArguments)
{
    std::string data = "*2\r\n$4\r\nECHO\r\n$5\r\nHello\r\n";
    RESPParser parser(data);

    EXPECT_TRUE(parser.has_next());
    auto command = parser.next_command();
    EXPECT_EQ(command.size(), 2);
    EXPECT_EQ(command[0], "ECHO");
    EXPECT_EQ(command[1], "Hello");

    EXPECT_FALSE(parser.has_next());
    EXPECT_EQ(parser.get_remaining_data(), "");
}
