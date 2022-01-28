#pragma once
#include "glad/gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>

#include "shader.hpp"
#include "log.hpp"

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam)
{
    logger_warn << "GL CALLBACK:" << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") 
                << "type = 0x" << type 
                << ", severity = 0x" << severity 
                << ", message = " << message;
}

class ContextManager
{
  public:
    int f = 5;

    struct Vertex
    {
        float x, y;
        float r, g, b;
    };
    Vertex vertices[3] = {{-0.6f, -0.4f, 1.f, 0.f, 0.f}, {0.6f, -0.4f, 0.f, 1.f, 0.f}, {0.f, 0.6f, 0.f, 0.f, 1.f}};

    static void error_callback(int error, const char *description)
    {
        logger_error << description;
    }

    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    ContextManager()
    {
        GLFWwindow *window;
        GLuint vertex_buffer, vertex_shader, fragment_shader, program, vertex_array_object;
        GLint mvp_location, vpos_location, vcol_location;

        glfwSetErrorCallback(error_callback);

        if (!glfwInit())
            exit(EXIT_FAILURE);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
        if (!window)
        {
            logger_error << "Failed to create Window or OpenGL Context!";
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwSetKeyCallback(window, key_callback);

        glfwMakeContextCurrent(window);
        gladLoadGL(glfwGetProcAddress);
        logger_info << "GLFW Version: " << glfwGetVersionString();
        logger_info << "OpenGL Version: " << glGetString(GL_VERSION);
        glfwSwapInterval(1);

        #ifdef DEBUG_GL
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
        #endif

        // NOTE: OpenGL error checks have been omitted for brevity

        glGenVertexArrays(1, &vertex_array_object);
        glBindVertexArray(vertex_array_object);
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        /*
           vertex_shader = glCreateShader(GL_VERTEX_SHADER);
           glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
           glCompileShader(vertex_shader);

           fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
           glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
           glCompileShader(fragment_shader);

           program = glCreateProgram();
           glAttachShader(program, vertex_shader);
           glAttachShader(program, fragment_shader);
           glLinkProgram(program);
          */
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

        while (!glfwWindowShouldClose(window))
        {
            float ratio;
            int width, height;
            mat4x4 m, p, mvp;

            glfwGetFramebufferSize(window, &width, &height);
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

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwDestroyWindow(window);

        glfwTerminate();
        exit(EXIT_SUCCESS);
    }
};
