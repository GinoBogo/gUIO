////////////////////////////////////////////////////////////////////////////////
/// \file      GUdpClient.hpp
/// \version   0.1
/// \date      August, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GUDPCLIENT_HPP_
#define GUDPCLIENT_HPP_

#include <cstdint>  // uint16_t
#include <stddef.h> // size_t

class GUdpClient {
    public:
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

    private:
    char m_tag_name[64];
    bool m_is_ready{false};
    int  m_socket_fd{-1};
};

#endif // GUDPCLIENT_HPP_
