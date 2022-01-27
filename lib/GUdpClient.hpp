////////////////////////////////////////////////////////////////////////////////
/// \file      GUdpClient.hpp
/// \version   0.1
/// \date      August, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GUDPCLIENT_HPP
#define GUDPCLIENT_HPP

#include <cstdint>  // uint16_t
#include <stddef.h> // size_t

class GUdpClient {
    public:
    // Maximum UDP datagram size: 65507 = (2^16 - 1) -20 (UDP header) - 8 (IPv4 header)
    static const size_t MAX_DATAGRAM_SIZE = 65507;

    GUdpClient(const char *remote_addr, uint16_t remote_port, const char *tag_name = nullptr);

    ~GUdpClient();

    auto IsReady() {
        return m_is_ready;
    }

    auto TagName() {
        return m_tag_name;
    }

    bool Receive(void *dst_buffer, size_t *dst_bytes);

    bool Send(void *src_buffer, size_t src_bytes);

    void Stop();

    private:
    char m_tag_name[64];
    bool m_is_ready{false};
    int  m_socket_fd{-1};
};

#endif // GUDPCLIENT_HPP
