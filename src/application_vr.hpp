#pragma once
#include "glfw.hpp"
extern "C"{
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
}

#include <cstddef>

#include "texture.hpp"
#include "init_vr.hpp"

namespace arDepthEstimation{

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
    GLuint offset_loc;


    public:

    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
        logger_info << "Key was pressed!";
    }

    void setup(){
        #define DEBUG_GL
        #ifdef DEBUG_GL
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
        glDebugMessageCallback(MessageCallback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
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

        myShader =  new Shader{vertex_shader_path, fragment_shader_path};

        offset_loc = glGetUniformLocation(myShader->m_program_id,"offset");

        vr = new arDepthEstimation::Vr{};

    }

    void draw(int width, int height){

        logger_info << "draw frame start";

        vr->start_frame();
        
        logger_info << "update texture start";
        vr->update_texture();        
        logger_info << "update texture finished";

        glUseProgram(myShader->m_program_id);
        
        vr->texture->bind();
        glEnable(GL_MULTISAMPLE);

        
        logger_info << "left eye rendering start";
       
        vr->bind_left_eye();
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glUniform1f(offset_loc,0.0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            vr->blit_frame_left();
        
        logger_info << "left eye rendering finished";


        logger_info << "right eye rendering start";

        vr->bind_right_eye();
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glUniform1f(offset_loc,0.5);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            vr->blit_frame_right();

        logger_info << "right eye rendering finished";

        logger_info << "window rendering start";

        vr->bind_window();
        glBindTexture(GL_TEXTURE_2D,vr->get_left_framebuffer_texture_id() );
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, width, height);
            glDrawArrays(GL_TRIANGLES, 0, 6);

        
        logger_info << "window rendering finished";
        
        glDisable(GL_MULTISAMPLE);
        
        glFlush();glFinish();
        vr->texture->unbind();

        logger_info << "vr->submit frames start";
        vr->submit_frames();
        logger_info << "vr->submit frames finished";
        glFinish();

        logger_info << "draw frame finished";
    }

    ~MainApplication(){
        delete vr;
        delete myShader;
    }
};
}