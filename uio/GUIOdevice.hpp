
////////////////////////////////////////////////////////////////////////////////
/// \file      GUIOdevice.hpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GUIODEVICE_HPP
#define GUIODEVICE_HPP

#include <cstddef>  // size_t
#include <cstdint>  // uint8_t, int32_t
#include <poll.h>   // poll
#include <unistd.h> // close, read, write

const auto uio_device_t_name_maxlen = 64;

struct uio_device_t {
    // SECTION: node properties
    int fd;
    int uio_num;
    int map_num;
    // SECTION: device attributes
    char   name[uio_device_t_name_maxlen];
    size_t addr;
    size_t offset;
    size_t size;
    // SECTION: memory & interrupt
    void*   mmap_addr;
    void*   virt_addr;
    int32_t irq_count;
};

class GUIOdevice {
    public:
    GUIOdevice(int uio_num, int map_num);
    ~GUIOdevice();

    bool Open();
    void Close();
    bool MapToMemory();

    bool IRQ_Wait(int timeout = -1, bool* is_timeout_expired = nullptr) {
        struct pollfd fds;
        fds.fd     = m_dev.fd;
        fds.events = POLLIN;

        auto _res{poll(&fds, 1, timeout)};
        if (_res == 0) {
            if (is_timeout_expired != nullptr) {
                *is_timeout_expired = true;
            }
            return false;
        }

        if (_res > 0) {
            // INFO: The only valid read() argument is a signed 32-bit integer.
            auto _ret{read(m_dev.fd, &m_dev.irq_count, sizeof(m_dev.irq_count))};
            return _ret != -1;
        }
        return false;
    }

    bool IRQ_Clear() const {
        int32_t _val{0x00000001};

        auto _ret{write(m_dev.fd, &_val, sizeof(_val))};
        return _ret != -1;
    }

    size_t GetMapAttribute(const char* attr_name, bool* error = nullptr, char* dst_buf = nullptr) const;
    void   PrintMapAttributes() const;

    [[nodiscard]] auto uio_num() const {
        return m_dev.uio_num;
    }

    [[nodiscard]] auto map_num() const {
        return m_dev.map_num;
    }

    [[nodiscard]] auto name() const {
        return (const char*)m_dev.name;
    }

    [[nodiscard]] auto addr() const {
        return m_dev.addr;
    }

    [[nodiscard]] auto offset() const {
        return m_dev.offset;
    }

    [[nodiscard]] auto size() const {
        return m_dev.size;
    }

    [[nodiscard]] auto high_addr() const {
        return m_dev.addr + (m_dev.size - 1);
    }

    [[nodiscard]] auto* virt_addr() const {
        return (uint8_t*)m_dev.virt_addr;
    }

    [[nodiscard]] auto irq_count() const {
        return m_dev.irq_count;
    }

    private:
    uio_device_t m_dev;
};

#endif // GUIODEVICE_HPP
