#pragma once

#include "pi_tracker/fd.h"

#include <cstddef>
#include <cstdint>
#include <span>

namespace pi_tracker
{

    // Послідовний порт у «сирому» режимі (8N1, без керування потоком)
    class SerialPort
    {
    public:
        // Кидає std::system_error, якщо порт не відкривається; std::invalid_argument — якщо швидкість не підтримується
        SerialPort(const char *path, int baudRate);

        // Читає до buffer.size() байтів; чекає не довше 0.1 с; повертає кількість прочитаних (може бути 0)
        std::size_t read(std::span<std::uint8_t> buffer) const;
        void write(std::span<const std::uint8_t> data) const;

    private:
        Fd m_fd;
    };

} // namespace pi_tracker