#pragma once

#include "pi_tracker/fd.h"

#include <cstdint>
#include <span>

namespace pi_tracker
{

    // Пристрій на шині I2C з регістровим протоколом (IMU, магнітометри, EEPROM...)
    class I2cDevice
    {
    public:
        // Відкриває шину (наприклад, "/dev/i2c-1"); кидає std::system_error, якщо не вдалося
        I2cDevice(const char *bus, std::uint16_t address);

        // Читає out.size() байтів, починаючи з регістра startRegister, однією транзакцією
        void readRegisters(std::uint8_t startRegister, std::span<std::uint8_t> out) const;
        std::uint8_t readRegister(std::uint8_t reg) const;
        void writeRegister(std::uint8_t reg, std::uint8_t value) const;

        void write(std::span<const std::uint8_t> data) const; // одне повідомлення-запис
        void read(std::span<std::uint8_t> out) const;         // одне повідомлення-читання (I2C_M_RD)
        
    private:
        Fd m_fd; // RAII: файл шини закриється разом з об'єктом
        std::uint16_t m_address;
    };

} // namespace pi_tracker