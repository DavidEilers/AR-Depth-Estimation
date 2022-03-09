#pragma once
#include "glfw.hpp"
extern "C"{
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
}

#include <cstddef>

#include "texture.hpp"
#include "init_vr.hpp"
#include "mesh.hpp"
#include "depth_estimation.hpp"
#include "sampler.hpp"

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
    GLuint vertex_buffer, vao;
    Shader* myShader;
    Mesh* cubeMesh;
    GLuint offset_loc;
    GLuint transform_loc;
    arDepthEstimation::Vr* vr;
    glm::mat4 identity_mat{1.0f};
    DepthEstimator * depth_estimator;
    LinearSampler sampler{};
    


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

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void *)0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindVertexArray(0);

        glClearColor(0.5f, 0.5f, 0.5, 1.0);

        std::string shader_dir("assets\\shader\\");
        std::string vertex_shader_path(shader_dir + "screen_quad.vert.glsl");
        std::string fragment_shader_path(shader_dir + "screen_quad.frag.glsl");

        myShader =  new Shader{vertex_shader_path, fragment_shader_path};

        offset_loc = glGetUniformLocation(myShader->m_program_id,"offset");
        transform_loc = glGetUniformLocation(myShader->m_program_id,"transform");

        vr = new Vr{};
        cubeMesh = new Mesh{};
        int camera_feed_width = vr->texture->get_width();
        int camera_feed_height = vr->texture->get_height();
        depth_estimator = new DepthEstimator{camera_feed_width,camera_feed_height,false,1.6};
        sampler.initialize_sampler();

    }

    void draw(int width, int height){
        draw_vr();
        draw_window(width, height);
        glFlush();
        glFinish();
    }

    void inline draw_window(int width, int height){
        vr->bind_window();
        glUseProgram(myShader->m_program_id);
        glBindVertexArray(vao);
        sampler.bind(0);
        //glBindTexture(GL_TEXTURE_2D,vr->get_left_framebuffer_texture_id() );
        //glBindTexture(GL_TEXTURE_2D,depth_estimator->get_framebuffer_texture_id());
        glBindTextureUnit(0,depth_estimator->get_framebuffer_texture_id());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUniformMatrix4fv(transform_loc,1,GL_FALSE,glm::value_ptr(identity_mat));
            glViewport(0, 0, width, height);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindTextureUnit(0,0);
        sampler.unbind(0);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void inline draw_vr(){ 
        vr->start_frame();
        vr->update_texture();
        depth_estimator->update_depth_map(vr->texture->get_texture_id(),vr->texture->get_texture_id());
        vr->update_camera_transform_matrix();
        
        vr->texture->bind();
        glEnable(GL_MULTISAMPLE);
       
        vr->bind_left_eye();
            glUseProgram(myShader->m_program_id);
            glBindVertexArray(vao);
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glUniform1f(offset_loc,0.0);
            glUniformMatrix4fv(transform_loc,1,GL_FALSE,glm::value_ptr(identity_mat));
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            glUseProgram(0);
            cubeMesh->draw();
            vr->blit_frame_left();


        vr->bind_right_eye();
            glUseProgram(myShader->m_program_id);
            glBindVertexArray(vao);
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glUniform1f(offset_loc,0.5);
            glUniformMatrix4fv(transform_loc,1,GL_FALSE,glm::value_ptr(identity_mat));
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            glUseProgram(0);
            cubeMesh->draw();
            vr->blit_frame_right();
        
        glDisable(GL_MULTISAMPLE);
        
        vr->texture->unbind();
        vr->submit_frames();
        glFinish();

    }

    ~MainApplication(){
        delete vr;
        delete myShader;
        delete depth_estimator;
    }
};
}