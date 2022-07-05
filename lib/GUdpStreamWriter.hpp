
////////////////////////////////////////////////////////////////////////////////
/// \file      GUdpStreamWriter.hpp
/// \version   0.1
/// \date      July, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GUDPSTREAMWRITER_HPP
#define GUDPSTREAMWRITER_HPP

#include <cerrno>    // errno
#include <cstring>   // memset, strerror
#include <netdb.h>   // addrinfo
#include <streambuf> // basic_streambuf<char>
#include <unistd.h>  // close

class GUdpStreamWriter : public std::streambuf {
    using base = std::streambuf;

    public:
    // Maximum UDP datagram size: 65507 = (2^16 - 1) - 20 (UDP header) - 8 (IPv4 header)
    static const size_t MAX_DATAGRAM_SIZE = ((2 << 15) - 1) - 20 - 8;
    // Maximum Transmission Unit: 1500 (Ethernet II frame format) - 20 (UDP header) - 8 (IPv4 header)
    static const size_t MTU_DATAGRAM_SIZE = 1500 - 20 - 8;

    GUdpStreamWriter(const std::string& addr, uint16_t port, size_t length = MTU_DATAGRAM_SIZE) {
        open(addr, port, length);
    }

    GUdpStreamWriter(const char* addr, uint16_t port, size_t length = MTU_DATAGRAM_SIZE) {
        if (addr != nullptr && port > 0) {
            open(addr, port, length);
        }
    }

    GUdpStreamWriter() {
    }

    ~GUdpStreamWriter() {
        close();
    }

    // WARNING: copy constructor not allowed
    GUdpStreamWriter(const GUdpStreamWriter& other) = delete;

    // WARNING: assignment operator not allowed
    GUdpStreamWriter& operator=(const GUdpStreamWriter& other) = delete;

    bool open(const std::string& addr, uint16_t port, size_t length = MTU_DATAGRAM_SIZE) {
        close();

        m_length = length > 0 ? length : MAX_DATAGRAM_SIZE;
        m_buffer = new (std::nothrow) char[m_length];

        struct addrinfo  hints;
        struct addrinfo* res;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        m_addr = addr;
        m_port = std::to_string(port);

        auto error_code{getaddrinfo(m_addr.c_str(), m_port.c_str(), &hints, &res)};
        if (error_code != 0) {
            m_last_error = gai_strerror(error_code);
            goto free_and_exit;
        }

        m_socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (m_socket_fd == -1) {
            m_last_error = gai_strerror(error_code);
            goto free_and_exit;
        }

        if (connect(m_socket_fd, res->ai_addr, res->ai_addrlen) == -1) {
            m_last_error = gai_strerror(error_code);
            goto free_and_exit;
        }

        reset();
        m_is_open = true;

free_and_exit:
        freeaddrinfo(res);
        return m_is_open;
    }

    void close() {
        if (m_is_open) {
            flush();
            m_is_open = false;
        }

        if (m_buffer != nullptr) {
            delete[] m_buffer;
            m_buffer = nullptr;
        }
    }

    GUdpStreamWriter& operator<<(char_type __c) {
        base::sputc(__c);
        return *this;
    }

    GUdpStreamWriter& operator<<(const char_type* __s) {
        auto __n = (std::streamsize)strlen(__s);
        base::xsputn(__s, __n);
        return *this;
    }

    GUdpStreamWriter& reset() {
        base::setp(m_buffer, m_buffer + m_length);
        return *this;
    }

    GUdpStreamWriter& write(const char_type* __s, std::streamsize __n) {
        base::xsputn(__s, __n);
        return *this;
    }

    GUdpStreamWriter& flush() {
        if (m_is_open) {
            auto bytes = (size_t)(pptr() - pbase());
            send(m_socket_fd, m_buffer, bytes, 0);
        }
        reset();
        return *this;
    }

    GUdpStreamWriter& put(char_type __c) {
        base::sputc(__c);
        return *this;
    }

    GUdpStreamWriter& endl() {
        base::sputc('\n');
        return flush();
    }

    auto is_open() {
        return m_is_open;
    }

    auto last_error() {
        return m_last_error.c_str();
    }

    private:
    // SECTION: stream
    size_t m_length{0};
    char*  m_buffer{nullptr};

    // SECTION: socket
    std::string m_addr;
    std::string m_port;
    bool        m_is_open{false};
    int         m_socket_fd{-1};
    std::string m_last_error;
};

#endif // GUDPSTREAMWRITER_HPP