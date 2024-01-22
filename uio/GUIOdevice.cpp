
////////////////////////////////////////////////////////////////////////////////
/// \file      GUIOdevice.cpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GUIOdevice.hpp"

#include "../lib/GLogger.hpp"

#include <cerrno>     // errno
#include <cstdlib>    // strtoul
#include <cstring>    // strncpy
#include <fcntl.h>    // open, ioctl
#include <strings.h>  // memset
#include <sys/mman.h> // mmap, munmap

auto uio_device_reset = [](uio_device_t* dev, bool clear_all) {
    auto uio_num = dev->uio_num;
    auto map_num = dev->map_num;

    memset(dev, 0, sizeof(uio_device_t));
    dev->fd        = -1;
    dev->mmap_addr = MAP_FAILED;
    dev->virt_addr = MAP_FAILED;

    if (!clear_all) {
        dev->uio_num = uio_num;
        dev->map_num = map_num;
    }
};

GUIOdevice::GUIOdevice(int uio_num, int map_num) {
    uio_device_reset(&m_dev, true);
    m_dev.uio_num = uio_num;
    m_dev.map_num = map_num;
}

GUIOdevice::~GUIOdevice() {
    Close();
}

bool GUIOdevice::Open() {
    char _buf[32];
    snprintf(_buf, sizeof(_buf), "/dev/uio%d", m_dev.uio_num);

    int _fd{open(_buf, O_RDWR | O_SYNC)};
    if (_fd == -1) {
        LOG_FORMAT(error, "Cannot open the \"uio%d\" device [E%d]", m_dev.uio_num, errno);
        return false;
    }

    bool error;
    GetMapAttribute("name", &error, m_dev.name);
    m_dev.fd     = _fd;
    m_dev.addr   = GetMapAttribute("addr", &error);
    m_dev.offset = GetMapAttribute("offset", &error);
    m_dev.size   = GetMapAttribute("size", &error);
    return !error;
}

void GUIOdevice::Close() {
    if (m_dev.fd > 0) {
        IRQ_Clear();
        close(m_dev.fd);
    }

    if (m_dev.mmap_addr != MAP_FAILED) {
        munmap(m_dev.mmap_addr, m_dev.size);
    }

    uio_device_reset(&m_dev, false);
}

bool GUIOdevice::MapToMemory() {
    auto page_size   = sysconf(_SC_PAGESIZE);
    auto page_offset = page_size * m_dev.map_num;

    m_dev.mmap_addr = mmap(nullptr, m_dev.size, PROT_READ | PROT_WRITE, MAP_SHARED, m_dev.fd, page_offset);
    if (m_dev.mmap_addr == MAP_FAILED) {
        LOG_FORMAT(error, "Cannot map the \"uio%d\" device to user space [E%d]", m_dev.uio_num, errno);
        return false;
    }

    m_dev.virt_addr = static_cast<uint8_t*>(m_dev.mmap_addr) + m_dev.offset;
    return true;
}

size_t GUIOdevice::GetMapAttribute(const char* attr_name, bool* error, char* dst_buff) const {
    char _buf[64];
    snprintf(_buf, sizeof(_buf), "/sys/class/uio/uio%d/maps/map%d/%s", m_dev.uio_num, m_dev.map_num, attr_name);

    int _fd{open(_buf, O_RDONLY)};
    if (_fd < 0) {
        LOG_FORMAT(error, "Cannot open the \"uio%d/map%d/%s\" attribute [E%d]", m_dev.uio_num, m_dev.map_num, attr_name, errno);
        if (error != nullptr) {
            *error = true;
        }
        return 0;
    }

    auto bytes{read(_fd, _buf, sizeof(_buf))};
    close(_fd);

    if (bytes < 0) {
        LOG_FORMAT(error, "Cannot read the \"uio%d/map%d/%s\" attribute [E%d]", m_dev.uio_num, m_dev.map_num, attr_name, errno);
        if (error != nullptr) {
            *error = true;
        }
        return 0;
    }

    if (bytes > 0) {
        _buf[bytes - 1] = 0;
        if (dst_buff == nullptr) {
            auto _val{strtoul(_buf, nullptr, 16)};
            if (error != nullptr) {
                *error = false;
            }
            return _val;
        }
        strncpy(dst_buff, _buf, (size_t)bytes - 1);
        return 0;
    }

    LOG_FORMAT(warning, "The \"uio%d/map%d/%s\" attribute is empty", m_dev.uio_num, m_dev.map_num, attr_name);
    if (error != nullptr) {
        *error = true;
    }
    return 0;
}

void GUIOdevice::PrintMapAttributes() const {
    char _file[32];
    snprintf(_file, sizeof(_file), "uio%d/maps/map%d", uio_num(), map_num());

    LOG_FORMAT(info, "UIO Map Attributes <FILE> <VALUE> <LABEL> of \"%s\" device:", name());
    LOG_FORMAT(info, "  %s | 0x%012lx | %s", _file, addr(), "device base address");
    LOG_FORMAT(info, "  %s | 0x%012lx | %s", _file, offset(), "device base offset");
    LOG_FORMAT(info, "  %s | 0x%012lx | %s", _file, size(), "device block size");
    LOG_FORMAT(info, "  %s | 0x%012lx | %s", _file, high_addr(), "device high address");
    LOG_FORMAT(info, "  %s | 0x%012lx | %s", _file, virt_addr(), "virtual base address");
}
