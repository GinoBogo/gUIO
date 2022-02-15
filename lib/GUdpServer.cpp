////////////////////////////////////////////////////////////////////////////////
/// \file      GUdpServer.cpp
/// \version   0.1
/// \date      July, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GUdpServer.hpp"

#include "GLogger.hpp"
#include "GUdpClient.hpp"

#include <errno.h>  // errno
#include <netdb.h>  // addrinfo
#include <stdio.h>  // snprintf
#include <stdlib.h> // atoi
#include <string.h> // bzero
#include <unistd.h> // close

GUdpServer::GUdpServer(const char *local_addr, uint16_t local_port, const char *tag_name) {
    if (tag_name != nullptr) {
        snprintf(m_tag_name, sizeof(m_tag_name), "\"%s\" UDP Server", tag_name);
    }
    else {
        snprintf(m_tag_name, sizeof(m_tag_name), "UDP Server");
    }

    struct addrinfo hints, *res;

    bzero(&hints, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;

    bzero(s_addr, sizeof(s_addr));
    bzero(s_port, sizeof(s_port));

    snprintf(s_addr, sizeof(s_addr), "%s", local_addr);
    snprintf(s_port, sizeof(s_port), "%u", local_port);

    if (!strlen(s_addr)) {
        strcpy(s_addr, "0.0.0.0");
    }

    auto error_code{getaddrinfo(s_addr, s_port, &hints, &res)};
    if (error_code != 0) {
        LOG_WRITE(error, gai_strerror(error_code));
        goto free_and_exit;
    }

    m_socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (m_socket_fd == -1) {
        LOG_WRITE(error, strerror(errno));
        goto free_and_exit;
    }

    if (bind(m_socket_fd, res->ai_addr, res->ai_addrlen) == -1) {
        LOG_WRITE(error, strerror(errno));
        goto free_and_exit;
    }

    m_is_ready = true;
    LOG_FORMAT(debug, "%s constructor (%s:%s)", m_tag_name, s_addr, s_port);

free_and_exit:
    freeaddrinfo(res);
}

GUdpServer::~GUdpServer() {
    if (m_socket_fd != -1) {
        close(m_socket_fd);
        LOG_FORMAT(debug, "%s closed", m_tag_name);
    }
    LOG_FORMAT(debug, "%s destructor", m_tag_name);
}

bool GUdpServer::Receive(void *dst_buffer, size_t *dst_bytes) {
    if (!m_is_ready || dst_buffer == nullptr || dst_bytes == nullptr) {
        return false;
    }

    auto _addr{(struct sockaddr *)&m_peer_addr};
    auto bytes{recvfrom(m_socket_fd, dst_buffer, MAX_DATAGRAM_SIZE, 0, _addr, &m_peer_addr_len)};
    if (bytes == -1) {
        return false;
    }

    *dst_bytes = static_cast<size_t>(bytes);
    return true;
}

bool GUdpServer::Send(void *src_buffer, size_t src_bytes) {
    if (!m_is_ready || src_buffer == nullptr) {
        return false;
    }

    auto _addr{(struct sockaddr *)&m_peer_addr};
    auto bytes{sendto(m_socket_fd, src_buffer, src_bytes, 0, _addr, m_peer_addr_len)};
    if (bytes == -1) {
        return false;
    }

    return true;
}

void GUdpServer::Stop() {
    if (m_socket_fd != -1) {
        auto client = GUdpClient(s_addr, (uint16_t)atoi(s_port));

        char msg;
        client.Send(&msg, 0);
    }
}
