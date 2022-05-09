////////////////////////////////////////////////////////////////////////////////
/// \file      GBuffer.hpp
/// \version   0.1
/// \date      May, 2016
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GBUFFER_HPP
#define GBUFFER_HPP

#include <cstdint> // uint8_t, uint32_t
#include <cstring> // memset

class GBuffer {
    public:
    GBuffer(const uint32_t max_size = 0);

    GBuffer(const GBuffer& buffer);

    ~GBuffer();

    GBuffer& operator=(const GBuffer& buffer);

    bool Wrap(uint8_t* buf_data, const uint32_t buf_size);

    bool Append(const uint8_t* src_data, const uint32_t src_count);

    void SetCount(const uint32_t value);

    void Increase(const uint32_t delta);

    inline void Reset() {
        m_count = 0;
        p_next  = p_data;
    }

    inline void Clear() {
        memset(p_data, 0, m_size);
        m_count = 0;
        p_next  = p_data;
    }

    inline void SmartClear() {
        if (m_count) {
            memset(p_data, 0, m_count);
            m_count = 0;
            p_next  = p_data;
        }
    }

    inline auto IsReady() {
        return m_is_ready;
    }

    inline auto IsWrapper() const {
        return m_is_wrapper;
    }

    inline auto IsEmpty() const {
        return !m_count;
    }

    inline auto IsFull() const {
        return !(m_size - m_count);
    }

    inline auto size() const {
        return m_size;
    }

    inline auto count() const {
        return m_count;
    }

    inline auto free() const {
        return m_size - m_count;
    }

    inline auto data() const {
        return p_data;
    }

    inline auto next() const {
        return p_next;
    }

    private:
    bool     m_is_ready;
    bool     m_is_wrapper;
    uint32_t m_size;
    uint32_t m_count;
    uint8_t* p_data;
    uint8_t* p_next;
};

#endif // GBUFFER_HPP
