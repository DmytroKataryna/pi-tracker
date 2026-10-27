#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace pi_tracker
{

    struct TfLunaFrame
    {
        std::uint16_t distanceCm{};
        std::uint16_t amplitude{};
        double temperatureC{};
    };

    // Збирає кадри TF-Luna з потоку байтів UART
    class TfLunaParser
    {
    public:
        // Подати наступний байт. Повертає кадр, коли зібрано і перевірено всі 9 байтів
        std::optional<TfLunaFrame> feed(std::uint8_t byte);
        std::size_t checksumErrors() const { return m_checksumErrors; }

    private:
        std::array<std::uint8_t, 9> m_frame{}; // байти поточного кадру
        std::size_t m_index{};                 // скільки байтів кадру вже зібрано
        std::size_t m_checksumErrors{};
    };

} // namespace pi_tracker