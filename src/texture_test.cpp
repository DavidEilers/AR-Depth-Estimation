#include <iostream>
#include <cstddef>
#include <cinttypes>

#include "texture.hpp"
#include "gtest/gtest.h"
#include "glfw.hpp"



class TextureTestApp : public arDepthEstimation::Application{
    
    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){

    }
    void setup(){
        arDepthEstimation::LinearSampler sampler{};
        uint8_t pixel_data[] =  {255,0,0,255};
        arDepthEstimation::Texture*  texture = new arDepthEstimation::Texture{1,1,
                                            GL_RGBA8,GL_UNSIGNED_BYTE,pixel_data,&sampler,0};
        GLint buffer_id;


        texture->bind();

        glGetIntegerv(GL_TEXTURE_BINDING_2D,&buffer_id);
        ASSERT_NE(buffer_id,0);
        
        texture->unbind();

        GLint temp_buff_id;

        glGetIntegerv(GL_TEXTURE_BINDING_2D,&temp_buff_id);
        ASSERT_EQ(temp_buff_id,0);

        delete texture;

        GLboolean buffer_exists = glIsBuffer(buffer_id);

        ASSERT_EQ(buffer_exists, GL_FALSE);

    }

    void draw(int width, int height){
        throw std::runtime_error("Finished Test");
    }
};

// Test if data in image is accessable through x and y coordinates
TEST(TextureTest, lifeCycle)
{
    TextureTestApp app;
    EXPECT_THROW(arDepthEstimation::ContextManager mgr{&app},std::runtime_error);
}
