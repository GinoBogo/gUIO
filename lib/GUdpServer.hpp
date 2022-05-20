
////////////////////////////////////////////////////////////////////////////////
/// \file      GUdpServer.hpp
/// \version   0.1
/// \date      July, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GUDPSERVER_HPP
#define GUDPSERVER_HPP

#include <cstdint>      // uint16_t
#include <sys/socket.h> // sockaddr_storage, socklen_t

class GUdpServer {
    public:
    // Maximum UDP datagram size: 65507 = (2^16 - 1) -20 (UDP header) - 8 (IPv4 header)
    static const size_t MAX_DATAGRAM_SIZE = 65507;

    GUdpServer(const char* local_addr, uint16_t local_port, const char* tag_name = nullptr);

    ~GUdpServer();

    [[nodiscard]] auto IsReady() const {
        return m_is_ready;
    }

    [[nodiscard]] auto TagName() const {
        return m_tag_name;
    }

    bool Receive(void* dst_buffer, size_t* dst_bytes);

    bool Send(void* src_buffer, size_t src_bytes);

    void Stop();

    private:
    char                    s_addr[32];
    char                    s_port[16];
    char                    m_tag_name[64];
    bool                    m_is_ready{false};
    int                     m_socket_fd{-1};
    struct sockaddr_storage m_peer_addr {};
    socklen_t               m_peer_addr_len{sizeof(struct sockaddr_storage)};
};

#endif // GUDPSERVER_HPP
