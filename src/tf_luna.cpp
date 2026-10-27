#include "pi_tracker/tf_luna.h"

namespace pi_tracker
{

    namespace
    {
        constexpr std::uint8_t kHeader = 0x59;

        std::uint16_t toUint16(std::uint8_t low, std::uint8_t high)
        {
            return static_cast<std::uint16_t>((high << 8) | low);
        }
    }

    std::optional<TfLunaFrame> TfLunaParser::feed(std::uint8_t byte)
    {
        if (m_index == 0 || m_index == 1)
        {
            if (byte == kHeader)
            {
                m_frame[m_index] = byte;
                m_index += 1;
            }
            else
            {
                m_frame = std::array<std::uint8_t, 9>{};
                m_index = 0;
            }
            return std::nullopt;
        }

        if (m_index != 9)
        {
            m_frame[m_index] = byte;
            m_index += 1;
        }

        if (m_index == 9)
        {
            m_index = 0;
            auto sum = m_frame[0] + m_frame[1] + m_frame[2] + m_frame[3] + m_frame[4] + m_frame[5] + m_frame[6] + m_frame[7];
            if (static_cast<std::uint8_t>(sum) != m_frame[8])
            {
                ++m_checksumErrors;
                return std::nullopt;
            }

            const double temperatureC = toUint16(m_frame[6], m_frame[7]) / 8.0 - 256.0;
            const std::uint16_t distanceCm = toUint16(m_frame[2], m_frame[3]);
            const std::uint16_t amplitude = toUint16(m_frame[4], m_frame[5]);

            return TfLunaFrame(distanceCm, amplitude, temperatureC);
        }
        else
        {
            return std::nullopt;
        }

        return std::nullopt;
    }

}