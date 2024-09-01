#include "../include/RESPString.hpp"
#include <gtest/gtest.h>

TEST(RESPStringTest, Serialize)
{
    RESPString resp("OK");
    EXPECT_EQ(resp.serialize(), "+OK\r\n");
}

TEST(RESPStringTest, Deserialize)
{
    RESPString resp = RESPString::deserialize("+OK\r\n");
    EXPECT_EQ(resp.serialize(), "+OK\r\n");
}