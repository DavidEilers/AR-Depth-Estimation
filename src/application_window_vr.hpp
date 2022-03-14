#pragma once
#include "glfw.hpp"

#include <cstddef>
#include <map>

#include "depth_estimation.hpp"
#include "init_vr.hpp"
#include "mesh.hpp"
#include "sampler.hpp"
#include "texture.hpp"

namespace arDepthEstimation
{

class WindowRenderer
{
    struct TextureInfo{
        std::string m_name;
        GLuint m_texture_id;
        bool m_is_upside_down;


        TextureInfo(std::string name,GLuint texture_id, bool is_upside_down):
        m_name{name},
        m_texture_id{texture_id},
        m_is_upside_down{is_upside_down}
        {}
        
        TextureInfo():TextureInfo("",0,false)
        {}

        ~TextureInfo(){}
    };

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
    GLuint m_framebuffer_id;
    TextureInfo m_active_texture{};
    GLuint m_offset_loc;
    GLuint m_transform_loc;
    GLuint m_is_upside_down_loc;
    glm::mat4 m_identity_mat{1.0f};
    DepthEstimator *m_depth_estimator;
    LinearSampler m_sampler{};
    std::vector<TextureInfo> m_framebuffer_textures;



  public:
    float m_translation[3] = {-0.1,0.0,-0.5};
    float m_scale = 0.2;
    float m_y_rotation_degrees = 45.0;

    void add_framebuffer_texture(std::string name, GLuint texture_id, bool is_upside_down=false){
        m_framebuffer_textures.emplace_back(name,texture_id,is_upside_down);
    }
    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        logger_info << "Key was pressed!";
    }

    WindowRenderer(GLuint framebuffer_id, GLuint active_texture_id) : m_framebuffer_id{framebuffer_id}
    {
        m_active_texture.m_texture_id=active_texture_id;
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
        std::string vertex_shader_path(shader_dir + "screen_quad_simple.vert.glsl");
        std::string fragment_shader_path(shader_dir + "screen_quad_simple.frag.glsl");

        m_shader = new Shader{vertex_shader_path, fragment_shader_path};

        //m_offset_loc = glGetUniformLocation(m_shader->m_program_id, "offset");
        //m_transform_loc = glGetUniformLocation(m_shader->m_program_id, "transform");
        m_is_upside_down_loc = glGetUniformLocation(m_shader->m_program_id, "is_upside_down");
        m_sampler.initialize_sampler();
    }

    void inline draw_imgui(){
        ImGui::Begin("Options");
        ImGui::BeginListBox("framebuffer");
        for(auto &e : m_framebuffer_textures){
            const bool selected = (e.m_texture_id==m_active_texture.m_texture_id);
            if(ImGui::Selectable(e.m_name.c_str(),&selected)){
                m_active_texture = e;
            }
            if(selected){
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();

        ImGui::SliderFloat3("position", m_translation, -10.0, 0.0);
        ImGui::SliderFloat("scale", &m_scale, 0.0, 2.0);
        ImGui::SliderFloat("y rotation in degrees", &m_y_rotation_degrees, 0.0, 360.0);
        ImGui::End();
    }

    void inline draw_window(int width, int height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER,m_framebuffer_id);
        glUseProgram(m_shader->m_program_id);
        glBindVertexArray(m_vao);
        m_sampler.bind(0);
        glBindTextureUnit(0, m_active_texture.m_texture_id);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniform1i(m_is_upside_down_loc, m_active_texture.m_is_upside_down? GL_TRUE: GL_FALSE );
        glViewport(0, 0, width, height);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindTextureUnit(0, 0);
        m_sampler.unbind(0);
        glBindVertexArray(0);
        glUseProgram(0);
        draw_imgui();
    }

    ~WindowRenderer()
    {
        delete m_shader;
    }
};
} // namespace arDepthEstimation