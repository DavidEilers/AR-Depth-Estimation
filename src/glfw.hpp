#pragma once
#include "glad/gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>
#include <memory>

#include "shader.hpp"
#include "log.hpp"

namespace arDepthEstimation{

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam)
{
    logger_warn << "GL CALLBACK:" << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") 
                << "type = 0x" << type 
                << ", severity = 0x" << severity 
                << ", message = " << message;
}

struct Application{
    virtual void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)=0;
    virtual void setup()=0;
    virtual void draw(int width, int height)=0;
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
        //m_app->key_callback( window, key, scancode, action, mods);
    }

    GLFWwindow *window;
    int width, height;

    bool should_window_close(){
        return glfwWindowShouldClose(window);
    }

    void update_window(){
        glfwGetFramebufferSize(window, &width, &height);
        glfwPollEvents();
    }

    void swap_buffers(){
        glfwSwapBuffers(window);
    }

    ContextManager()
    {
        glfwSetErrorCallback(error_callback);

        if (!glfwInit())
            exit(EXIT_FAILURE);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        #define DEBUG_GL
        #ifdef DEBUG_GL
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
        #endif

        window = glfwCreateWindow(800, 600, "Simple example", NULL, NULL);
        if (!window)
        {
            logger_error << "Failed to create Window or OpenGL Context!";
            glfwTerminate();
            return;
        }

        //glfwSetKeyCallback(window, key_callback);

        glfwMakeContextCurrent(window);
        gladLoadGL(glfwGetProcAddress);
        logger_info << "GLFW Version: " << glfwGetVersionString();
        logger_info << "OpenGL Version: " << glGetString(GL_VERSION);
        glfwSwapInterval(1);

    }

    ~ContextManager(){
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

    std::unique_ptr<ContextManager> context = nullptr;

    void run_app(Application * app){
        context = std::make_unique<ContextManager>();

        app->setup();
        double time_frame_start;
        double time_frame_finish;
        double frame_time = 0;
        double prev_print_time = glfwGetTime();
        double frame_time_avg = 0;
        constexpr double timer_print_threshold = 5.0; //Every 5 seconds
        while(context->should_window_close() == false){
            time_frame_start = glfwGetTime();

            context->update_window();
            app->draw(context->width, context->height);
            glFlush();glFinish();

            time_frame_finish = glfwGetTime();
            frame_time = time_frame_finish-time_frame_start;
            frame_time_avg = (frame_time_avg+frame_time)/2.0;            
            if(time_frame_finish - prev_print_time > timer_print_threshold){
                logger_info << "frametime:" << frame_time_avg;
                prev_print_time = time_frame_finish;
                frame_time_avg = frame_time; 
            }
            
            context->swap_buffers();
        }
        context.reset();
    }

}