#include "pi_tracker/tf_luna.h"

#include <cstdint>
#include <format>
#include <iostream>
#include <vector>

int main()
{
    const std::vector<std::uint8_t> stream{
        0x00,
        0x7F,
        0x12, // сміття: почали читати посеред потоку
        0x59,
        0x59,
        0x64,
        0x00,
        0xE8,
        0x03,
        0xA0,
        0x08,
        0xA9, // коректний: 100 см, amp 1000, 20 °C
        0x59,
        0x59,
        0x64,
        0x00,
        0xE8,
        0x03,
        0xA0,
        0x08,
        0x00, // зіпсована контрольна сума
        0x59,
        0x59,
        0xC8,
        0x00,
        0x10,
        0x27,
        0xA0,
        0x08,
        0x59, // коректний: 200 см, amp 10000, 20 °C
    };

    pi_tracker::TfLunaParser parser;
    int frames = 0;
    for (const std::uint8_t b : stream)
    {
        if (auto f = parser.feed(b))
        {
            ++frames;
            std::cout << std::format("frame: {} cm, amp {}, {:.1f} C\n", f->distanceCm, f->amplitude, f->temperatureC);
        }
    }
    std::cout << std::format("frames: {}, checksum errors: {}\n", frames, parser.checksumErrors());
}