//
// Created by Sakata on 05.06.2025.
//

#include "gtest/gtest.h"

TEST(ExampleTest, BasicTest1)
{
    EXPECT_EQ(4+5, 9);
}

TEST(ExampleTest, BasicTest2)
{
    EXPECT_EQ(5+3, 8);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}