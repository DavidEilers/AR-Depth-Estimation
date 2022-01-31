#pragma once
#include "glfw.hpp"
extern "C"{
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
}

#include <cstddef>

class MainApplication : public Application{

 
    struct Vertex
    {
        float x, y;
    };

    //A screen quad in OpenGL [-1, 1] x [-1, 1] screen-space
    Vertex vertices[6] = 
    {// top-left     lower left    top right
        {-1, -1},    {-1, 1},      {1, -1},
    //  lower left   lower right   upper right
        {-1, 1},     {1, 1},       {1, -1}
    };
    GLuint vertex_buffer, vertex_shader, fragment_shader, program, vertex_array_object;
    GLuint image_sampler, image_id;


    public:

    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
        logger_info << "Key was pressed!";
    }

    void setup(){
        
        #ifdef DEBUG_GL
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
        #endif

        glGenVertexArrays(1, &vertex_array_object);
        glBindVertexArray(vertex_array_object);
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void *)0);

        glClearColor(0.5f, 0.5f, 0.5, 1.0);

        std::string shader_dir("assets\\shader\\");
        std::string vertex_shader_path(shader_dir + "screen_quad.vert.glsl");
        std::string fragment_shader_path(shader_dir + "screen_quad.frag.glsl");
        Shader myShader{vertex_shader_path, fragment_shader_path};
        vertex_shader = myShader.m_vertex_shader_id;
        fragment_shader = myShader.m_fragment_shader_id;
        program = myShader.m_program_id;

        glGenSamplers(1,&image_sampler);
        glSamplerParameteri(image_sampler,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(image_sampler,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(image_sampler,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(image_sampler,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(image_sampler,GL_TEXTURE_MAX_LEVEL, 0);

        glGenTextures(1,&image_id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, image_id);

        int width, height, channels;
        std::byte *image_data =(std::byte*) stbi_load("assets\\test_data\\Adirondack-perfect\\im0.png", &width, &height, &channels, 4);
        if(image_data == nullptr || channels < 3 || channels > 4){
            logger_error << "couldn't load image!";
            std::runtime_error("couldn't load image!");
        }

        glBindSampler(0, image_sampler);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        stbi_image_free(image_data);

    }

    void draw(int width, int height){

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
       
        glBindSampler(GL_TEXTURE0, image_sampler);
        glBindTexture(GL_TEXTURE_2D, image_id);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};