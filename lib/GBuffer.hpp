////////////////////////////////////////////////////////////////////////////////
/// \file      GBuffer.hpp
/// \version   0.1
/// \date      May, 2016
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GBUFFER_HPP_
#define GBUFFER_HPP_

#include <cstdint> // uint8_t, uint32_t
#include <cstring> // memset

class GBuffer {
    public:
    GBuffer(const uint32_t buf_size = 0);

    ~GBuffer();

    inline void Reset();

    inline void Clear();

    inline void SmartClear();

    bool Wrap(uint8_t *buf_data, const uint32_t buf_size);

    bool Append(const uint8_t *src_data, const uint32_t src_count);

    void SetCount(const uint32_t value);

    void Increase(const uint32_t delta);

    inline bool IsWrapper() const {
        return m_is_wrapper;
    }

    inline bool IsEmpty() const {
        return !m_count;
    }

    inline bool IsFull() const {
        return !(m_size - m_count);
    }

    inline uint32_t size() const {
        return m_size;
    }

    inline uint32_t count() const {
        return m_count;
    }

    inline uint32_t free() const {
        return m_size - m_count;
    }

    inline uint8_t *data() const {
        return p_data;
    }

    inline uint8_t *next() const {
        return p_next;
    }

    private:
    bool     m_is_wrapper;
    uint32_t m_size;
    uint32_t m_count;
    uint8_t *p_data;
    uint8_t *p_next;
};

inline void GBuffer::Reset() {
    m_count = 0;
    p_next  = p_data;
}

inline void GBuffer::Clear() {
    memset(p_data, 0, m_size);
    Reset();
}

inline void GBuffer::SmartClear() {
    if (m_count) {
        memset(p_data, 0, m_count);
        Reset();
    }
}

#endif /* GBUFFER_HPP_ */
