#pragma once
#include "glad/gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "linmath.h"

#include <memory>
#include <stdlib.h>

#include "log.hpp"
#include "shader.hpp"
#include "frametime_probe.hpp"
#include "asset_path.hpp"

namespace arDepthEstimation
{

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam)
{
    logger_warn << "GL CALLBACK:" << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") << "type = 0x" << type
                << ", severity = 0x" << severity << ", message = " << message;
}

struct Application
{
    virtual void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) = 0;
    virtual void setup() = 0;
    virtual void draw(int width, int height) = 0;
};

class ContextManager
{
  public:
    static void error_callback(int error, const char *description)
    {
        logger_error << description;
    }

    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        // m_app->key_callback( window, key, scancode, action, mods);
    }

    void close_window(){
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }

    GLFWwindow *m_window;
    int m_width, m_height;
    FrameTimeProbe m_ftp_glfwFrametime{"GLFW frametime"};

    bool should_window_close()
    {
        return glfwWindowShouldClose(m_window);
    }

    void update_window()
    {
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        glfwPollEvents();
    }

    void swap_buffers()
    {
        glfwSwapBuffers(m_window);
    }

    ContextManager()
    {
        glfwSetErrorCallback(error_callback);

        if (!glfwInit())
            exit(EXIT_FAILURE);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

//#define DEBUG_GL
#ifdef DEBUG_GL
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

        m_window = glfwCreateWindow(800, 600, "Simple example", NULL, NULL);
        if (!m_window)
        {
            logger_error << "Failed to create Window or OpenGL Context!";
            glfwTerminate();
            return;
        }

        // glfwSetKeyCallback(window, key_callback);

        glfwMakeContextCurrent(m_window);
        gladLoadGL(glfwGetProcAddress);
        logger_info << "GLFW Version: " << glfwGetVersionString();
        logger_info << "OpenGL Version: " << glGetString(GL_VERSION);
        glfwSwapInterval(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 450");

    }

    ~ContextManager()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
};

std::unique_ptr<ContextManager> g_context = nullptr;
AssetPath g_asset_path{};

void run_app(Application *app)
{
    g_context = std::make_unique<ContextManager>();

    app->setup();
    
    constexpr double timer_print_threshold = 5.0; // Every 5 seconds
    while (g_context->should_window_close() == false)
    {
        g_context->m_ftp_glfwFrametime.start();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        g_context->update_window();
        app->draw(g_context->m_width, g_context->m_height);
        glFlush();
        glFinish();

        g_context->m_ftp_glfwFrametime.stop();
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        ImGui::Render();
        glViewport(0, 0, g_context->m_width, g_context->m_height);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        g_context->swap_buffers();
    }
    g_context.reset();
}

} // namespace arDepthEstimation