#pragma once
#include "glfw.hpp"
extern "C"
{
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
}

#include <cstddef>
#include <filesystem>
#include <tuple>
#include <utility>

#include "depth_estimation.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "frametime_probe.hpp"

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
    FrameTimeProbe m_ftp_depth_estimation{"Depth estimation total"};
    FrameTimeProbe m_ftp_test{"test"};
    std::pair<std::vector<arDepthEstimation::Texture>,std::vector<arDepthEstimation::Texture>> m_stereo_image_pairs{};
    std::vector<std::string> m_stereo_image_pair_names{};
    int frame_counter = 0;
    size_t m_stereo_image_iterator{0};
    bool m_first{false};

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

        std::string vertex_shader_path{g_asset_path.get_path({"shader","screen_quad_simple.vert.glsl"}).string() };
        std::string fragment_shader_path{g_asset_path.get_path({"shader","screen_quad_simple.frag.glsl"}).string()};
        // Shader myShader{vertex_shader_path, fragment_shader_path};
        m_shader = new Shader{vertex_shader_path, fragment_shader_path};

        stbi_set_flip_vertically_on_load(true);

        std::filesystem::path test_data_path{g_asset_path.get_path({"test_data"})};
        const size_t test_data_path_length = test_data_path.string().length() +1;

        
        m_sampler.initialize_sampler();
        int i = 0;

        m_stereo_image_pairs.first.reserve(23);
        m_stereo_image_pairs.second.reserve(23);


        for(const auto& entry : std::filesystem::directory_iterator(test_data_path)){
            m_stereo_image_pair_names.emplace_back(entry.path().string().substr(test_data_path_length));
            logger_info << "Loading : " << m_stereo_image_pair_names.back();
            std::filesystem::path left_image_path = entry;
            left_image_path /= "im0.png";
            int width, height, channels;
            std::byte *image_data_left =
                (std::byte *)stbi_load(left_image_path.string().c_str(), &width, &height, &channels, 4);
            if (image_data_left == nullptr || channels < 3 || channels > 4)
            {
                logger_error << "couldn't load image!";
                std::runtime_error("couldn't load image!");
            }
            
        
            
            std::filesystem::path right_image_path = entry;
            right_image_path /= "im1.png";
            std::byte *image_data_right =
                (std::byte *)stbi_load(right_image_path.string().c_str(), &width, &height, &channels, 4);
            if (image_data_right == nullptr || channels < 3 || channels > 4)
            {
                logger_error << "couldn't load image!";
                std::runtime_error("couldn't load image!");
            } 
            
            m_stereo_image_pairs.first.emplace_back(static_cast<size_t>(width), static_cast<size_t>(height), GL_RGBA8, GL_UNSIGNED_BYTE, image_data_left, &m_sampler, 0);
            m_stereo_image_pairs.second.emplace_back(static_cast<size_t>(width), static_cast<size_t>(height), GL_RGBA8, GL_UNSIGNED_BYTE, image_data_right, &m_sampler, 0);
            stbi_image_free(image_data_left);
            stbi_image_free(image_data_right);
            //if(i==1){break;}
            i++;

        }


        /*int width, height, channels;
        std::byte *image_data_left =
            (std::byte *)stbi_load("assets\\test_data\\Adirondack-perfect\\im0.png", &width, &height, &channels, 4);
        if (image_data_left == nullptr || channels < 3 || channels > 4)
        {
            logger_error << "couldn't load image!";
            std::runtime_error("couldn't load image!");
        }

        m_sampler.initialize_sampler();
        m_texture_left =
            new arDepthEstimation::Texture{static_cast<size_t>(width), static_cast<size_t>(height), GL_RGBA8, GL_UNSIGNED_BYTE, image_data_left, &m_sampler, 0};
        stbi_image_free(image_data_left);

        std::byte *image_data_right =
            (std::byte *)stbi_load("assets\\test_data\\Adirondack-perfect\\im1.png", &width, &height, &channels, 4);
        if (image_data_right == nullptr || channels < 3 || channels > 4)
        {
            logger_error << "couldn't load image!";
            std::runtime_error("couldn't load image!");
        }
        m_texture_right =
            new arDepthEstimation::Texture{static_cast<size_t>(width), static_cast<size_t>(height), GL_RGBA8, GL_UNSIGNED_BYTE, image_data_right, &m_sampler, 0};
        stbi_image_free(image_data_right);*/

        m_texture_left = &(m_stereo_image_pairs.first[0]);
        m_texture_right = &(m_stereo_image_pairs.second[0]);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_depth_estimator = new DepthEstimator{static_cast<int>(m_stereo_image_pairs.first[0].get_width()), static_cast<int>(m_stereo_image_pairs.first[0].get_height()), false};
    }

    void draw(int width, int height)
    {
        m_ftp_depth_estimation.start();
        m_depth_estimator->update_depth_map(m_stereo_image_pairs.first[m_stereo_image_iterator].get_texture_id(), m_stereo_image_pairs.second[m_stereo_image_iterator].get_texture_id());
        m_ftp_depth_estimation.stop();

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
        m_ftp_test.start();
        m_ftp_test.stop();
        frame_counter++;
         if(frame_counter%500==0){
            if(m_first == true){
                logger_frametime << m_ftp_depth_estimation.to_csv_line(m_stereo_image_pair_names[m_stereo_image_iterator]); m_ftp_depth_estimation.reset();
                //logger_frametime << m_depth_estimator->get_ftp_downscale().to_string();m_depth_estimator->get_ftp_downscale().reset();
                //logger_frametime << m_ftp_test.to_string();m_ftp_test.reset();
                //logger_frametime << "------------------------------";
                if(m_stereo_image_iterator >= m_stereo_image_pairs.first.size()-1){
                    g_context->close_window();
                    return;
                }
                m_stereo_image_iterator++;
                m_first = false;
            }else{
                m_ftp_depth_estimation.reset(); m_first = true;
            }
        }
    }
};
} // namespace arDepthEstimation