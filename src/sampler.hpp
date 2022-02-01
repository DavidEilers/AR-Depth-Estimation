#pragma once

#include <cstring>
#include <stdexcept>
#include <vector>
//#include "glad/gl.h"

namespace arDepthEstimation
{

class ISampler
{
  public:
    virtual void bind(GLint position) = 0;
    virtual void unbind(GLint position) = 0;
};

class LinearSampler : public ISampler
{

    GLuint m_sampler_id;

  public:
    /**
     * @brief Does nothing
     */
    LinearSampler() : m_sampler_id{0}
    {
    }

    /**
     * @brief initializes sampler
     *
     * @cond needs an existing binded OpenGL Context
     */
    void initialize_sampler()
    {
        glGenSamplers(1, &m_sampler_id);
        glSamplerParameteri(m_sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(m_sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(m_sampler_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(m_sampler_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(m_sampler_id, GL_TEXTURE_MAX_LEVEL, 0);
    }

    void bind(GLint position) override
    {
        glBindSampler(position, m_sampler_id);
    }

    void unbind(GLint position) override
    {
        glBindSampler(position, 0);
    }

    ~LinearSampler()
    {
        glDeleteSamplers(1, &m_sampler_id);
    }
};

} // namespace arDepthEstimation