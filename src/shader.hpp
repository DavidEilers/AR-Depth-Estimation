#pragma once

//std header
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

//library header
#include <glad/gl.h>

//my header
#include "log.hpp"

class Shader{
    public:
    std::string m_vertex_shader;
    std::string m_fragment_shader;
    GLuint m_vertex_shader_id;
    GLuint m_fragment_shader_id;
    GLuint m_program_id;

    Shader(std::string vertex_shader_path, std::string fragment_shader_path):
    m_vertex_shader{vertex_shader_path}, m_fragment_shader{fragment_shader_path}
    {
        logger_info << "Trying to load shader:" <<  vertex_shader_path << fragment_shader_path;
        std::ifstream vertex_shader_is (vertex_shader_path,std::ifstream::in);
        if(vertex_shader_is.is_open() == false){
            throw std::runtime_error("couldn't open vertex shader!");
        }
        
        std::stringstream vertex_shader_code{};
        vertex_shader_code << vertex_shader_is.rdbuf();

        std::ifstream fragment_shader_is(fragment_shader_path,std::ifstream::in);
        if(vertex_shader_is.is_open() == false){
            throw std::runtime_error("couldn't open fragment shader!");
        }
        std::stringstream fragment_shader_code{};
        fragment_shader_code << fragment_shader_is.rdbuf();
        createShader(vertex_shader_code.str(),fragment_shader_code.str());
        
    }

    void createShader(std::string vertex_shader_code, std::string fragment_shader_code){

        m_vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        m_fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

        //If we got no shader_id throw exception
        if(m_vertex_shader_id == -1 || m_fragment_shader_id == -1)
            std::runtime_error("didn't get shader_id from OpenGL");

        const char * vertex_shader_p =  vertex_shader_code.c_str();
        const char * fragment_shader_p =  fragment_shader_code.c_str();
        glShaderSource(m_vertex_shader_id,1, &vertex_shader_p,NULL);
        glShaderSource(m_fragment_shader_id,1, &fragment_shader_p, NULL);

        glCompileShader(m_vertex_shader_id);
        glCompileShader(m_fragment_shader_id);

        GLint compile_status;
        glGetShaderiv(m_vertex_shader_id, GL_COMPILE_STATUS, &compile_status);
        if(compile_status == GL_FALSE){
            logger_warn << "Couldn't create vertex shader!";
            std::vector<char> error_log;
            error_log.resize(1024);
            GLint log_length;
            glGetShaderInfoLog(m_vertex_shader_id,error_log.size(),&log_length,error_log.data());
            error_log.resize(log_length);
            error_log.push_back('\0');
            logger_warn << error_log.data();
            throw std::runtime_error("couldn't create vertex shader!");
        }

        glGetShaderiv(m_fragment_shader_id, GL_COMPILE_STATUS, &compile_status);
        if(compile_status == GL_FALSE){
            logger_warn << "Couldn't create fragment shader!";
            std::vector<char> error_log;
            error_log.resize(1024);
            GLint log_length;
            glGetShaderInfoLog(m_fragment_shader_id,error_log.size(),&log_length,error_log.data());
            error_log.resize(log_length);
            error_log.push_back('\0');
            logger_warn << error_log.data();
            throw std::runtime_error("couldn't create fragment shader!");
        }

        m_program_id = glCreateProgram();
        glAttachShader(m_program_id, m_vertex_shader_id);
        glAttachShader(m_program_id, m_fragment_shader_id);
        glLinkProgram(m_program_id);

        GLint link_status;
        glGetProgramiv(m_program_id,GL_LINK_STATUS,&link_status);
        if(link_status == GL_FALSE){
            logger_warn << "Couldn't link shader program!";
            std::vector<char> error_log;
            error_log.resize(1024);
            GLint log_length;
            glGetProgramInfoLog(m_program_id, error_log.size(),&log_length,error_log.data());
            error_log.resize(log_length);
            error_log.push_back('\0');
            logger_warn << error_log.data();
            throw std::runtime_error("couldn't link shader program!");
        }



    }
};