////////////////////////////////////////////////////////////////////////////////
/// \file      GUdpServer.hpp
/// \version   0.1
/// \date      July, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GUDPSERVER_HPP_
#define GUDPSERVER_HPP_

#include <cstdint>      // uint16_t
#include <sys/socket.h> // sockaddr_storage, socklen_t

class GUdpServer {
    public:
    GUdpServer(const char *local_addr, uint16_t local_port, const char *tag_name = nullptr);
    ~GUdpServer();

    auto IsReady() {
        return m_is_ready;
    }

    auto TagName() {
        return m_tag_name;
    }

    bool Receive(void *dst_buffer, size_t *dst_bytes);

    bool Send(void *src_buffer, size_t src_bytes);

    private:
    char                    m_tag_name[64];
    bool                    m_is_ready{false};
    int                     m_socket_fd{-1};
    struct sockaddr_storage m_peer_addr {};
    socklen_t               m_peer_addr_len{sizeof(struct sockaddr_storage)};
};

#endif // GUDPSERVER_HPP_
