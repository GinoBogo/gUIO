////////////////////////////////////////////////////////////////////////////////
/// \file      GMAPdevice.hpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GMAPDEVICE_HPP_
#define GMAPDEVICE_HPP_

#include <cstddef> // size_t

struct map_device_t {
    // SECTION: node properties
    int fd;
    // SECTION: device properties
    size_t addr;
    size_t size;
    // SECTION: memory fields
    void *mmap_addr;
    void *virt_addr;
};

class GMAPdevice {
    public:
    GMAPdevice(size_t addr, size_t size);
    ~GMAPdevice();

    bool Open();
    void Close();
    bool MapToMemory();

    template <typename T> auto Read(size_t offset, T *dst_buf, size_t words = 1) {
        auto is_valid = [&]() {
            auto _t1{dst_buf != nullptr && words > 0};
            auto _t2{offset + words - 1 <= m_dev.size / sizeof(T)};
            return _t1 && _t2;
        };

        if (is_valid()) {
            auto virt_addr{static_cast<T *>(m_dev.virt_addr)};
            if (words == 1)
                *dst_buf = virt_addr[offset];
            else
                for (decltype(words) i{0}; i < words; ++i) dst_buf[i] = virt_addr[offset + i];
            return true;
        }
        return false;
    }

    template <typename T> auto Write(size_t offset, T *src_buf, size_t words = 1) {
        auto is_valid = [&]() {
            auto _t1{src_buf != nullptr && words > 0};
            auto _t2{offset + words - 1 <= m_dev.size / sizeof(T)};
            return _t1 && _t2;
        };

        if (is_valid()) {
            auto virt_addr{static_cast<T *>(m_dev.virt_addr)};
            if (words == 1)
                virt_addr[offset] = *src_buf;
            else
                for (decltype(words) i{0}; i < words; ++i) virt_addr[offset + i] = src_buf[i];
            return true;
        }
        return false;
    }

    auto virt_addr() {
        return m_dev.virt_addr;
    }

    private:
    map_device_t m_dev;
};

#endif /* GMAPDEVICE_HPP_ */
