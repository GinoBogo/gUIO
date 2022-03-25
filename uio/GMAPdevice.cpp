////////////////////////////////////////////////////////////////////////////////
/// \file      GMAPdevice.cpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GMAPdevice.hpp"

#include "GLogger.hpp"

#include <errno.h>    // errno
#include <fcntl.h>    // open
#include <stdio.h>    // snprintf
#include <string.h>   // memset
#include <sys/mman.h> // mmap, munmap
#include <unistd.h>   // close, read, write

auto map_device_reset = [](map_device_t* dev, bool clear_all) {
    auto addr = dev->addr;
    auto size = dev->size;

    memset(dev, 0, sizeof(map_device_t));
    dev->fd        = -1;
    dev->mmap_addr = MAP_FAILED;
    dev->virt_addr = MAP_FAILED;

    if (!clear_all) {
        dev->addr = addr;
        dev->size = size;
    }
};

GMAPdevice::GMAPdevice(size_t addr, size_t size) {
    map_device_reset(&m_dev, true);
    m_dev.addr = addr;
    m_dev.size = size;
    if (!size) {
        LOG_FORMAT(error, "Wrong block size for the 0x%08x base address", addr);
    }
}

GMAPdevice::~GMAPdevice() {
    Close();
}

bool GMAPdevice::Open() {
    m_dev.fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (m_dev.fd == -1) {
        LOG_FORMAT(error, "Cannot open the \"/dev/mem\" device [E%d]", errno);
        return false;
    }
    return true;
}

void GMAPdevice::Close() {
    if (m_dev.fd != -1) {
        close(m_dev.fd);
    }

    if (m_dev.mmap_addr != MAP_FAILED) {
        if (munmap(m_dev.mmap_addr, m_dev.size) == -1) {
            LOG_FORMAT(error, "Cannot unmap the 0x%08x address from user space [E%d]", m_dev.addr, errno);
        }
    }

    map_device_reset(&m_dev, false);
}

bool GMAPdevice::MapToMemory() {
    const size_t page_size   = (size_t)sysconf(_SC_PAGESIZE);
    const size_t page_mask   = page_size - 1;
    const size_t mmap_offset = m_dev.addr & ~page_mask;
    const size_t virt_offset = m_dev.addr & page_mask;

    //   page_size: 0x00001000 (4096)
    //   page_mask: 0x00000FFF
    //  ~page_mask: 0xFFFFF000
    //    dev_addr: 0xFFFFAE54
    // mmap_offset: 0xFFFFA000
    // virt_offset: 0x00000E54

    m_dev.mmap_addr = mmap(nullptr, m_dev.size, PROT_READ | PROT_WRITE, MAP_SHARED, m_dev.fd, (off_t)mmap_offset);
    if (m_dev.mmap_addr == MAP_FAILED) {
        LOG_FORMAT(error, "Cannot map the 0x%08x address to user space [E%d]", m_dev.addr, errno);
        return false;
    }

    m_dev.virt_addr = static_cast<char*>(m_dev.mmap_addr) + virt_offset;
    return true;
}
