#pragma once

#include "pi_tracker/i2c_device.h"

#include <array>
#include <chrono>
#include <cstdint>

namespace pi_tracker
{

    enum class AccelRange : std::uint8_t
    {
        G2 = 0,
        G4 = 1,
        G8 = 2,
        G16 = 3
    };
    enum class GyroRange : std::uint8_t
    {
        Dps250 = 0,
        Dps500 = 1,
        Dps1000 = 2,
        Dps2000 = 3
    };

    // Одне вимірювання у фізичних одиницях
    struct ImuReading
    {
        std::chrono::steady_clock::time_point timestamp;
        std::array<double, 3> accel{}; // м/с², X Y Z
        std::array<double, 3> gyro{};  // °/с, X Y Z (з урахуванням калібрування)
        double temperature{};          // °C
    };

    // Драйвер MPU-6500 поверх I2C
    class Imu
    {
    public:
        // Скидає і налаштовує чип; кидає виняток, якщо WHO_AM_I не той
        Imu(const char *bus, std::uint16_t address, AccelRange accelRange, GyroRange gyroRange);

        ImuReading read() const;

        // Усереднює `samples` вимірювань гіроскопа в спокої і запам'ятовує їх як зміщення нуля
        void calibrateGyro(int samples);
        const std::array<double, 3> &gyroBias() const { return m_gyroBias; }

    private:
        I2cDevice m_device;
        double m_accelScale; // м/с² на одну одиницю сирого значення
        double m_gyroScale;  // °/с на одну одиницю сирого значення
        std::array<double, 3> m_gyroBias{};
    };

} // namespace pi_tracker