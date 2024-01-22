
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
    GBuffer(uint32_t size = 0);

    GBuffer(const GBuffer& buffer) = delete;

    ~GBuffer();

    GBuffer& operator=(const GBuffer& buffer) = delete;

    bool Wrap(uint8_t* buf_data, uint32_t buf_size);

    bool Append(const uint8_t* src_data, uint32_t src_used);

    void SetCount(uint32_t value);

    void Increase(uint32_t delta);

    auto Reset() {
        m_used = 0;
        p_next = p_data;
    }

    auto Clear() {
        memset(p_data, 0, m_size);
        m_used = 0;
        p_next = p_data;
    }

    auto SmartClear() {
        if (m_used > 0) {
            memset(p_data, 0, m_used);
            m_used = 0;
            p_next = p_data;
        }
    }

    [[nodiscard]] auto IsReady() const {
        return m_is_ready;
    }

    [[nodiscard]] auto IsWrapper() const {
        return m_is_wrapper;
    }

    [[nodiscard]] auto IsEmpty() const {
        return (m_used == 0);
    }

    [[nodiscard]] auto IsFull() const {
        return ((m_size - m_used) == 0);
    }

    [[nodiscard]] auto size() const {
        return m_size;
    }

    [[nodiscard]] auto used() const {
        return m_used;
    }

    [[nodiscard]] auto free() const {
        return m_size - m_used;
    }

    [[nodiscard]] auto data() const {
        return p_data;
    }

    [[nodiscard]] auto next() const {
        return p_next;
    }

  private:
    bool     m_is_ready{false};
    bool     m_is_wrapper;
    uint32_t m_size;
    uint32_t m_used;
    uint8_t* p_data;
    uint8_t* p_next;
};

#endif // GBUFFER_HPP
