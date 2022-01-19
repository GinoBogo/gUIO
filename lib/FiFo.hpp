////////////////////////////////////////////////////////////////////////////////
/// \file      FiFo.hpp
/// \version   0.1
/// \date      May, 2016
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef FIFO_HPP_
#define FIFO_HPP_

#include "Buffer.hpp"

#include <pthread.h>

class FiFo {
    public:
    FiFo(const uint32_t item_size, const uint32_t fifo_depth);

    ~FiFo();

    void Reset();

    void Clear();

    void SmartClear();

    bool Push(const Buffer *src_buff);

    bool Push(const uint8_t *src_data, const uint32_t src_count);

    bool Pop(Buffer *dst_buff);

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
    uint32_t        m_size;
    uint32_t        m_depth;
    uint32_t        m_count;
    uint32_t        m_iR;
    uint32_t        m_iW;
    Buffer        **p_fifo;
    pthread_mutex_t m_mutex;
};

#endif /* FIFO_HPP_ */
