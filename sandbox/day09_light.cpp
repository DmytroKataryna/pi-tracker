#include "pi_tracker/i2c_device.h"

#include <thread> // Для std::this_thread::sleep_for
#include <chrono> // Для std::chrono::milliseconds та літералу 'ms'

using namespace std::chrono_literals; // Дозволяє писати 180ms замість std::chrono::milliseconds(180)

#include <array>
#include <cstdint>
#include <format> // std::format (C++20), як String.format у Java
#include <iostream>

namespace bh1750
{
    constexpr std::uint16_t kAddress = 0x23;
    constexpr double kCountsPerLux = 1.2;
    constexpr std::array<std::uint8_t, 1> kPowerOn{0x01};
    constexpr std::array<std::uint8_t, 1> kContinuousHighRes{0x10};
}

std::int16_t toInt16(std::uint8_t high, std::uint8_t low)
{
    return static_cast<std::int16_t>((static_cast<std::uint16_t>(high) << 8) | low);
}

int main()
{
    pi_tracker::I2cDevice light{"/dev/i2c-1", bh1750::kAddress};
    light.write(bh1750::kPowerOn);
    light.write(bh1750::kContinuousHighRes);
    std::this_thread::sleep_for(180ms);

    while (true)
    {
        std::array<std::uint8_t, 2> raw{};
        light.read(raw);
        const double lux = toInt16(raw[0], raw[1]) / bh1750::kCountsPerLux;
        std::cout << std::format("light: {:8.1f} lux\n", lux); // '\n' → рядок одразу на екрані
        std::this_thread::sleep_for(500ms);
    }
}