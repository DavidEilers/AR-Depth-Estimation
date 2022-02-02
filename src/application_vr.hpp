#pragma once
#include "glfw.hpp"
extern "C"{
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
}

#include <cstddef>

#include "texture.hpp"
#include "init_vr.hpp"

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
    GLuint vertex_buffer, vertex_array_object;
    Shader* myShader;
    arDepthEstimation::Vr* vr;


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
        //Shader myShader{vertex_shader_path, fragment_shader_path};
        myShader =  new Shader{vertex_shader_path, fragment_shader_path};

        vr = new arDepthEstimation::Vr{};

    }

    void draw(int width, int height){

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        vr->update_texture();

        glUseProgram(myShader->m_program_id);
       
        vr->texture->bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        vr->texture->unbind();
    }

    ~MainApplication(){
        delete vr;
        delete myShader;
    }
};