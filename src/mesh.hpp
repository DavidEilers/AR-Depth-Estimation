#pragma once

#include <cmath>

#include <glad/gl.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "shader.hpp"

namespace arDepthEstimation
{

struct __attribute__((packed)) MeshVertex
{
    float x;
    float y;
    float z;
    float texture_u;
    float texture_v;
    float normal_x;
    float normal_y;
    float normal_z;
};

const MeshVertex cube_vertices[] = {
    {-1, -1, -1,0,0,0,0,0}, // indices 0 {lower near left}
    {1, -1, -1,0,0,0,0,0},  // indices 1 {lower near right}
    {1, -1, 1,0,0,0,0,0},   // indices 2 {lower far right}
    {-1, -1, 1,0,0,0,0,0},  // indices 3 {lower far left}
    {-1, 1, -1,0,0,0,0,0},  // indices 4 {upper near left}
    {1, 1, -1,0,0,0,0,0},   // indices 5 {upper near right}
    {1, 1, 1,0,0,0,0,0},    // indices 6 {upper far right}
    {-1, 1, 1,0,0,0,0,0}    // indices 7 {upper far left}
};

const GLushort cube_indices[]{
    0, 1, 4, // camera side
    4, 1, 5, // camera side
    0, 4, 3, // left side
    7, 3, 4, // left side
    2, 6, 7, // away the camera side
    2, 3, 7, // away the camera side
    5, 2, 6, // right side
    5, 1, 2, // right side
    4, 5, 7, // upper side
    5, 6, 7, // upper side
    0, 3, 1, // downfacing side
    1, 3, 2  // donwnfacing side
};

class Mesh
{

    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_mvp_loc;
    Shader *m_shader = nullptr;
    double m_time_start;
    size_t indices_size;

  public:
    Mesh()
    {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ibo);
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

        
        std::string obj_dir("assets\\obj\\");
        std::string cube_obj_path(obj_dir + "utah_teapot.obj");

        std::vector<MeshVertex> vertices;
        std::vector<GLushort> indices;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string error;
        logger_info << "Before loading an Object";
        if(tinyobj::LoadObj(&attrib,&shapes,&materials,&warn,&error,cube_obj_path.c_str())==false){
            throw std::runtime_error("Could not load obj file: "+warn+error);
        }

        logger_info << "Loaded object from file";

        for(size_t i = 0; i < attrib.vertices.size()/3; i++){
            MeshVertex tmp{
                attrib.vertices[i*3+0], //attrib vertice layout x, y , z, x, y, z...
                attrib.vertices[i*3+1], 
                attrib.vertices[i*3+2],
                0,0,
                0,0,0
            };
            vertices.push_back(tmp);
        }
        for(const tinyobj::shape_t & shape : shapes){
            for(const tinyobj::index_t& index : shape.mesh.indices){
                indices.push_back((GLushort)index.vertex_index);
            }
        }
        indices_size = indices.size();
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0])*vertices.size(), vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0])*indices.size(), indices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void *)offsetof(MeshVertex, x));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void *)offsetof(MeshVertex, texture_u));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void *)offsetof(MeshVertex, normal_x));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        std::string shader_dir("assets\\shader\\");
        std::string vertex_shader_path(shader_dir + "mesh.vert.glsl");
        std::string fragment_shader_path(shader_dir + "mesh.frag.glsl");

        m_shader = new Shader{vertex_shader_path, fragment_shader_path};

        m_mvp_loc = glGetUniformLocation(m_shader->m_program_id, "mvp");
        m_time_start = glfwGetTime();
    }

    void draw(glm::mat4 vp_matrix, float* translation, float scale, float rot )
    {
        glm::mat4 model_matrix = glm::mat4{1.0f};
        float degrees = rot;//std::fmod(((glfwGetTime() - m_time_start) * 10), 360.0);
        model_matrix = glm::translate(model_matrix,glm::vec3{translation[0],translation[1],translation[2]});
        model_matrix = glm::rotate(model_matrix, glm::radians(degrees), glm::vec3{0.0f, 1.0f, 0.0f});
        model_matrix = glm::scale(model_matrix, glm::vec3{scale});
        glm::mat4 mvp_matrix = vp_matrix*model_matrix;
        
        glUseProgram(m_shader->m_program_id);
        glUniformMatrix4fv(m_mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp_matrix));
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void *)(0));
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void draw(glm::mat4 vp_matrix, glm::mat4 model_matrix )
    {
        //model_matrix[0][0] *= 0.1;
        //model_matrix[1][1] *= 0.1;
        //model_matrix[2][2] *= 0.1;
        //glm::vec4 buff = model_matrix[0];
        //model_matrix[0]= model_matrix[1];
        //model_matrix[1]= buff;
        glm::mat4 scale_mat{1.0f};
        scale_mat = glm::scale(scale_mat,glm::vec3{0.1});
        glm::mat4 mvp_matrix = vp_matrix*model_matrix*scale_mat;
        
        glUseProgram(m_shader->m_program_id);
        glUniformMatrix4fv(m_mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp_matrix));
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_SHORT, (void *)(0));
        glBindVertexArray(0);
        glUseProgram(0);
    }

    ~Mesh()
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_vao);
        delete m_shader;
    }
};

} // namespace arDepthEstimation