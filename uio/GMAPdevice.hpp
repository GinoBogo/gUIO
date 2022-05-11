////////////////////////////////////////////////////////////////////////////////
/// \file      GMAPdevice.hpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GMAPDEVICE_HPP
#define GMAPDEVICE_HPP

#include <cstddef> // size_t
#include <cstdint> // uint32_t
#include <list>    // std::list

struct map_device_t {
    // SECTION: node properties
    int fd;
    // SECTION: device properties
    size_t addr;
    size_t size;
    // SECTION: memory fields
    void* mmap_addr;
    void* virt_addr;
};

class GMAPdevice {
    public:
    typedef struct {
        uint32_t offset;
        uint32_t value;

    } reg_pair_t;

    typedef std::list<reg_pair_t> reg_list_t;

    GMAPdevice(size_t addr, size_t size);
    ~GMAPdevice();

    bool Open();
    void Close();
    bool MapToMemory();

    auto Read(reg_list_t& list) {
        if (list.size() > 0) {
            auto _virt_addr{static_cast<uint32_t*>(m_dev.virt_addr)};
            for (auto it{list.begin()}; it != list.end(); ++it) {
                if (it->offset <= m_dev.size / sizeof(uint32_t)) {
                    it->value = _virt_addr[it->offset];
                }
            }
            return true;
        }
        return false;
    }

    auto Write(reg_list_t& list) {
        if (list.size() > 0) {
            auto _virt_addr{static_cast<uint32_t*>(m_dev.virt_addr)};
            for (auto it{list.cbegin()}; it != list.cend(); ++it) {
                if (it->offset <= m_dev.size / sizeof(uint32_t)) {
                    _virt_addr[it->offset] = it->value;
                }
            }
            return true;
        }
        return false;
    }

    template <typename T> auto Read(size_t offset, T* dst_buf, size_t words = 1) {
        auto _t1{dst_buf != nullptr && words > 0};
        auto _t2{offset + words - 1 <= m_dev.size / sizeof(T)};

        if (_t1 && _t2) {
            auto _virt_addr{static_cast<T*>(m_dev.virt_addr)};
            if (words == 1)
                *dst_buf = _virt_addr[offset];
            else
                for (decltype(words) i{0}; i < words; ++i) dst_buf[i] = _virt_addr[offset + i];
            return true;
        }
        return false;
    }

    template <typename T> auto Write(size_t offset, T* src_buf, size_t words = 1) {
        auto _t1{src_buf != nullptr && words > 0};
        auto _t2{offset + words - 1 <= m_dev.size / sizeof(T)};

        if (_t1 && _t2) {
            auto _virt_addr{static_cast<T*>(m_dev.virt_addr)};
            if (words == 1)
                _virt_addr[offset] = *src_buf;
            else
                for (decltype(words) i{0}; i < words; ++i) _virt_addr[offset + i] = src_buf[i];
            return true;
        }
        return false;
    }

    template <typename T, typename R = uint32_t> auto OverRead(size_t offset, T* dst_buf, size_t words = 1) {
        auto _t1{dst_buf != nullptr && words > 0};

        if (_t1) {
            volatile auto _virt_addr{static_cast<R*>(m_dev.virt_addr)};
            if (words == 1)
                *dst_buf = static_cast<T>(_virt_addr[offset]);
            else
                for (decltype(words) i{0}; i < words;) dst_buf[i++] = static_cast<T>(_virt_addr[offset]);
            return true;
        }
        return false;
    }

    template <typename T, typename R = uint32_t> auto OverWrite(size_t offset, T* src_buf, size_t words = 1) {
        auto _t1{src_buf != nullptr && words > 0};

        if (_t1) {
            volatile auto _virt_addr{static_cast<R*>(m_dev.virt_addr)};
            if (words == 1)
                _virt_addr[offset] = static_cast<R>(*src_buf);
            else
                for (decltype(words) i{0}; i < words;) _virt_addr[offset] = static_cast<R>(src_buf[i++]);
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

#endif // GMAPDEVICE_HPP
