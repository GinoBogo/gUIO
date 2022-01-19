////////////////////////////////////////////////////////////////////////////////
/// \file      FiFo.cpp
/// \version   0.1
/// \date      May, 2016
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "FiFo.hpp"

FiFo::FiFo(const uint32_t item_size, const uint32_t fifo_depth) {
    m_size  = item_size;
    m_depth = fifo_depth;
    m_count = 0;
    m_iW    = 0;
    m_iR    = 0;
    m_item  = nullptr;

    if (m_size && m_depth) {
        m_item = new Buffer *[m_depth];

        for (uint32_t i = 0; i < m_depth; i++) {
            m_item[i] = new Buffer(m_size);
        }
    }

    pthread_mutex_init(&m_mutex, nullptr);
}

FiFo::~FiFo() {
    for (uint32_t i = 0; i < m_depth; i++) {
        delete m_item[i];
    }

    delete[] m_item;
}

void FiFo::Reset() {
    pthread_mutex_lock(&m_mutex);

    uint32_t i = 0, N = m_depth;

    while (N--) {
        m_item[i++]->Reset();
    }

    m_count = 0;
    m_iW    = 0;
    m_iR    = 0;

    pthread_mutex_unlock(&m_mutex);
}

void FiFo::Clear() {
    pthread_mutex_lock(&m_mutex);

    uint32_t i = 0, N = m_depth;

    while (N--) {
        m_item[i++]->Clear();
    }

    m_count = 0;
    m_iW    = 0;
    m_iR    = 0;

    pthread_mutex_unlock(&m_mutex);
}

void FiFo::SmartClear() {
    pthread_mutex_lock(&m_mutex);

    uint32_t i = 0, N = m_depth;

    while (N--) {
        m_item[i++]->SmartClear();
    }

    m_count = 0;
    m_iW    = 0;
    m_iR    = 0;

    pthread_mutex_unlock(&m_mutex);
}

bool FiFo::Push(const Buffer *src_buff) {
    if (src_buff) {
        pthread_mutex_lock(&m_mutex);

        if (!IsFull()) {
            Buffer *_item = m_item[m_iW];

            _item->Reset();

            if (_item->Append(src_buff->data(), src_buff->count())) {
                ++m_iW;
                ++m_count;

                if (m_iW == m_depth) {
                    m_iW = 0;
                }

                pthread_mutex_unlock(&m_mutex);

                return true;
            }
        }

        pthread_mutex_unlock(&m_mutex);
    }

    return false;
}

bool FiFo::Push(const uint8_t *src_data, const uint32_t src_count) {
    if (src_data && src_count) {
        pthread_mutex_lock(&m_mutex);

        if (!IsFull()) {
            Buffer *_item = m_item[m_iW];

            _item->Reset();

            if (_item->Append(src_data, src_count)) {
                ++m_iW;
                ++m_count;

                if (m_iW == m_depth) {
                    m_iW = 0;
                }

                pthread_mutex_unlock(&m_mutex);

                return true;
            }
        }

        pthread_mutex_unlock(&m_mutex);
    }

    return false;
}

bool FiFo::Pop(Buffer *dst_buff) {
    if (dst_buff) {
        pthread_mutex_lock(&m_mutex);

        if (!IsEmpty()) {
            Buffer *_item = m_item[m_iR];

            dst_buff->Reset();

            if (dst_buff->Append(_item->data(), _item->count())) {
                ++m_iR;
                --m_count;

                if (m_iR == m_depth) {
                    m_iR = 0;
                }

                pthread_mutex_unlock(&m_mutex);

                return true;
            }
        }

        pthread_mutex_unlock(&m_mutex);
    }

    return false;
}

int32_t FiFo::Pop(uint8_t *dst_data, const uint32_t dst_size) {
    if (dst_data && dst_size) {
        pthread_mutex_lock(&m_mutex);

        if (!IsEmpty()) {
            Buffer  *_item = m_item[m_iR];
            uint32_t bytes = _item->count();

            if (dst_size >= bytes) {
                memcpy((void *)dst_data, (void *)_item->data(), bytes);

                ++m_iR;
                --m_count;

                if (m_iR == m_depth) {
                    m_iR = 0;
                }

                pthread_mutex_unlock(&m_mutex);

                return (int32_t)bytes;
            }
        }

        pthread_mutex_unlock(&m_mutex);
    }

    return -1;
}
