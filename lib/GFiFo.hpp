////////////////////////////////////////////////////////////////////////////////
/// \file      GFiFo.hpp
/// \version   0.1
/// \date      May, 2016
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GFIFO_HPP
#define GFIFO_HPP

#include "GBuffer.hpp"

#include <mutex>

class GFiFo {
    public:
    GFiFo(const uint32_t item_size, const uint32_t fifo_depth);

    ~GFiFo();

    void Reset();

    void Clear();

    void SmartClear();

    bool Push(const GBuffer *src_buff);

    bool Push(const uint8_t *src_data, const uint32_t src_count);

    bool Pop(GBuffer *dst_buff);

    int32_t Pop(uint8_t *dst_data, const uint32_t dst_size);

    inline bool IsEmpty() {
        return !m_count;
    }

    inline bool IsFull() {
        return m_count == m_depth;
    }

    inline uint32_t size() const {
        return m_size;
    }

    inline uint32_t depth() const {
        return m_depth;
    }

    inline uint32_t count() const {
        return m_count;
    }

    inline uint32_t free() const {
        return m_depth - m_count;
    }

    private:
    uint32_t   m_size;
    uint32_t   m_depth;
    uint32_t   m_count;
    uint32_t   m_iR;
    uint32_t   m_iW;
    GBuffer   **p_fifo;
    std::mutex m_mutex;
};

#endif // GFIFO_HPP
