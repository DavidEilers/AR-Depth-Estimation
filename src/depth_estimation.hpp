#pragma once

#include <glad/gl.h>

#include "downscale.hpp"
#include "log.hpp"
#include "sampler.hpp"
#include "shader.hpp"

namespace arDepthEstimation
{

class DepthEstimator
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
    int m_input_width;
    int m_input_height;
    int m_output_width;
    int m_output_height;
    bool m_is_rgb;
    GLuint m_framebuffer_id;
    GLuint m_framebuffer_texture_id;
    Downscaler *m_downscaler;

    void inline create_framebuffer()
    {
        glGenFramebuffers(1, &m_framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);

        glGenTextures(1, &m_framebuffer_texture_id);
        glBindTexture(GL_TEXTURE_2D, m_framebuffer_texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_output_width, m_output_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_framebuffer_texture_id, 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            throw std::runtime_error("framebuffer not complete!");
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void inline delete_framebuffer()
    {
        glDeleteFramebuffers(1, &m_framebuffer_id);
        glDeleteTextures(1, &m_framebuffer_texture_id);
    }

    Shader *m_shader;
    LinearSampler m_sampler_left_eye{};
    LinearSampler m_sampler_right_eye{};
    GLuint m_texture_size_loc;
    GLuint m_is_rgb_loc;

    void inline create_shader()
    {
        std::string shader_dir("assets\\shader\\");
        std::string vertex_shader_path(shader_dir + "depth_estimation.vert.glsl");
        std::string fragment_shader_path(shader_dir + "depth_estimation.frag.glsl");

        m_shader = new Shader{vertex_shader_path, fragment_shader_path};

        m_sampler_left_eye.initialize_sampler();
        m_sampler_right_eye.initialize_sampler();

        m_texture_size_loc = glGetUniformLocation(m_shader->m_program_id, "texture_size");
        m_is_rgb_loc = glGetUniformLocation(m_shader->m_program_id, "is_rgb");

        logger_info << "texture size Loc = " << m_texture_size_loc;
    }

    void inline delete_shader()
    {
        delete m_shader;
    }

    GLuint m_vao;
    GLuint m_vbo;

    void inline create_vao()
    {
        glGenVertexArrays(1, &m_vao), glGenBuffers(1, &m_vbo);
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void inline delete_vao()
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
    }

  public:
    DepthEstimator(int input_width, int input_height, bool is_single_texture, float gamma = 1.0,
                   bool is_upside_down = false, bool is_rgb = false)
        : m_input_width{input_width}, m_input_height{input_height}, m_output_width{input_width / 4},
          m_output_height{input_height / 4}, m_is_rgb{is_rgb}
    {
        create_framebuffer();
        create_shader();
        create_vao();
        logger_info << "Depth Estimatior" << m_input_width / 2 << m_input_height / 2 << m_output_width
                    << m_output_height;
        m_downscaler = new Downscaler{m_input_width / 2, m_input_height / 2, m_output_width,
                                      m_output_height,   is_single_texture,  gamma,
                                      is_upside_down, is_rgb};
    }

    ~DepthEstimator()
    {
        delete_framebuffer();
        delete_shader();
        delete_vao();
        delete m_downscaler;
    }

    GLuint get_framebuffer_texture_id()
    {
        return m_framebuffer_texture_id;
    }

    void update_depth_map(GLuint left_eye_texture_id, GLuint right_eye_texture_id)
    {
        m_downscaler->downscale(left_eye_texture_id, right_eye_texture_id);
        GLuint texture_left = m_downscaler->get_framebuffer_left_texture_id();
        GLuint texture_right = m_downscaler->get_framebuffer_right_texture_id();
        glUseProgram(m_shader->m_program_id);
        glUniform2i(m_texture_size_loc, m_output_width, m_output_height);
        glUniform1i(m_is_rgb_loc, m_is_rgb ? (GL_TRUE) : (GL_FALSE));
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
        glViewport(0, 0, m_output_width, m_output_height);
        // glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(m_vao);
        m_sampler_left_eye.bind(0);
        m_sampler_right_eye.bind(1);
        glBindTextureUnit(0, texture_left);
        glBindTextureUnit(1, texture_right);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindTextureUnit(0, 0);
        glBindTextureUnit(1, 0);
        m_sampler_left_eye.unbind(0);
        m_sampler_right_eye.unbind(1);
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
    }

    void update_depth_map(GLuint both_eyes_texture_id)
    {
        m_downscaler->downscale(both_eyes_texture_id);
        GLuint texture_left = m_downscaler->get_framebuffer_left_texture_id();
        GLuint texture_right = m_downscaler->get_framebuffer_right_texture_id();
        glUseProgram(m_shader->m_program_id);
        glUniform2i(m_texture_size_loc, m_output_width / 2, m_output_height);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
        glViewport(0, 0, m_output_width, m_output_height);
        // glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(m_vao);
        m_sampler_left_eye.bind(0);
        m_sampler_right_eye.bind(1);
        glBindTextureUnit(0, texture_left);
        glBindTextureUnit(1, texture_right);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindTextureUnit(0, 0);
        glBindTextureUnit(1, 0);
        m_sampler_left_eye.unbind(0);
        m_sampler_right_eye.unbind(1);
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
    }

    GLuint get_luminance_camera_left(){
        return m_downscaler->get_framebuffer_left_texture_id();
    }
    GLuint get_luminance_camera_right(){
        return m_downscaler->get_framebuffer_right_texture_id();
    }
};
} // namespace arDepthEstimation