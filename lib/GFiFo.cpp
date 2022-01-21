////////////////////////////////////////////////////////////////////////////////
/// \file      GFiFo.cpp
/// \version   0.1
/// \date      May, 2016
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GFiFo.hpp"

GFiFo::GFiFo(const uint32_t item_size, const uint32_t fifo_depth) {
    m_size  = item_size;
    m_depth = fifo_depth;
    m_count = 0;
    m_iW    = 0;
    m_iR    = 0;
    p_fifo  = nullptr;

    if (m_size && m_depth) {
        p_fifo = new GBuffer *[m_depth];

        for (decltype(m_depth) i{0}; i < m_depth; i++) {
            p_fifo[i] = new GBuffer(m_size);
        }
    }
}

GFiFo::~GFiFo() {
    for (decltype(m_depth) i{0}; i < m_depth; i++) {
        delete p_fifo[i];
    }

    delete[] p_fifo;
}

void GFiFo::Reset() {
    const std::lock_guard<std::mutex> lock(m_mutex);

    uint32_t i = 0, N = m_depth;

    while (N--) {
        p_fifo[i++]->Reset();
    }

    m_count = 0;
    m_iW    = 0;
    m_iR    = 0;
}

void GFiFo::Clear() {
    const std::lock_guard<std::mutex> lock(m_mutex);

    uint32_t i = 0, N = m_depth;

    while (N--) {
        p_fifo[i++]->Clear();
    }

    m_count = 0;
    m_iW    = 0;
    m_iR    = 0;
}

void GFiFo::SmartClear() {
    const std::lock_guard<std::mutex> lock(m_mutex);

    uint32_t i = 0, N = m_depth;

    while (N--) {
        p_fifo[i++]->SmartClear();
    }

    m_count = 0;
    m_iW    = 0;
    m_iR    = 0;
}

bool GFiFo::Push(const GBuffer *src_buff) {
    if (src_buff) {
        const std::lock_guard<std::mutex> lock(m_mutex);

        if (!IsFull()) {
            GBuffer *_item = p_fifo[m_iW];

            _item->Reset();

            if (_item->Append(src_buff->data(), src_buff->count())) {
                ++m_iW;
                ++m_count;

                if (m_iW == m_depth) {
                    m_iW = 0;
                }

                return true;
            }
        }
    }

    return false;
}

bool GFiFo::Push(const uint8_t *src_data, const uint32_t src_count) {
    if (src_data && src_count) {
        const std::lock_guard<std::mutex> lock(m_mutex);

        if (!IsFull()) {
            GBuffer *_item = p_fifo[m_iW];

            _item->Reset();

            if (_item->Append(src_data, src_count)) {
                ++m_iW;
                ++m_count;

                if (m_iW == m_depth) {
                    m_iW = 0;
                }

                return true;
            }
        }
    }

    return false;
}

bool GFiFo::Pop(GBuffer *dst_buff) {
    if (dst_buff) {
        const std::lock_guard<std::mutex> lock(m_mutex);

        if (!IsEmpty()) {
            GBuffer *_item = p_fifo[m_iR];

            dst_buff->Reset();

            if (dst_buff->Append(_item->data(), _item->count())) {
                ++m_iR;
                --m_count;

                if (m_iR == m_depth) {
                    m_iR = 0;
                }

                return true;
            }
        }
    }

    return false;
}

int32_t GFiFo::Pop(uint8_t *dst_data, const uint32_t dst_size) {
    if (dst_data && dst_size) {
        const std::lock_guard<std::mutex> lock(m_mutex);

        if (!IsEmpty()) {
            GBuffer  *_item = p_fifo[m_iR];
            uint32_t bytes = _item->count();

            if (dst_size >= bytes) {
                memcpy((void *)dst_data, (void *)_item->data(), bytes);

                ++m_iR;
                --m_count;

                if (m_iR == m_depth) {
                    m_iR = 0;
                }

                return (int32_t)bytes;
            }
        }
    }

    return -1;
}
