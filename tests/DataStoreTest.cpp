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

TEST(DataStoreTest, ListOperations)
{
    DataStore data_store;
    int size = data_store.lpush("mylist", "two");
    EXPECT_EQ(size, 1);
    size = data_store.lpush("mylist", "one");
    EXPECT_EQ(size, 2);
    size = data_store.rpush("mylist", "three");
    EXPECT_EQ(size, 3);

    auto value = data_store.lrange("mylist", 0, -1);
    std::vector<std::string> expected_list = {"one", "two", "three"};
    EXPECT_EQ(value, expected_list);
}

TEST(DataStoreTest, SaveAndLoad)
{
    DataStore data_store;
    data_store.set("key1", "value1");
    data_store.set("key2", "value2", std::chrono::seconds(5));

    bool saved = data_store.save("data.rdb");
    EXPECT_TRUE(saved);

    DataStore new_store;
    bool loaded = new_store.load("data.rdb");
    EXPECT_TRUE(loaded);

    EXPECT_EQ(new_store.get("key1"), "value1");
    EXPECT_EQ(new_store.get("key2"), "value2");
}
