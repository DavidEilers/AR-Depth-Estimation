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
#include "application_window_vr.hpp"
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
    GLuint m_vertex_buffer, m_vao;
    Shader *m_shader;
    Mesh *m_cube_mesh;
    GLuint m_offset_loc;
    GLuint m_transform_loc;
    GLuint m_cam_proj_loc;
    GLuint m_eye_unproj_loc;
    GLuint m_eye_to_left_cam_loc;
    GLuint m_eye_to_right_cam_loc;
    GLuint m_hmd_to_cam_loc;
    GLuint m_is_upside_down_loc;
    GLuint m_is_left_loc;
    arDepthEstimation::Vr *m_vr;
    glm::mat4 m_identity_mat{1.0f};
    DepthEstimator *m_depth_estimator;
    LinearSampler m_sampler{};
    WindowRenderer *m_window_renderer;
    LinearSampler m_disparity_sampler{};

    double frametime_VR;
    FrameTimeProbe ftp_totalFrameTime{"Total FrameTime"};
    FrameTimeProbe ftp_gettingCameraFrame{"Getting OpenVR camera frame"};
    FrameTimeProbe ftp_depthEstimator{"Depth estimation"};
    FrameTimeProbe ftp_renderingLeft{"Left eye rendering"};
    FrameTimeProbe ftp_renderingRight{"Right eye rendering"};
    FrameTimeProbe ftp_submitFrame{"Submiting eye frames to OpenVR"};
    double frametime_renderingLeft;
    double frametime_renderingRight;
    double frametime_submit;
    int frame_counter = 0;

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
        m_cam_proj_loc = glGetUniformLocation(m_shader->m_program_id, "cam_proj");
        m_eye_unproj_loc = glGetUniformLocation(m_shader->m_program_id, "eye_unproj");
        m_eye_to_left_cam_loc = glGetUniformLocation(m_shader->m_program_id, "eye_to_left_cam");
        m_eye_to_right_cam_loc = glGetUniformLocation(m_shader->m_program_id, "eye_to_right_cam");
        m_hmd_to_cam_loc = glGetUniformLocation(m_shader->m_program_id, "hmd_to_cam");
        m_is_upside_down_loc = glGetUniformLocation(m_shader->m_program_id, "is_upside_down");
        m_is_left_loc = glGetUniformLocation(m_shader->m_program_id, "is_left");

        m_vr = new Vr{};
        m_cube_mesh = new Mesh{};
        int camera_feed_width = m_vr->m_texture->get_width();
        int camera_feed_height = m_vr->m_texture->get_height();
        m_depth_estimator = new DepthEstimator{camera_feed_width, camera_feed_height,true, 1.6, true, true};
        m_sampler.initialize_sampler();
        m_disparity_sampler.initialize_sampler();
        m_window_renderer = new WindowRenderer{0,m_depth_estimator->get_framebuffer_texture_id()};
        m_window_renderer->add_framebuffer_texture("Image both cameras",m_vr->m_texture->get_texture_id(),true);
        m_window_renderer->add_framebuffer_texture("Luminance Left",m_depth_estimator->get_luminance_camera_left());
        m_window_renderer->add_framebuffer_texture("Luminance Right",m_depth_estimator->get_luminance_camera_right());
        m_window_renderer->add_framebuffer_texture("Disparity Map Left",m_depth_estimator->get_framebuffer_texture_id());
        m_window_renderer->add_framebuffer_texture("HMD output left",m_vr->get_left_framebuffer_texture_id());
        m_window_renderer->add_framebuffer_texture("HMD output right",m_vr->get_right_framebuffer_texture_id());

    }

    void draw(int width, int height)
    {
        ftp_totalFrameTime.start();
        frame_counter++;
        draw_vr();
        ftp_totalFrameTime.stop();
        if(frame_counter%1000==0){
            logger_frametime << ftp_totalFrameTime.to_string(); ftp_totalFrameTime.reset();
            logger_frametime << ftp_gettingCameraFrame.to_string(); ftp_gettingCameraFrame.reset();
            logger_frametime << ftp_depthEstimator.to_string(); ftp_depthEstimator.reset();
            logger_frametime << ftp_renderingLeft.to_string(); ftp_renderingLeft.reset();
            logger_frametime << ftp_renderingRight.to_string(); ftp_renderingRight.reset();
            logger_frametime << ftp_submitFrame.to_string(); ftp_submitFrame.reset();
            logger_frametime << "------------------------------";
        }
        m_window_renderer->draw_window(width,height);
        glFlush();
        glFinish();
    }

    void inline draw_vr()
    {
        
        ftp_gettingCameraFrame.start();
        m_vr->start_frame();
        m_vr->update_texture();
        ftp_gettingCameraFrame.stop();
        
        ftp_depthEstimator.start();
        m_depth_estimator->update_depth_map(m_vr->m_texture->get_texture_id());
        m_vr->update_camera_transform_matrix();
        ftp_depthEstimator.stop();

        m_vr->m_texture->bind();
        glEnable(GL_MULTISAMPLE);
        


        ftp_renderingLeft.start();

        m_vr->bind_left_eye();
        glUseProgram(m_shader->m_program_id);
        glBindVertexArray(m_vao);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniform1f(m_offset_loc, 0.0);
        glUniform1i(m_is_upside_down_loc, GL_FALSE);
        glUniform1i(m_is_left_loc, GL_TRUE);
        glUniformMatrix4fv(m_cam_proj_loc, 1, GL_FALSE, glm::value_ptr(m_vr->m_cam_proj_mat[0]));
        glUniformMatrix4fv(m_eye_unproj_loc, 1, GL_FALSE, glm::value_ptr(m_vr->m_eye_unproj[0]));
        glUniformMatrix4fv(m_eye_to_left_cam_loc, 1, GL_FALSE, glm::value_ptr(m_vr->m_eye_to_cam_left_mat[0]));
        glUniformMatrix4fv(m_eye_to_right_cam_loc, 1, GL_FALSE, glm::value_ptr(m_vr->m_eye_to_cam_right_mat[0]));
        glUniformMatrix4fv(m_hmd_to_cam_loc, 1, GL_FALSE, glm::value_ptr(m_vr->m_hmd_to_cam[0]));
        m_disparity_sampler.bind(1);
        glBindTextureUnit(1,m_depth_estimator->get_framebuffer_texture_id());
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindTextureUnit(1,0);
        m_disparity_sampler.unbind(1);
        glBindVertexArray(0);
        glUseProgram(0);
       //glClear(GL_DEPTH_BUFFER_BIT);
       //m_cube_mesh->draw((m_vr->m_view_to_eye_mat[0])*(glm::inverse(m_vr->m_hmd_mat)),m_window_renderer->m_translation,m_window_renderer->m_scale, m_window_renderer->m_y_rotation_degrees);
        m_cube_mesh->draw((m_vr->m_view_to_eye_mat[0])*(glm::inverse(m_vr->m_hmd_mat)),m_vr->m_cube_mat);
        glDisable(GL_DEPTH_TEST);
        m_vr->blit_frame_left();

        ftp_renderingLeft.stop();

        
        ftp_renderingRight.start();

        m_vr->bind_right_eye();
        glUseProgram(m_shader->m_program_id);
        glBindVertexArray(m_vao);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniform1f(m_offset_loc, 0.5);
        glUniform1i(m_is_upside_down_loc, GL_FALSE);
        glUniform1i(m_is_left_loc, GL_FALSE);
        glUniformMatrix4fv(m_cam_proj_loc, 1, GL_FALSE, glm::value_ptr(m_vr->m_cam_proj_mat[1]));
        glUniformMatrix4fv(m_eye_unproj_loc, 1, GL_FALSE, glm::value_ptr(m_vr->m_eye_unproj[1]));
        glUniformMatrix4fv(m_eye_to_left_cam_loc, 1, GL_FALSE, glm::value_ptr(m_vr->m_eye_to_cam_left_mat[1]));
        glUniformMatrix4fv(m_eye_to_right_cam_loc, 1, GL_FALSE, glm::value_ptr(m_vr->m_eye_to_cam_right_mat[1]));
        glUniformMatrix4fv(m_hmd_to_cam_loc, 1, GL_FALSE, glm::value_ptr(m_vr->m_hmd_to_cam[1]));
        m_disparity_sampler.bind(1);
        glBindTextureUnit(1,m_depth_estimator->get_framebuffer_texture_id());
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindTextureUnit(1,0);
        m_disparity_sampler.unbind(1);
        glBindVertexArray(0);
        glUseProgram(0);
        //glClear(GL_DEPTH_BUFFER_BIT);
        //m_cube_mesh->draw((m_vr->m_view_to_eye_mat[1])*(glm::inverse(m_vr->m_hmd_mat)),m_window_renderer->m_translation, m_window_renderer->m_scale, m_window_renderer->m_y_rotation_degrees);
        m_cube_mesh->draw((m_vr->m_view_to_eye_mat[1])*(glm::inverse(m_vr->m_hmd_mat)),m_vr->m_cube_mat);
        glDisable(GL_DEPTH_TEST);
        m_vr->blit_frame_right();

        
        ftp_renderingRight.stop();

        ftp_submitFrame.start();

        glDisable(GL_MULTISAMPLE);

        m_vr->m_texture->unbind();
        m_vr->submit_frames();
        glFinish();
        ftp_submitFrame.stop();
    }

    ~MainApplication()
    {
        delete m_vr;
        delete m_shader;
        delete m_depth_estimator;
    }
};
} // namespace arDepthEstimation