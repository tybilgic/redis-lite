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

TEST(DataStoreTest, EXISTS)
{
    DataStore data_store;
    data_store.set("key", "value");
    EXPECT_TRUE(data_store.exists("key"));
    EXPECT_FALSE(data_store.exists("no-key"));
}

TEST(DataStoreTest, DEL)
{
    DataStore data_store;
    data_store.set("key", "value");
    bool deleted = data_store.del("key");
    EXPECT_TRUE(deleted);
    EXPECT_FALSE(data_store.exists("key"));
    deleted = data_store.del("no-key");
    EXPECT_FALSE(deleted);
}

TEST(DataStoreTest, INCR_DECR)
{
    DataStore data_store;
    data_store.set("key", "10");
    int value = data_store.incr("key");
    EXPECT_EQ(value, 11);
    value = data_store.decr("key");
    EXPECT_EQ(value, 10);
}
