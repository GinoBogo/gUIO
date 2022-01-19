////////////////////////////////////////////////////////////////////////////////
/// \file      Buffer.cpp
/// \version   0.1
/// \date      May, 2016
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "Buffer.hpp"

Buffer::Buffer(const uint32_t buf_size) {
    m_is_wrapper = true;
    m_size       = buf_size;

    if (buf_size) {
        m_is_wrapper = false;
        p_data       = new uint8_t[buf_size];
        Reset();
    }
}

Buffer::~Buffer() {
    if (!m_is_wrapper) {
        delete[] p_data;
    }
}

bool Buffer::Wrap(uint8_t *buf_data, const uint32_t buf_size) {
    if (m_is_wrapper) {
        if (buf_data && buf_size) {
            m_size = buf_size;
            p_data = buf_data;
            Reset();
            return true;
        }
    }
    return false;
}

bool Buffer::Append(const uint8_t *src_data, const uint32_t src_count) {
    if (!src_data || !src_count || free() < src_count) {
        return false;
    }

    memcpy(p_next, src_data, src_count);

    m_count += src_count;
    p_next += src_count;

    return true;
}

void Buffer::SetCount(const uint32_t value) {
    if (value >= m_size) {
        m_count = m_size;
        p_next  = nullptr;
    }
    else {
        m_count = value;
        p_next  = p_data + value;
    }
}

void Buffer::Increase(const uint32_t delta) {
    if (delta >= free()) {
        m_count = m_size;
        p_next  = nullptr;
    }
    else {
        m_count += delta;
        p_next += delta;
    }
}
