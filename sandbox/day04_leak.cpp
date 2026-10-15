#include <cerrno>
#include <cstring>
#include <iostream>
#include <fcntl.h>

int main()
{
    for (int i = 0; i < 2000; ++i)
    {
        const int fd = ::open("/etc/hostname", O_RDONLY); // відкрили і «забули» закрити
        if (fd < 0)
        {
            std::cerr << "Спроба " << i << ": " << std::strerror(errno) << '\n';
            return 1;
        }
    }
    std::cout << "OK\n";
}