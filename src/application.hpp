#pragma once
#include "glfw.hpp"
extern "C"{
#include "stb_image.h"
}

class MainApplication : public Application{

    struct Vertex
    {
        float x, y;
        float r, g, b;
    };
    Vertex vertices[3] = {{-0.6f, -0.4f, 1.f, 0.f, 0.f}, {0.6f, -0.4f, 0.f, 1.f, 0.f}, {0.f, 0.6f, 0.f, 0.f, 1.f}};

    GLint mvp_location, vpos_location, vcol_location;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program, vertex_array_object;

    public:

    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
        logger_info << "Key was pressed!";
    }

    void setup(){
        
        #ifdef DEBUG_GL
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
        #endif

        glGenVertexArrays(1, &vertex_array_object);
        glBindVertexArray(vertex_array_object);
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        std::string shader_dir("assets\\shader\\");
        std::string vertex_shader_path(shader_dir + "example.vert.glsl");
        std::string fragment_shader_path(shader_dir + "example.frag.glsl");
        Shader myShader{vertex_shader_path, fragment_shader_path};
        vertex_shader = myShader.m_vertex_shader_id;
        fragment_shader = myShader.m_fragment_shader_id;
        program = myShader.m_program_id;

        mvp_location = glGetUniformLocation(program, "MVP");
        vpos_location = glGetAttribLocation(program, "vPos");
        vcol_location = glGetAttribLocation(program, "vCol");

        if (mvp_location == -1 || vpos_location == -1 || vcol_location == -1)
        {
            logger_warn << "Some uniform location was not found!";
        }
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void *)(sizeof(float) * 2));

        glClearColor(0.5f, 0.5f, 0.5, 1.0);
    }

    void draw(int width, int height){
        float ratio;
        mat4x4 m, p, mvp;
        ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float)glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat *)mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
};