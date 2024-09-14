#include <chrono>
#include <thread>
#include <gtest/gtest.h>
#include "../include/DataStore.hpp"

TEST(DataStoreTest, SET)
{
    DataStore data_store;
    std::string key = "key1";
    std::string value = "value1";
    data_store.set(key, value);

    EXPECT_EQ(data_store.get(key), value);
}

TEST(DataStoreTest, GET)
{
    DataStore data_store;
    std::string key = "key1";
    std::string value = "value1";

    EXPECT_EQ(data_store.get(key), "");

    data_store.set(key, value);
    EXPECT_EQ(data_store.get(key), value);
}

TEST(DataStoreTest, SETWithExpiry)
{
    DataStore data_store;
    data_store.set("key", "value", std::chrono::seconds(2));
    auto value = data_store.get("key");
    EXPECT_EQ(value, "value");

    std::this_thread::sleep_for(std::chrono::seconds(3));
    value = data_store.get("key");
    EXPECT_EQ(value, "");
}

