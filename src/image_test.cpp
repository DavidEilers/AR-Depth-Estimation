#include <iostream>

#include "image.hpp"
#include "gtest/gtest.h"

class ImageTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }

    uint8_t data[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    arDepthEstimation::Image<uint8_t> test_image{3, 3, data};
};

// Test if data in image is accessable through x and y coordinates
TEST_F(ImageTest, dataIntegrity)
{

    for (int i = 0; i < 9; i++)
    {
        ASSERT_EQ(test_image.at(i % 3, i / 3), i);
    }
}

// If Image coordinates are out of bounds should throw error
TEST_F(ImageTest, outOfBoundAccess)
{
    EXPECT_THROW(test_image.at(6, 3), std::out_of_range);
    EXPECT_THROW(test_image.at(3, 6), std::out_of_range);
    EXPECT_THROW(test_image.at(6, 6), std::out_of_range);
}