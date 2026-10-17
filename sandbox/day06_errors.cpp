#include "pi_tracker/fd.h"

#include <iostream>
#include <stdexcept>
#include <system_error>

// Друкує повідомлення в деструкторі, щоб було видно, коли його знищено
struct Tracer
{
    ~Tracer() { std::cout << "[Tracer] destroyed\n"; }
};

void processSensor()
{
    Tracer tracer;
    pi_tracker::Fd fd{"/etc/hostname"};
    std::cout << "opened fd " << fd.get() << '\n';
    throw std::runtime_error("sensor timeout"); // щось пішло не так
    std::cout << "never printed\n";
}

int main()
{
    try
    {
        processSensor();
    }
    catch (const std::exception &e)
    {
        std::cout << "caught: " << e.what() << '\n';
    }

    try
    {
        pi_tracker::Fd missing{"/no/such/file"};
        std::cout << "never printed\n";
    }
    catch (const std::system_error &e)
    {
        std::cout << "caught: " << e.what() << " (errno " << e.code().value() << ")\n";
    }
}