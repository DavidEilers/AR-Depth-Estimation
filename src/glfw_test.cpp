#include "glfw.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using ::testing::AtLeast;

struct TestApp: public arDepthEstimation::Application{
    MOCK_METHOD(void, key_callback, (GLFWwindow *window, int key, int scancode, int action, int mods), (override));
    MOCK_METHOD(void, setup, (), (override));
    MOCK_METHOD(void, draw, (int width, int height), (override));
};

TEST(GLFWTest, simpleProgram)
{ 
    TestApp myApp;
    EXPECT_CALL(myApp, setup()).Times(1);
    EXPECT_CALL(myApp, draw(testing::_, testing::_)).Times(AtLeast(1));
    arDepthEstimation::run_app(&myApp);
}