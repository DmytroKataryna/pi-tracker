#include "pi_tracker/serial_port.h"
#include "pi_tracker/tf_luna.h"

#include <array>
#include <chrono>
#include <format>
#include <iostream>

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;

int main()
{
    pi_tracker::SerialPort port{"/dev/ttyAMA0", 115200};
    pi_tracker::TfLunaParser parser;

    int frames = 0;
    int unreliable = 0;
    int emptyReads = 0;
    const auto start = Clock::now();

    while (Clock::now() - start < 10s)
    {
        std::array<std::uint8_t, 64> buffer{};
        const std::size_t n = port.read(buffer);
        if (n == 0)
        {
            ++emptyReads; // 0.1 с тиші: лідар відключився або не та швидкість
            continue;
        }

        for (std::size_t i = 0; i < n; ++i)
        {
            if (auto frame = parser.feed(buffer[i]))
            {
                ++frames;
                if (frame->amplitude < static_cast<std::uint16_t>(100) || frame->amplitude == 65535)
                {
                    ++unreliable;
                    continue;
                }

                if (frames % 20 == 0)
                {
                    std::cout << std::format("distance: {:4} cm   amp: {:5}   temp: {:.1f} °C\n",
                                             frame->distanceCm, frame->amplitude, frame->temperatureC);
                }
            }
        }
    }

    std::cerr << std::format("frames: {}, unreliable: {}, checksum errors: {}, empty reads: {}\n",
                             frames, unreliable, parser.checksumErrors(), emptyReads);
}