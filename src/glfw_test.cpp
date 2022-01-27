#include "glfw.hpp"
#include "gtest/gtest.h"

TEST(GLFWTest, simpleProgram)
{
    EXPECT_EXIT(ContextManager a{}, testing::ExitedWithCode(EXIT_SUCCESS), "");
}