#include <utility> // std::move
#include <vector>
#include <array>
#include <cerrno>  // errno — код останньої помилки системного виклику
#include <cstring> // std::strerror — текст помилки
#include <iostream>
#include <string>

#include <fcntl.h>  // open, O_RDONLY (системні виклики Linux, C API)
#include <unistd.h> // read, close

class Fd
{
public:
    // explicit забороняє неявні перетворення: Fd f = "/etc/hostname"; не скомпілюється
    explicit Fd(const char *path, int flags = O_RDONLY)
        : m_fd{::open(path, flags)} // member initializer list: ініціалізуємо поле до тіла конструктора
    {
        if (m_fd < 0)
        {
            std::cerr << "open(" << path << ") failed: " << std::strerror(errno) << '\n';
        }
        else
        {
            std::cout << "[Fd] opened " << path << " -> fd " << m_fd << '\n';
        }
    }

    Fd(Fd &&other) noexcept // && — r-value посилання: «об'єкт, з якого можна забрати ресурс»
        : m_fd{other.m_fd}
    {
        other.m_fd = -1; // тепер деструктор other нічого не закриє
    }

    Fd &operator=(Fd &&other) noexcept
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

    ~Fd()
    {
        if (m_fd >= 0)
        {
            ::close(m_fd);
            std::cout << "[Fd] closed fd " << m_fd << '\n';
        }
    }

    Fd(const Fd &) = delete;
    Fd &operator=(const Fd &) = delete;

    bool isValid() const
    {
        return m_fd >= 0;
    }

    int get() const
    {
        return m_fd;
    }

private:
    int m_fd{-1}; // -1 означає «нічого не відкрито»
};

Fd openThermal()
{
    Fd fd{"/sys/class/thermal/thermal_zone0/temp"};
    std::cout << "opened inside function, valid: " << fd.isValid() << '\n';
    return fd;
}

int main()
{
    Fd a = openThermal(); // 1. повернення з функції
    Fd b = std::move(a);  // 2. явна передача володіння
    std::cout << "a valid: " << a.isValid() << ", b valid: " << b.isValid() << '\n';

    std::vector<Fd> devices; // 3. зберігання у векторі
    devices.push_back(Fd{"/etc/hostname"});
    devices.push_back(Fd{"/etc/os-release"});
    std::cout << "devices: " << devices.size() << '\n';

    Fd c{"/etc/hostname"};
    c = std::move(b); // 4. move-присвоєння: c закриває свій файл і забирає b
    std::cout << "c valid: " << c.isValid() << ", b valid: " << b.isValid() << '\n';
    std::cout << "end of main\n";
}