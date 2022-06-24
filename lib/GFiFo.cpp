
////////////////////////////////////////////////////////////////////////////////
/// \file      GFiFo.cpp
/// \version   0.1
/// \date      May, 2016
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GFiFo.hpp"

#include <algorithm> // std::min

GFiFo::GFiFo(const uint32_t item_size, const uint32_t fifo_depth, const int max_level, const int min_level) {
    m_size      = item_size;
    m_depth     = fifo_depth;
    m_max_level = max_level < 1 ? -1 : std::min(max_level, static_cast<int>(fifo_depth));
    m_min_level = min_level < 0 ? -1 : std::min(min_level, static_cast<int>(fifo_depth));

    if (m_max_level >= 1 && m_min_level >= 0 && m_max_level > m_min_level) {
        m_fsm_level = MIN_LEVEL_PASSED;
    }
    else {
        m_fsm_level = TRANSITION_OFF;
    }

    if ((m_size > 0) && (m_depth > 0)) {
        p_fifo = new GBuffer*[m_depth];

        for (decltype(m_depth) i{0}; i < m_depth; ++i) {
            p_fifo[i] = new GBuffer(m_size);
        }
    }
    Reset();
}

GFiFo::~GFiFo() {
    for (decltype(m_depth) i{0}; i < m_depth; ++i) {
        delete p_fifo[i];
        p_fifo[i] = nullptr;
    }

    delete[] p_fifo;
    p_fifo = nullptr;
}

void GFiFo::Reset() {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    if (p_fifo != nullptr) {
        for (decltype(m_depth) i{0}; i < m_depth; ++i) {
            p_fifo[i]->Reset();
        }
    }

    m_used = 0;
    m_iW   = 0;
    m_iR   = 0;

    if (m_fsm_level != TRANSITION_OFF) {
        m_fsm_level = MIN_LEVEL_PASSED;
    }
}

void GFiFo::Clear() {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    if (p_fifo != nullptr) {
        for (decltype(m_depth) i{0}; i < m_depth; ++i) {
            p_fifo[i]->Clear();
        }
    }

    m_used = 0;
    m_iW   = 0;
    m_iR   = 0;

    if (m_fsm_level != TRANSITION_OFF) {
        m_fsm_level = MIN_LEVEL_PASSED;
    }
}

void GFiFo::SmartClear() {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    if (p_fifo != nullptr) {
        for (decltype(m_depth) i{0}; i < m_depth; ++i) {
            p_fifo[i]->SmartClear();
        }
    }

    m_used = 0;
    m_iW   = 0;
    m_iR   = 0;

    if (m_fsm_level != TRANSITION_OFF) {
        m_fsm_level = MIN_LEVEL_PASSED;
    }
}

bool GFiFo::Push(const GBuffer* src_buff) {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    if (src_buff != nullptr) {
        if (!IsFull()) {
            GBuffer* _item = p_fifo[m_iW];

            _item->Reset();

            if (_item->Append(src_buff->data(), src_buff->used())) {
                ++m_iW;
                ++m_used;

                if (m_iW == m_depth) {
                    m_iW = 0;
                }

                return true;
            }
        }
    }

    return false;
}

bool GFiFo::Push(const uint8_t* src_data, const uint32_t src_count) {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    if ((src_data != nullptr) && (src_count > 0)) {
        if (!IsFull()) {
            GBuffer* _item = p_fifo[m_iW];

            _item->Reset();

            if (_item->Append(src_data, src_count)) {
                ++m_iW;
                ++m_used;

                if (m_iW == m_depth) {
                    m_iW = 0;
                }

                return true;
            }
        }
    }

    return false;
}

bool GFiFo::Pop(GBuffer* dst_buff) {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    if (dst_buff != nullptr) {
        if (!IsEmpty()) {
            GBuffer* _item = p_fifo[m_iR];

            dst_buff->Reset();

            if (dst_buff->Append(_item->data(), _item->used())) {
                ++m_iR;
                --m_used;

                if (m_iR == m_depth) {
                    m_iR = 0;
                }

                return true;
            }
        }
    }

    return false;
}

int32_t GFiFo::Pop(uint8_t* dst_data, const uint32_t dst_size) {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    if ((dst_data != nullptr) && (dst_size > 0)) {
        if (!IsEmpty()) {
            GBuffer* _item = p_fifo[m_iR];
            uint32_t bytes = _item->used();

            if (dst_size >= bytes) {
                memcpy((void*)dst_data, (void*)_item->data(), bytes);

                ++m_iR;
                --m_used;

                if (m_iR == m_depth) {
                    m_iR = 0;
                }

                return (int32_t)bytes;
            }
        }
    }

    return -1;
}

bool GFiFo::IsLevelChanged(fsm_levels_t* new_level, fsm_levels_t* old_level) {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    auto _state_changed{false};

    if (old_level != nullptr) {
        *old_level = m_fsm_level;
    }

    if (m_fsm_level != TRANSITION_OFF) {
        auto _current_level{static_cast<int>(m_used)};

        if (m_max_level >= 1 && _current_level >= m_max_level) {
            _state_changed = m_fsm_level != MAX_LEVEL_PASSED;
            m_fsm_level    = MAX_LEVEL_PASSED;
        }

        if (m_min_level >= 0 && _current_level <= m_min_level) {
            _state_changed = m_fsm_level != MIN_LEVEL_PASSED;
            m_fsm_level    = MIN_LEVEL_PASSED;
        }
    }

    if (new_level != nullptr) {
        *new_level = m_fsm_level;
    }

    return _state_changed;
}
