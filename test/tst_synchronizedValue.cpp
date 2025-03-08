#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "concurrency/Synch_Value.h"

using namespace testing;
using namespace NS_dtools;
using namespace NS_concurrency;

TEST(SYNCHRONIZEDVALUE, CreateNewValueUpdateAlways)
{
    Synch_Value<int> val(5);
}

TEST(SYNCHRONIZEDVALUE, CreateNewValueUpdateInOrder)
{
    Synch_Value<int> val(5, UPDATEINORDER);
}

TEST(SYNCHRONIZEDVALUE, GetValueOfNewVal)
{
    Synch_Value<int> val(5);
    int i = val.get();
    ASSERT_EQ(i, 5);
}

TEST(SYNCHRONIZEDVALUE, GetValueOfNewValUpdateInOrder)
{
    Synch_Value<int> val(5, UPDATEINORDER);
    int i = val.get();
    ASSERT_EQ(i, 5);
}

TEST(SYNCHRONIZEDVALUE, BlockingUpdateAlwaysGetIsLastSetValue)
{
    Synch_Value<int> val(0);
    for(int i = 0; i < 200; ++i)
    {
        val.set(i);
    }
    ASSERT_EQ(val.get(), 199);
}

TEST(SYNCHRONIZEDVALUE, BlockingUpdateInOrderGetIsNextSetValue)
{
    Synch_Value<int> val(0, UPDATEINORDER);
    for(int i = 0; i < 200; ++i)
    {
        val.set(i);
    }
    for(int i = 0; i < 200; ++i)
    {
        ASSERT_EQ(val.get(), i);
    }
    ASSERT_EQ(val.get(), 199);
}

