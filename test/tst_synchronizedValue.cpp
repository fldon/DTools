#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "SynchronizedValue.h"

using namespace testing;
using namespace SynchonizedValue;

TEST(SYNCHRONIZEDVALUE, CreateNewValueUpdateAlways)
{
    SynchronizedValue<int> val(5);
}

TEST(SYNCHRONIZEDVALUE, CreateNewValueUpdateInOrder)
{
    SynchronizedValue<int> val(5, UPDATEINORDER);
}

TEST(SYNCHRONIZEDVALUE, GetValueOfNewVal)
{
    SynchronizedValue<int> val(5);
    int i = val.get();
    ASSERT_EQ(i, 5);
}

TEST(SYNCHRONIZEDVALUE, GetValueOfNewValUpdateInOrder)
{
    SynchronizedValue<int> val(5, UPDATEINORDER);
    int i = val.get();
    ASSERT_EQ(i, 5);
}

TEST(SYNCHRONIZEDVALUE, BlockingUpdateAlwaysGetIsLastSetValue)
{
    SynchronizedValue<int> val(0);
    for(int i = 0; i < 200; ++i)
    {
        val.set(i);
    }
    ASSERT_EQ(val.get(), 199);
}

TEST(SYNCHRONIZEDVALUE, BlockingUpdateInOrderGetIsNextSetValue)
{
    SynchronizedValue<int> val(0, UPDATEINORDER);
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

