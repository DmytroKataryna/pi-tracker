#pragma once

#include "pi_tracker/fd.h"

#include <cstdint>
#include <string_view>

#include <netinet/in.h> // sockaddr_in

namespace pi_tracker
{

    // Надсилає UDP-датаграми на одну IPv4-адресу
    class UdpSender
    {
    public:
        // Кидає std::system_error, якщо сокет не створюється; std::invalid_argument — якщо адреса некоректна
        UdpSender(const char *host, std::uint16_t port);

        // Одне повідомлення = одна датаграма. false — не відправлено (наприклад, мережа недоступна)
        bool send(std::string_view message) const;

    private:
        Fd m_fd;
        sockaddr_in m_address{};
    };

} // namespace pi_tracker