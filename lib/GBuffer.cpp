
////////////////////////////////////////////////////////////////////////////////
/// \file      GBuffer.cpp
/// \version   0.1
/// \date      May, 2016
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GBuffer.hpp"

GBuffer::GBuffer(const uint32_t size) {
    m_size       = size;
    m_is_wrapper = m_size == 0;

    if (!m_is_wrapper) {
        p_data     = new uint8_t[m_size];
        m_is_ready = true;
        Reset();
    }
}

GBuffer::GBuffer(const GBuffer& buffer) {
    *this = buffer;
}

GBuffer::~GBuffer() {
    release_resources();
}

GBuffer& GBuffer::operator=(const GBuffer& buffer) {
    if (this != &buffer) {
        release_resources();

        m_size       = buffer.size();
        m_is_wrapper = m_size == 0;

        if (!m_is_wrapper) {
            p_data     = new uint8_t[m_size];
            m_is_ready = true;
            Reset();
            Append(buffer.data(), buffer.count());
        }
    }
    return *this;
}

bool GBuffer::Wrap(uint8_t* buf_data, const uint32_t buf_size) {
    if (m_is_wrapper && !m_is_ready) {
        if ((buf_data != nullptr) && (buf_size > 0)) {
            m_size     = buf_size;
            p_data     = buf_data;
            m_is_ready = true;
            Reset();
        }
        return m_is_ready;
    }
    return false;
}

bool GBuffer::Append(const uint8_t* src_data, const uint32_t src_count) {
    if (!m_is_ready || (src_data == nullptr) || (src_count == 0) || free() < src_count) {
        return false;
    }

    memcpy(p_next, src_data, src_count);

    m_count += src_count;
    p_next += src_count;

    return true;
}

void GBuffer::SetCount(const uint32_t value) {
    if (!m_is_ready) {
        return;
    }

    if (value >= m_size) {
        m_count = m_size;
        p_next  = nullptr;
    }
    else {
        m_count = value;
        p_next  = p_data + value;
    }
}

void GBuffer::Increase(const uint32_t delta) {
    if (!m_is_ready) {
        return;
    }

    if (delta >= free()) {
        m_count = m_size;
        p_next  = nullptr;
    }
    else {
        m_count += delta;
        p_next += delta;
    }
}
