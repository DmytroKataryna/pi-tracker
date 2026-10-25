#include "pi_tracker/imu.h"

#include <format>
#include <thread>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <iomanip>

using namespace std::chrono_literals;

namespace pi_tracker
{

    namespace
    {
        namespace reg
        {
            constexpr std::uint8_t kSmplrtDiv = 0x19;
            constexpr std::uint8_t kConfig = 0x1A;
            constexpr std::uint8_t kGyroConfig = 0x1B;
            constexpr std::uint8_t kAccelConfig = 0x1C;
            constexpr std::uint8_t kAccelConfig2 = 0x1D;
            constexpr std::uint8_t kAccelXoutH = 0x3B;
            constexpr std::uint8_t kPwrMgmt1 = 0x6B;
            constexpr std::uint8_t kWhoAmI = 0x75;
        } // namespace reg

        constexpr std::uint8_t kExpectedWhoAmI = 0x70;
        constexpr double kStandardGravity = 9.80665; // м/с² на 1 g

        std::int16_t toInt16(std::uint8_t high, std::uint8_t low)
        {
            return static_cast<std::int16_t>((high << 8) | low);
        }

    } // namespace

    Imu::Imu(const char *bus, std::uint16_t address, AccelRange accelRange, GyroRange gyroRange)
        : m_device{bus, address},
          m_accelScale{kStandardGravity / (16384.0 / (1 << static_cast<int>(accelRange)))},
          m_gyroScale{1.0 / (131.0 / (1 << static_cast<int>(gyroRange)))}
    {

        m_device.writeRegister(reg::kPwrMgmt1, 0x80);
        std::this_thread::sleep_for(100ms);

        m_device.writeRegister(reg::kPwrMgmt1, 0x01);
        std::this_thread::sleep_for(50ms);

        uint8_t i_am = m_device.readRegister(reg::kWhoAmI);
        if (i_am != kExpectedWhoAmI)
        {
            throw std::runtime_error(std::format("Imu: unexpected WHO_AM_I {:#04x}", i_am));
        }

        m_device.writeRegister(reg::kConfig, 0x03);
        m_device.writeRegister(reg::kSmplrtDiv, 9);

        m_device.writeRegister(reg::kGyroConfig, static_cast<std::uint8_t>(static_cast<std::uint8_t>(gyroRange) << 3));
        m_device.writeRegister(reg::kAccelConfig, static_cast<std::uint8_t>(static_cast<std::uint8_t>(accelRange) << 3));
        m_device.writeRegister(reg::kAccelConfig2, 0x03);
        std::this_thread::sleep_for(50ms);
    }

    ImuReading Imu::read() const
    {
        std::array<uint8_t, 14> out = {};
        m_device.readRegisters(reg::kAccelXoutH, out);

        int16_t ax = toInt16(out[0], out[1]);
        int16_t ay = toInt16(out[2], out[3]);
        int16_t az = toInt16(out[4], out[5]);
        int16_t temp = toInt16(out[6], out[7]);
        int16_t gx = toInt16(out[8], out[9]);
        int16_t gy = toInt16(out[10], out[11]);
        int16_t gz = toInt16(out[12], out[13]);

        std::array<double, 3> accel{
            ax * m_accelScale,
            ay * m_accelScale,
            az * m_accelScale,
        };
        std::array<double, 3> gyro{
            gx * m_gyroScale - m_gyroBias[0],
            gy * m_gyroScale - m_gyroBias[1],
            gz * m_gyroScale - m_gyroBias[2],
        };

        ImuReading reading{};
        reading.timestamp = std::chrono::steady_clock::now();
        reading.accel = accel;
        reading.gyro = gyro;
        reading.temperature = static_cast<double>(temp) / 333.87 + 21.0;
        return reading;
    }

    void Imu::calibrateGyro(int samples)
    {
        m_gyroBias = std::array<double, 3>{};

        double gx_sum = 0.0;
        double gy_sum = 0.0;
        double gz_sum = 0.0;
        for (auto i = 0; i < samples; i++)
        {
            ImuReading reading = read();
            gx_sum += reading.gyro[0];
            gy_sum += reading.gyro[1];
            gz_sum += reading.gyro[2];

            std::this_thread::sleep_for(10ms);
        }

        double gx_bias = gx_sum / static_cast<double>(samples);
        double gy_bias = gy_sum / static_cast<double>(samples);
        double gz_bias = gz_sum / static_cast<double>(samples);

        m_gyroBias = std::array<double, 3>{gx_bias, gy_bias, gz_bias};
    }

} // namespace pi_tracker