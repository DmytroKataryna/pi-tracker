#include "pi_tracker/i2c_device.h"

#include <array>
#include <cstdint>
#include <format> // std::format (C++20), як String.format у Java
#include <iostream>

namespace mpu6500
{
    constexpr std::uint16_t kAddress = 0x68;
    constexpr std::uint8_t kWhoAmI = 0x75;
    constexpr std::uint8_t kPwrMgmt1 = 0x6B;
    constexpr std::uint8_t kAccelXoutH = 0x3B; // початок 14 байтів: accel, temp, gyro

    constexpr double kAccelLsbPerG = 16384.0; // ±2g
    constexpr double kGyroLsbPerDps = 131.0;  // ±250 °/с
}

std::int16_t toInt16(std::uint8_t high, std::uint8_t low)
{
    return static_cast<std::int16_t>((static_cast<std::uint16_t>(high) << 8) | low);
}

int main()
{
    pi_tracker::I2cDevice imu{"/dev/i2c-1", mpu6500::kAddress};

    // {:#04x} — шістнадцятковий формат з префіксом 0x, ширина 4: 0x70
    std::cout << std::format("WHO_AM_I   = {:#04x}\n", imu.readRegister(mpu6500::kWhoAmI));
    std::cout << std::format("PWR_MGMT_1 = {:#04x}\n", imu.readRegister(mpu6500::kPwrMgmt1));

    std::array<std::uint8_t, 14> raw{};
    imu.readRegisters(mpu6500::kAccelXoutH, raw);

    for (size_t i = 0; i < raw.size(); i++)
    {
        // std::cout << "Byte: " << i << " V: " << static_cast<int>(raw[i]) << "\n";
    }

    double ax = static_cast<double>(toInt16(raw[0], raw[1])) / mpu6500::kAccelLsbPerG;
    double ay = static_cast<double>(toInt16(raw[2], raw[3])) / mpu6500::kAccelLsbPerG;
    double az = static_cast<double>(toInt16(raw[4], raw[5])) / mpu6500::kAccelLsbPerG;

    double temp = static_cast<double>(toInt16(raw[6], raw[7])) / 333.87 + 21.0;

    double gx = static_cast<double>(toInt16(raw[8], raw[9])) / mpu6500::kGyroLsbPerDps;
    double gy = static_cast<double>(toInt16(raw[10], raw[11])) / mpu6500::kGyroLsbPerDps;
    double gz = static_cast<double>(toInt16(raw[12], raw[13])) / mpu6500::kGyroLsbPerDps;

    std::cout << "ax: " << std::format("{:+.3f}\n", ax);
    std::cout << "ay: " << std::format("{:+.3f}\n", ay);
    std::cout << "az: " << std::format("{:+.3f}\n", az);

    std::cout << "temp: " << std::format("{:+.3f}\n", temp);

    std::cout << "gx: " << std::format("{:+.3f}\n", gx);
    std::cout << "gy: " << std::format("{:+.3f}\n", gy);
    std::cout << "gz: " << std::format("{:+.3f}\n", gz);
}