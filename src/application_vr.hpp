#pragma once
#include "glfw.hpp"
extern "C"
{
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
}

#include <cstddef>

#include "depth_estimation.hpp"
#include "init_vr.hpp"
#include "mesh.hpp"
#include "sampler.hpp"
#include "texture.hpp"

namespace arDepthEstimation
{

class MainApplication : public Application
{

    struct Vertex
    {
        float x, y;
    };
    // A screen quad in OpenGL [-1, 1] x [-1, 1] screen-space
    Vertex vertices[6] = {
        {-1, -1}, // top left
        {-1, 1},  // lower left
        {1, -1},  // top right
        {-1, 1},  // lower left
        {1, 1},   // lower right
        {1, -1}   // upper right
    };
    GLuint m_vertex_buffer, m_vao;
    Shader *m_shader;
    Mesh *m_cube_mesh;
    GLuint m_offset_loc;
    GLuint m_transform_loc;
    GLuint m_is_upside_down_loc;
    arDepthEstimation::Vr *m_vr;
    glm::mat4 m_identity_mat{1.0f};
    DepthEstimator *m_depth_estimator;
    LinearSampler m_sampler{};

  public:
    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        logger_info << "Key was pressed!";
    }

    void setup()
    {
#define DEBUG_GL
#ifdef DEBUG_GL
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(MessageCallback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        glGenBuffers(1, &m_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void *)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glClearColor(0.5f, 0.5f, 0.5, 1.0);

        std::string shader_dir("assets\\shader\\");
        std::string vertex_shader_path(shader_dir + "screen_quad.vert.glsl");
        std::string fragment_shader_path(shader_dir + "screen_quad.frag.glsl");

        m_shader = new Shader{vertex_shader_path, fragment_shader_path};

        m_offset_loc = glGetUniformLocation(m_shader->m_program_id, "offset");
        m_transform_loc = glGetUniformLocation(m_shader->m_program_id, "transform");
        m_is_upside_down_loc = glGetUniformLocation(m_shader->m_program_id, "is_upside_down");

        m_vr = new Vr{};
        m_cube_mesh = new Mesh{};
        int camera_feed_width = m_vr->m_texture->get_width();
        int camera_feed_height = m_vr->m_texture->get_height();
        m_depth_estimator = new DepthEstimator{camera_feed_width, camera_feed_height, false, 1.6, true};
        m_sampler.initialize_sampler();
    }

    void draw(int width, int height)
    {
        draw_vr();
        draw_window(width, height);
        glFlush();
        glFinish();
    }

    void inline draw_window(int width, int height)
    {
        m_vr->bind_window();
        glUseProgram(m_shader->m_program_id);
        glBindVertexArray(m_vao);
        m_sampler.bind(0);
        // glBindTexture(GL_TEXTURE_2D,vr->get_left_framebuffer_texture_id() );
        // glBindTexture(GL_TEXTURE_2D,depth_estimator->get_framebuffer_texture_id());
        glBindTextureUnit(0, m_depth_estimator->get_framebuffer_texture_id());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniformMatrix4fv(m_transform_loc, 1, GL_FALSE, glm::value_ptr(m_identity_mat));
        glUniform1i(m_is_upside_down_loc, GL_FALSE);
        glViewport(0, 0, width, height);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindTextureUnit(0, 0);
        m_sampler.unbind(0);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void inline draw_vr()
    {
        m_vr->start_frame();
        m_vr->update_texture();
        m_depth_estimator->update_depth_map(m_vr->m_texture->get_texture_id(), m_vr->m_texture->get_texture_id());
        m_vr->update_camera_transform_matrix();

        m_vr->m_texture->bind();
        glEnable(GL_MULTISAMPLE);

        m_vr->bind_left_eye();
        glUseProgram(m_shader->m_program_id);
        glBindVertexArray(m_vao);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniform1f(m_offset_loc, 0.0);
        glUniform1i(m_is_upside_down_loc, GL_TRUE);
        glUniformMatrix4fv(m_transform_loc, 1, GL_FALSE, glm::value_ptr(m_identity_mat));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glUseProgram(0);
        m_cube_mesh->draw();
        m_vr->blit_frame_left();

        m_vr->bind_right_eye();
        glUseProgram(m_shader->m_program_id);
        glBindVertexArray(m_vao);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniform1f(m_offset_loc, 0.5);
        glUniform1i(m_is_upside_down_loc, GL_TRUE);
        glUniformMatrix4fv(m_transform_loc, 1, GL_FALSE, glm::value_ptr(m_identity_mat));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glUseProgram(0);
        m_cube_mesh->draw();
        m_vr->blit_frame_right();

        glDisable(GL_MULTISAMPLE);

        m_vr->m_texture->unbind();
        m_vr->submit_frames();
        glFinish();
    }

    ~MainApplication()
    {
        delete m_vr;
        delete m_shader;
        delete m_depth_estimator;
    }
};
} // namespace arDepthEstimation