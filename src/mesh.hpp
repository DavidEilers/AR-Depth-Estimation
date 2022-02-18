#pragma once

#include <cmath>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "shader.hpp"

namespace arDepthEstimation{

    struct __attribute__((packed)) MeshVertex{
        float x;
        float y;
        float z;
    };

    const MeshVertex cube_vertices[]={           // indices:            0           1           2           3
        {-1,-1,-1},{1,-1,-1},{1,-1,1},{-1,-1,1}, //lower vertices: {near left},{near right},{far right}, {far left}
                                                 // indices:            4           5            6            7
        {-1,1,-1},{1,1,-1},{1,1,1},{-1,1,1}      // upper vertices: {near left},{near right},{far right}, {far left}
        };
    
    const GLushort cube_indices[]{
        0,1,4, // camera side
        4,1,5, // camera side
        0,4,3, // left side
        7,3,4, // left side
        7,6,2, // away the camera side
        2,3,7, // away the camera side
        5,2,6, // right side
        5,1,2, // right side
        4,5,7, // upper side
        5,6,7, // upper side
        0,3,1, // downfacing side
        1,3,2 // donwnfacing side
    };

class Mesh{

    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    GLuint mvp_loc;
    Shader* myShader=nullptr;
    double time_start;

    public:

    Mesh(){
        glGenVertexArrays(1,&vao);
        glGenBuffers(1,&vbo);
        glGenBuffers(1,&ibo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ARRAY_BUFFER,sizeof(cube_vertices),cube_vertices,GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(cube_indices),cube_indices,GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void *)0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

        std::string shader_dir("assets\\shader\\");
        std::string vertex_shader_path(shader_dir + "mesh.vert.glsl");
        std::string fragment_shader_path(shader_dir + "mesh.frag.glsl");

        myShader =  new Shader{vertex_shader_path, fragment_shader_path};

        mvp_loc = glGetUniformLocation(myShader->m_program_id,"mvp");
        time_start = glfwGetTime();
        
        }

    void draw(){
        glm::mat4 mvp_matrix{1.0f};
        mvp_matrix = glm::scale(mvp_matrix,glm::vec3{0.1});
        float degrees = std::fmod(((glfwGetTime()-time_start)*10),360.0);
        mvp_matrix = glm::rotate(mvp_matrix,glm::radians(degrees),glm::vec3{0.0f,1.0f,0.0f});
        //mvp_matrix = glm::translate(mvp_matrix,glm::vec3{0.0,0.0,0.4});
        mvp_matrix[3][2]=0.4;
        //glDisable(GL_CULL_FACE);
        glUseProgram(myShader->m_program_id);
        glUniformMatrix4fv(mvp_loc,1,GL_FALSE,glm::value_ptr(mvp_matrix));
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_SHORT,(void *)(0));
        //glEnable(GL_CULL_FACE);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    ~Mesh(){
        glDeleteVertexArrays(1,&vao);
        glDeleteBuffers(1,&vbo);
        glDeleteBuffers(1,&vao);
        delete myShader;
    }
};

}