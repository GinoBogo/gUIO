
////////////////////////////////////////////////////////////////////////////////
/// \file      GUdpClient.hpp
/// \version   0.1
/// \date      August, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GUDPCLIENT_HPP
#define GUDPCLIENT_HPP

#include <cstddef> // size_t
#include <cstdint> // uint16_t

class GUdpClient {
    public:
    // Maximum UDP datagram size: 65507 = (2^16 - 1) - 20 (UDP header) - 8 (IPv4 header)
    static const size_t MAX_DATAGRAM_SIZE = ((2 << 15) - 1) - 20 - 8;

    GUdpClient(const char* remote_addr, uint16_t remote_port, const char* tag_name = nullptr);

    ~GUdpClient();

    [[nodiscard]] auto IsReady() const {
        return m_is_ready;
    }

    [[nodiscard]] auto TagName() const {
        return m_tag_name;
    }

    [[nodiscard]] auto address() const {
        return s_addr;
    }

    [[nodiscard]] auto port() const {
        return s_port;
    }

    bool Receive(void* dst_buffer, size_t* dst_bytes) const;

    bool Send(void* src_buffer, size_t src_bytes) const;

    void Stop();

    private:
    char s_addr[32];
    char s_port[16];
    char m_tag_name[64];
    bool m_is_ready{false};
    int  m_socket_fd{-1};
};

#endif // GUDPCLIENT_HPP
