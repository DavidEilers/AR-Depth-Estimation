#pragma once

#include "texture.hpp"
#include "log.hpp"

namespace arDepthEstimation{

class Framebuffer{

    GLuint m_framebuffer_id;
    GLuint m_renderbuffer_id;
    GLuint m_framebuffer_texture_id;
    GLuint m_framebuffer_resolve_id;
    GLuint m_framebuffer_resolve_texture_id;
    size_t m_width;
    size_t m_height;
    LinearSampler m_sampler{};

    public:
    Framebuffer(size_t width_, size_t height_): m_width{width_}, m_height{height_}{

	    m_sampler.bind(0);
        glGenFramebuffers(1,&m_framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);

        glGenRenderbuffers(1, &m_renderbuffer_id);
        glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer_id);
	    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT32F, m_width, m_height);

	    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,	m_renderbuffer_id );

        glGenTextures(1,&m_framebuffer_texture_id);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_framebuffer_texture_id);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, m_width, m_height, true);// 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_framebuffer_texture_id,0);
        
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE){
            std::string error_str{"framebuffer with renderbuffer not complete!ERROR: "};
                        
            switch(status){
                case GL_FRAMEBUFFER_UNDEFINED: error_str+="UNDEFINED";break;
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: error_str+="INCOMPLETE_ATTACHMENT";break; 
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: error_str+="INCOMPLETE_MISSING_ATTACHMENT";break; 
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: error_str+="INCOMPLETE_DRAW_BUFFER";break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: error_str+="INCOMPLETE_READ_BUFFER";break;
                case GL_FRAMEBUFFER_UNSUPPORTED: error_str+="UNSUPPORTED";break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: error_str+="INCOMPLETE_MULTISAMPLE";break;
                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: error_str+="INCOMPLETE_LAYER_TARGETS";break;
                default: error_str+="unknown error_str";break;
            }

            throw std::runtime_error(error_str.c_str());

        }

        //Resolve framebuffer

        

        glGenFramebuffers(1, &m_framebuffer_resolve_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_resolve_id);

        glGenTextures(1, &m_framebuffer_resolve_texture_id);
        glBindTexture(GL_TEXTURE_2D, m_framebuffer_resolve_texture_id);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_framebuffer_resolve_texture_id, 0);


        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE){
            throw std::runtime_error("framebuffer not complete!");
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_sampler.unbind(0);

    }

    void bind(){
        glBindFramebuffer(GL_FRAMEBUFFER,m_framebuffer_id);
        glViewport(0, 0, m_width, m_height);
    }

    void unbind(){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLuint get_framebuffer_id(){
        return m_framebuffer_id;
    }

    GLuint get_framebuffer_resolve_id(){
        return m_framebuffer_resolve_id;
    }

    GLuint get_framebuffer_resolve_texture_id(){
        return m_framebuffer_resolve_texture_id;
    }

    size_t get_width(){
        return m_width;
    }

    size_t get_height(){
        return m_height;
    }

    ~Framebuffer(){
        glDeleteTextures(1, &m_framebuffer_resolve_texture_id);
        glDeleteFramebuffers(1, &m_framebuffer_resolve_id);
        glDeleteTextures(1, &m_framebuffer_texture_id);
        glDeleteRenderbuffers(1, &m_renderbuffer_id);
        glDeleteFramebuffers(1, &m_framebuffer_id);
    }
};

}