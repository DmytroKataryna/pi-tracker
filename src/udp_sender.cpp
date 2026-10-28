#include "pi_tracker/udp_sender.h"

#include <cerrno>
#include <stdexcept>
#include <system_error>

#include <arpa/inet.h>  // inet_pton, htons
#include <sys/socket.h> // socket, sendto

namespace pi_tracker
{

    UdpSender::UdpSender(const char *host, std::uint16_t port)
        : m_fd{::socket(AF_INET, SOCK_DGRAM, 0)} // AF_INET — IPv4, SOCK_DGRAM — UDP
    {
        if (!m_fd.isValid())
        {
            throw std::system_error(errno, std::generic_category(), "socket");
        }

        m_address.sin_family = AF_INET;
        m_address.sin_port = htons(port);                         // host to network short: порт у мережевому порядку байтів (big-endian)
        if (::inet_pton(AF_INET, host, &m_address.sin_addr) != 1) // "192.168.0.10" → 4 байти
        {
            throw std::invalid_argument("UdpSender: invalid IPv4 address");
        }
    }

    bool UdpSender::send(std::string_view message) const
    {
        const ssize_t sent = ::sendto(m_fd.get(),
                                      message.data(),
                                      message.size(),
                                      0,
                                      reinterpret_cast<const sockaddr *>(&m_address),
                                      sizeof(m_address));
        return sent >= 0;
    }

} // namespace pi_tracker