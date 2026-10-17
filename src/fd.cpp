#include "pi_tracker/fd.h"

#include <cerrno>
#include <system_error>

#include <unistd.h>

namespace pi_tracker
{

    Fd::Fd(const char *path, int flags)
        : m_fd{::open(path, flags)}
    {
        if (m_fd < 0)
        {
            throw std::system_error(errno, std::generic_category(), path);
        }
    }

    Fd &Fd::operator=(Fd &&other) noexcept
    {
        if (this == &other)
        {
            return other;
        }
        if (this->m_fd >= 0)
        {
            ::close(m_fd);
        }
        this->m_fd = other.m_fd;
        other.m_fd = -1;
        return *this;
    }

    Fd::Fd(Fd &&other) noexcept // && — r-value посилання: «об'єкт, з якого можна забрати ресурс»
        : m_fd{other.m_fd}
    {
        other.m_fd = -1; // тепер деструктор other нічого не закриє
    }

    Fd::~Fd()
    {
        if (m_fd >= 0)
        {
            ::close(m_fd);
        }
    }
}