#pragma once
#include "glfw.hpp"
extern "C"
{
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
}

#include <cstddef>

#include "depth_estimation.hpp"
#include "mesh.hpp"
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
    GLuint m_vertex_buffer, m_vertex_array_object;
    arDepthEstimation::Texture *m_texture_left;
    arDepthEstimation::Texture *m_texture_right;
    arDepthEstimation::LinearSampler m_sampler{};
    Shader *m_shader;
    DepthEstimator *m_depth_estimator;

  public:
    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        logger_info << "Key was pressed!";
    }

    void setup()
    {
#ifdef DEBUG_GL
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
#endif

        glGenVertexArrays(1, &m_vertex_array_object);
        glBindVertexArray(m_vertex_array_object);
        glGenBuffers(1, &m_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void *)0);

        glClearColor(0.5f, 0.5f, 0.5, 1.0);

        std::string shader_dir("assets\\shader\\");
        std::string vertex_shader_path(shader_dir + "screen_quad_simple.vert.glsl");
        std::string fragment_shader_path(shader_dir + "screen_quad_simple.frag.glsl");
        // Shader myShader{vertex_shader_path, fragment_shader_path};
        m_shader = new Shader{vertex_shader_path, fragment_shader_path};

        stbi_set_flip_vertically_on_load(true);

        int width, height, channels;
        std::byte *image_data_left =
            (std::byte *)stbi_load("assets\\test_data\\Adirondack-perfect\\im0.png", &width, &height, &channels, 4);
        if (image_data_left == nullptr || channels < 3 || channels > 4)
        {
            logger_error << "couldn't load image!";
            std::runtime_error("couldn't load image!");
        }

        m_sampler.initialize_sampler();
        m_texture_left =
            new arDepthEstimation::Texture{width, height, GL_RGBA8, GL_UNSIGNED_BYTE, image_data_left, &m_sampler, 0};
        stbi_image_free(image_data_left);

        std::byte *image_data_right =
            (std::byte *)stbi_load("assets\\test_data\\Adirondack-perfect\\im1.png", &width, &height, &channels, 4);
        if (image_data_right == nullptr || channels < 3 || channels > 4)
        {
            logger_error << "couldn't load image!";
            std::runtime_error("couldn't load image!");
        }
        m_texture_right =
            new arDepthEstimation::Texture{width, height, GL_RGBA8, GL_UNSIGNED_BYTE, image_data_right, &m_sampler, 0};
        stbi_image_free(image_data_right);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_depth_estimator = new DepthEstimator{width, height, false};
    }

    void draw(int width, int height)
    {

        m_depth_estimator->update_depth_map(m_texture_left->get_texture_id(), m_texture_right->get_texture_id());

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_shader->m_program_id);
        glBindVertexArray(m_vertex_array_object);

        // texture_left->bind();
        GLuint texture_id = m_depth_estimator->get_framebuffer_texture_id();
        m_sampler.bind(0);
        glBindTextureUnit(0, texture_id);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindTextureUnit(0, 0);
        m_sampler.unbind(0);
        // texture_left->unbind();
        glBindVertexArray(0);
    }
};
} // namespace arDepthEstimation