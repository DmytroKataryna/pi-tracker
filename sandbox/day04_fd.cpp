#include <array>
#include <cerrno>  // errno — код останньої помилки системного виклику
#include <cstring> // std::strerror — текст помилки
#include <iostream>
#include <string>

#include <fcntl.h>  // open, O_RDONLY (системні виклики Linux, C API)
#include <unistd.h> // read, close

// Власник файлового дескриптора: відкриває в конструкторі, закриває в деструкторі
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

    ~Fd()
    {
        if (m_fd >= 0)
        {
            ::close(m_fd);
            std::cout << "[Fd] closed fd " << m_fd << '\n';
        }
    }

    // Забороняємо копіювати (пояснення нижче)
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

// Читає до 64 байтів з дескриптора і повертає їх як рядок
std::string readText(const Fd &fd)
{
    std::array<char, 64> buffer{};
    // read() — C API: приймає ВКАЗІВНИК на буфер (buffer.data()) і його розмір
    const ssize_t n = ::read(fd.get(), buffer.data(), buffer.size());
    if (n <= 0)
    {
        return {};
    }
    return std::string(buffer.data(), static_cast<std::size_t>(n));
}

int main()
{
    std::cout << "before block\n";
    {
        Fd temp{"/sys/class/thermal/thermal_zone0/temp"};
        if (temp.isValid())
        {
            std::cout << "CPU temp (m°C): " << readText(temp);
        }
        std::cout << "end of block\n";
    } // ← тут temp виходить з області видимості
    std::cout << "after block\n";

    Fd missing{"/no/such/file"};
    std::cout << "missing valid? " << missing.isValid() << '\n';
}