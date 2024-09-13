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

