////////////////////////////////////////////////////////////////////////////////
/// \file      GUdpClient.cpp
/// \version   0.1
/// \date      August, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GUdpClient.hpp"

#include "GLogger.hpp"

#include <errno.h>  // errno
#include <netdb.h>  // addrinfo
#include <stdio.h>  // snprintf
#include <string.h> // bzero
#include <unistd.h> // close

// Maximum size of UDP datagram: 65507 = (2^16 - 1) -20 (UDP header) - 8 (IPv4 header)
#define UDP_MAX_SIZE 65507

GUdpClient::GUdpClient(const char *remote_addr, uint16_t remote_port, const char *tag_name) {
    if (tag_name != nullptr) {
        snprintf(m_tag_name, sizeof(m_tag_name), "\"%s\" UDP Client", tag_name);
    }
    else {
        snprintf(m_tag_name, sizeof(m_tag_name), "UDP Client");
    }

    struct addrinfo hints, *res;

    bzero(&hints, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    char s_addr[32];
    char s_port[16];

    bzero(s_addr, sizeof(s_addr));
    bzero(s_port, sizeof(s_port));

    snprintf(s_addr, sizeof(s_addr), "%s", remote_addr);
    snprintf(s_port, sizeof(s_port), "%u", remote_port);

    if (!strlen(s_addr)) {
        strcpy(s_addr, "127.0.0.1");
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

    if (connect(m_socket_fd, res->ai_addr, res->ai_addrlen) == -1) {
        LOG_WRITE(error, strerror(errno));
        goto free_and_exit;
    }

    m_is_ready = true;
    LOG_FORMAT(debug, "%s constructor (%s:%s)", m_tag_name, s_addr, s_port);

free_and_exit:
    freeaddrinfo(res);
}

GUdpClient::~GUdpClient() {
    if (m_socket_fd != -1) {
        close(m_socket_fd);
        LOG_FORMAT(debug, "%s closed", m_tag_name);
    }
    LOG_FORMAT(debug, "%s destructor", m_tag_name);
}

bool GUdpClient::Receive(void *dst_buffer, size_t *dst_bytes) {
    if (!m_is_ready || dst_buffer == nullptr || dst_bytes == nullptr) {
        return false;
    }

    auto bytes{recv(m_socket_fd, dst_buffer, UDP_MAX_SIZE, 0)};
    if (bytes == -1) {
        return false;
    }

    *dst_bytes = static_cast<size_t>(bytes);
    return true;
}

bool GUdpClient::Send(void *src_buffer, size_t src_bytes) {
    if (!m_is_ready || src_buffer == nullptr) {
        return false;
    }

    auto bytes{send(m_socket_fd, src_buffer, src_bytes, 0)};
    if (bytes == -1) {
        return false;
    }

    return true;
}
