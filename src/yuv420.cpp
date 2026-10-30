#include "pi_tracker/yuv420.h"

#include <stdexcept>
#include <iostream>
#include <span>
#include <cstdint>

namespace pi_tracker
{

    Yuv420View::Yuv420View(
        std::span<const std::uint8_t> data,
        std::size_t width,
        std::size_t height) : m_data{data}, m_width{width}, m_height{height}
    {
        if (width % 2 != 0)
        {
            throw std::invalid_argument("width % 2 != 0");
        }

        if (height % 2 != 0)
        {
            throw std::invalid_argument("width % 2 != 0");
        }

        if (data.size() == 0)
        {
            throw std::invalid_argument("data.size() == 0");
        }

        if (data.size() < frameSize(width, height))
        {
            throw std::invalid_argument("data.size() >= frameSize(width, height)");
        }
    };

    std::size_t Yuv420View::frameSize(std::size_t width, std::size_t height)
    {
        return width * height * 3 / 2;
    };

    std::span<const std::uint8_t> Yuv420View::y() const
    {
        return m_data.subspan(0, m_width * m_height);
    };

    std::span<const std::uint8_t> Yuv420View::u() const
    {
        return m_data.subspan(m_width * m_height, m_width * m_height / 4);
    };

    std::span<const std::uint8_t> Yuv420View::v() const
    {
        const std::size_t ySize = m_width * m_height;
        return m_data.subspan(ySize + ySize / 4, ySize / 4);
    };

    std::uint8_t Yuv420View::luma(std::size_t column, std::size_t row) const
    {
        return m_data[row * m_width + column];
    };

}