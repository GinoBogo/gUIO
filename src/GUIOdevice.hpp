////////////////////////////////////////////////////////////////////////////////
/// \file      GUIOdevice.hpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GUIODEVICE_HPP
#define GUIODEVICE_HPP

#include <cstddef> // size_t
#include <cstdint> // uint8_t, int32_t

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
    void *  mmap_addr;
    void *  virt_addr;
    int32_t irq_count;
};

class GUIOdevice {
    public:
    GUIOdevice(int uio_num, int map_num);
    ~GUIOdevice();

    bool Open();
    void Close();
    bool MapToMemory();

    bool IRQ_Wait(int timeout = -1);
    bool IRQ_Clear();

    size_t GetMapAttribute(const char *attr_name, bool *error = nullptr, char *dst_buf = nullptr);
    void   PrintMapAttributes();

    auto uio_num() const {
        return m_dev.uio_num;
    }

    auto map_num() const {
        return m_dev.map_num;
    }

    auto name() const {
        return (const char *)m_dev.name;
    }

    auto addr() const {
        return m_dev.addr;
    }

    auto offset() const {
        return m_dev.offset;
    }

    auto size() const {
        return m_dev.size;
    }

    auto high_addr() const {
        return m_dev.addr + (m_dev.size - 1);
    }

    auto *virt_addr() const {
        return (uint8_t *)m_dev.virt_addr;
    }

    auto irq_count() const {
        return m_dev.irq_count;
    }

    private:
    uio_device_t m_dev;
};

#endif // GUIODEVICE_HPP
