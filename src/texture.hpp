#pragma once

#include <cstring>
#include <glad/gl.h>
#include <stdexcept>
#include <vector>

#include "sampler.hpp"

namespace arDepthEstimation
{

template <typename Pixel> class Texture
{
    GLuint m_texture_id;
    const size_t m_width;
    const size_t m_height;
    const GLenum m_pixel_format;
    const GLenum m_data_type;
    ISampler *m_sampler;
    GLint m_sampler_position;

  public:
    void bind()
    {
        m_sampler->bind(m_sampler_position);
        glBindTexture(GL_TEXTURE_2D, m_texture_id);
    }

    void unbind()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        m_sampler->unbind(m_sampler_position);
    }

    void upload_texture(Pixel *data)
    {
        bind();
        glTextureSubImage2D(m_texture_id,
                            0,                    // Mip-Map level
                            0, 0,                 // x and y offset
                            m_width, m_height,    // width and height_ of the texture
                            GL_RGBA, m_data_type, // pixel format and data type of sub-pixel
                            data                  // the pixel data
        );
        unbind();
    }

    /**
     * @brief Construct a new Texture object
     *
     * @param width_ Width of the image
     * @param height_ Height of the image
     * @param data Pointer to the pixeldata in subsequent packed memory
     * @param pixel_format_ gl pixel format of texture
     * @param data_type_ data_type_ of the sub-pixel
     * @cond data must point to width_*height_*sizeof(pixel) readable memory space
     */
    Texture(size_t width_, size_t height_, GLenum pixel_format_, GLenum data_type_, Pixel *data, ISampler *sampler_,
            GLint sampler_position_)
        : m_width{width_}, m_height{height_}, m_pixel_format{pixel_format_},
          m_data_type{data_type_}, m_sampler{sampler_}, m_sampler_position{sampler_position_}
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &m_texture_id);
        glTextureStorage2D(m_texture_id, 1, pixel_format_, m_width, m_height);
        upload_texture(data);
    }

    ~Texture()
    {
        glDeleteTextures(1, &m_texture_id);
    }
};

} // namespace arDepthEstimation