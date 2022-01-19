#pragma once

#include <cstring>
#include <stdexcept>
#include <vector>

namespace arDepthEstimation
{

template <typename Pixel> class Image
{
    std::vector<Pixel> m_image_data;
    const size_t width;
    const size_t height;

  public:
    /**
     * @brief Construct a new Image object
     *
     * @param width Width of the image
     * @param height Height of the image
     * @param data Pointer to the pixeldata in subsequent packed memory
     * @cond data must point to width*height*sizeof(pixel) readable memory space
     */
    Image(size_t width, size_t height, Pixel *data) : width{width}, height{height}
    {
        m_image_data.resize(width * height);
        std::memcpy(m_image_data.data(), data, sizeof(Pixel) * width * height);
        std::cout << "Image created" << std::endl;
    }

    ~Image()
    {
        // delete m_image_data;
        std::cout << "Image deleted!" << std::endl;
    }

    /**
     * @brief Return Pixel at Position (x,y)
     *
     * @param x x position
     * @param y y position
     * @return Pixel at position (x,y) starting from (0,0)
     */
    Pixel at(size_t x, size_t y)
    {
        if (x > width)
        {
            throw std::out_of_range("X is out of range!");
        }
        if (y > height)
        {
            throw std::out_of_range("Y is out of range!");
        }
        return m_image_data[y * width + x];
    }
};

} // namespace arDepthEstimation