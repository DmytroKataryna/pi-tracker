#include "pi_tracker/i2c_device.h"

#include <array>
#include <cerrno>
#include <system_error>
#include <iostream>

#include <linux/i2c.h>     // struct i2c_msg, I2C_M_RD
#include <linux/i2c-dev.h> // I2C_RDWR, struct i2c_rdwr_ioctl_data
#include <sys/ioctl.h>     // ioctl

namespace pi_tracker
{

    namespace // анонімний namespace: функція видна тільки в цьому .cpp (як private top-level у Kotlin)
    {

        // Виконує кілька I2C-повідомлень однією транзакцією (між ними — REPEATED START, а не STOP)
        void transfer(int fd, std::span<i2c_msg> messages)
        {
            i2c_rdwr_ioctl_data transaction{messages.data(), static_cast<__u32>(messages.size())};
            if (::ioctl(fd, I2C_RDWR, &transaction) < 0)
            {
                throw std::system_error(errno, std::generic_category(), "I2C_RDWR");
            }
        }

    } // namespace

    I2cDevice::I2cDevice(const char *bus, std::uint16_t address)
        : m_fd{bus, O_RDWR}, m_address{address}
    {
    }

    void I2cDevice::readRegisters(std::uint8_t startRegister, std::span<std::uint8_t> out) const
    {
        // Два повідомлення = рівно та послідовність з Частини 1:
        std::array<i2c_msg, 2> messages{{
            // 1) START → [адреса + W] → [номер регістра]
            {m_address, 0, 1, &startRegister},
            // 2) REPEATED START → [адреса + R] → out.size() байтів → NACK → STOP
            {m_address, I2C_M_RD, static_cast<__u16>(out.size()), out.data()},
        }};
        pi_tracker::transfer(m_fd.get(), messages);
    }

    std::uint8_t I2cDevice::readRegister(std::uint8_t reg) const
    {
        std::array<std::uint8_t, 1> value{};
        readRegisters(reg, value);
        return value[0];
    }

    void I2cDevice::writeRegister(std::uint8_t reg, std::uint8_t value) const
    {
        std::array<std::uint8_t, 2> data{reg, value};
        std::array<i2c_msg, 1> messages{{
            {m_address, 0, static_cast<__u16>(data.size()), data.data()},
        }};
        transfer(m_fd.get(), messages);
    }

    void I2cDevice::write(std::span<const std::uint8_t> data) const
    {
        std::array<i2c_msg, 1> messages{{
            {m_address, 0, static_cast<__u16>(data.size()), const_cast<std::uint8_t *>(data.data())},
        }};
        transfer(m_fd.get(), messages);
    }

    void I2cDevice::read(std::span<std::uint8_t> out) const
    {
        std::array<i2c_msg, 1> messages{{
            {m_address, I2C_M_RD, static_cast<__u16>(out.size()), out.data()},
        }};
        transfer(m_fd.get(), messages);
    }

}