#pragma once

#include <glad/gl.h>

#include "log.hpp"
#include "sampler.hpp"
#include "shader.hpp"

namespace arDepthEstimation
{

class Downscaler
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
    int m_per_eye_output_width;
    int m_per_eye_output_height;
    int m_output_height;
    float m_gamma;
    bool m_is_upside_down;
    GLuint m_framebuffer_id;
    GLuint m_framebuffer_right_texture_id;
    GLuint m_framebuffer_left_texture_id;

    void inline create_framebuffer()
    {
        glGenFramebuffers(1, &m_framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);

        glGenTextures(1, &m_framebuffer_left_texture_id);
        glGenTextures(1, &m_framebuffer_right_texture_id);
        glBindTexture(GL_TEXTURE_2D, m_framebuffer_left_texture_id);
        logger_info << "Input Dimension" << m_input_width << m_input_height;
        logger_info << "Output Dimension" << m_output_width << m_output_height;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_per_eye_output_width, m_per_eye_output_height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, m_framebuffer_right_texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_per_eye_output_width, m_per_eye_output_height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_framebuffer_left_texture_id, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_framebuffer_right_texture_id, 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            logger_error << "framebuffer not complete" << status;
            throw std::runtime_error("framebuffer not complete!");
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void inline delete_framebuffer()
    {
        glDeleteFramebuffers(1, &m_framebuffer_id);
        glDeleteTextures(1, &m_framebuffer_right_texture_id);
        glDeleteTextures(1, &m_framebuffer_left_texture_id);
    }

    Shader *m_shader;
    LinearSampler m_sampler_left_eye{};
    LinearSampler m_sampler_right_eye{};
    LinearSampler m_sampler_both_eyes{};
    GLuint m_texture_size_loc;
    GLuint m_is_single_texture_loc;
    GLuint m_gamma_loc;
    GLuint m_is_upside_down_loc;
    GLuint m_is_rgb_loc;
    bool m_is_single_texture;
    bool m_is_rgb;

    void inline create_shader()
    {
        std::string vertex_shader_path{g_asset_path.get_path({"shader","downscale.vert.glsl"}).string()};
        std::string fragment_shader_path{g_asset_path.get_path({"shader","downscale.frag.glsl"}).string()};

        m_shader = new Shader{vertex_shader_path, fragment_shader_path};

        m_sampler_left_eye.initialize_sampler();
        m_sampler_right_eye.initialize_sampler();

        m_texture_size_loc = glGetUniformLocation(m_shader->m_program_id, "texture_size");
        m_is_single_texture_loc = glGetUniformLocation(m_shader->m_program_id, "is_single_texture");
        m_gamma_loc = glGetUniformLocation(m_shader->m_program_id, "gamma");
        m_is_upside_down_loc = glGetUniformLocation(m_shader->m_program_id, "is_upside_down");
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
    Downscaler(int input_width, int input_height, int output_width, int output_height, bool is_single_texture,
               float gamma = 1.0, bool is_upside_down = false, bool is_rgb = false)
        : m_input_width{input_width}, m_input_height{input_height}, m_output_width{output_width},
          m_output_height{output_height}, m_is_single_texture{is_single_texture}, m_gamma{gamma}, m_is_upside_down{
                                                                                                      is_upside_down}, m_is_rgb{is_rgb}
    {
        m_per_eye_output_height = m_output_height;
        if (m_is_single_texture)
        {
            m_per_eye_output_width = m_output_width / 2;
        }
        else
        {
            m_per_eye_output_width = m_output_width;
        }
        create_framebuffer();
        create_shader();
        create_vao();
    }

    ~Downscaler()
    {
        delete_framebuffer();
        delete_shader();
        delete_vao();
    }

    GLuint get_framebuffer_left_texture_id()
    {
        return m_framebuffer_left_texture_id;
    }

    GLuint get_framebuffer_right_texture_id()
    {
        return m_framebuffer_right_texture_id;
    }

    void downscale(GLuint left_eye_texture_id, GLuint right_eye_texture_id)
    {
        glUseProgram(m_shader->m_program_id);
        glUniform2i(m_texture_size_loc, m_input_width, m_input_height);
        glUniform1i(m_is_single_texture_loc, GL_FALSE);
        glUniform1i(m_is_upside_down_loc, m_is_upside_down ? (GL_TRUE) : (GL_FALSE));
        glUniform1i(m_is_rgb_loc, m_is_rgb ? (GL_TRUE) : (GL_FALSE));
        glUniform1f(m_gamma_loc, m_gamma);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
        glViewport(0, 0, m_per_eye_output_width, m_per_eye_output_height);
        // glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(m_vao);
        m_sampler_left_eye.bind(0);
        m_sampler_right_eye.bind(1);
        glBindTextureUnit(0, left_eye_texture_id);
        glBindTextureUnit(1, right_eye_texture_id);
        const GLenum attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, attachments);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        const GLenum attachments_after[] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, attachments_after);

        glBindTextureUnit(0, 0);
        glBindTextureUnit(1, 0);
        m_sampler_left_eye.unbind(0);
        m_sampler_right_eye.unbind(1);
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
    }

    void downscale(GLuint both_eye_texture_id)
    {
        glUseProgram(m_shader->m_program_id);
        glUniform2i(m_texture_size_loc, m_input_width, m_input_height);
        glUniform1i(m_is_single_texture_loc, GL_TRUE);
        glUniform1i(m_is_upside_down_loc, m_is_upside_down ? (GL_TRUE) : (GL_FALSE));
        glUniform1f(m_gamma_loc, m_gamma);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
        glViewport(0, 0, m_per_eye_output_width, m_per_eye_output_height);
        // glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(m_vao);
        m_sampler_both_eyes.bind(2);
        glBindTextureUnit(2, both_eye_texture_id);
        const GLenum attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, attachments);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        const GLenum attachments_after[] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, attachments_after);

        glBindTextureUnit(2, 0);
        m_sampler_both_eyes.unbind(2);
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(0);
    }
};
} // namespace arDepthEstimation