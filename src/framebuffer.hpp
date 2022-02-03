#pragma once

#include "texture.hpp"
#include "log.hpp"

namespace arDepthEstimation{

class Framebuffer{

    GLuint m_framebuffer_id;
    GLuint m_renderbuffer_id;
    GLuint m_framebuffer_texture_id;
    size_t m_width;
    size_t m_height;

    public:
    Framebuffer(size_t width_, size_t height_): m_width{width_}, m_height{height_}{
        glGenFramebuffers(1,&m_framebuffer_id);
        logger_info << "Generated framebuffer";
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
        logger_info << "Bound framebuffer";

        glGenRenderbuffers(1, &m_renderbuffer_id);
        logger_info << "generated render framebuffer";
        glBindFramebuffer(GL_RENDERBUFFER, m_renderbuffer_id);
        logger_info << "bound render framebuffer";
	    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, m_width, m_height);

	    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,	m_renderbuffer_id );

        glGenTextures(1,&m_framebuffer_texture_id);
        glBindTexture(GL_TEXTURE_2D, m_framebuffer_texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_framebuffer_id,0);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE){
            throw std::runtime_error("framebuffer not complete!");
        }

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

    ~Framebuffer(){
        glDeleteTextures(1, &m_framebuffer_texture_id);
        glDeleteRenderbuffers(1, &m_renderbuffer_id);
        glDeleteFramebuffers(1, &m_framebuffer_id);
    }
};

}