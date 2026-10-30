#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace pi_tracker
{

    // Перегляд одного кадру YUV420 planar (I420), НЕ володіє пам'яттю:
    // спершу площина Y (width × height), потім U і V, кожна (width/2) × (height/2)
    class Yuv420View
    {
    public:
        // Кидає std::invalid_argument, якщо розміри непарні або даних менше, ніж frameSize()
        Yuv420View(std::span<const std::uint8_t> data, std::size_t width, std::size_t height);

        // Скільки байтів займає кадр: 1.5 байта на піксель
        static std::size_t frameSize(std::size_t width, std::size_t height);

        std::span<const std::uint8_t> y() const;
        std::span<const std::uint8_t> u() const;
        std::span<const std::uint8_t> v() const;

        // Яскравість пікселя в рядку row, стовпці column
        std::uint8_t luma(std::size_t column, std::size_t row) const;

        std::size_t width() const { return m_width; }
        std::size_t height() const { return m_height; }

    private:
        std::span<const std::uint8_t> m_data;
        std::size_t m_width;
        std::size_t m_height;
    };

} // namespace pi_tracker