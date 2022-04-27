////////////////////////////////////////////////////////////////////////////////
/// \file      GFiFo.cpp
/// \version   0.1
/// \date      May, 2016
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GFiFo.hpp"

GFiFo::GFiFo(const uint32_t item_size, const uint32_t fifo_depth, const int max_level, const int min_level) {
    m_size  = item_size;
    m_depth = fifo_depth;
    m_count = 0;
    m_iW    = 0;
    m_iR    = 0;
    p_fifo  = nullptr;

    m_max_level = max_level < 1 ? -1 : std::min(max_level, static_cast<int>(fifo_depth));
    m_min_level = min_level < 0 ? -1 : std::min(min_level, static_cast<int>(fifo_depth));

    auto check_1{m_max_level == m_min_level};
    auto check_2{m_max_level > 0 && m_max_level < m_min_level};

    if (check_1 || check_2) {
        m_fsm_state = TRANSITION_OFF;
    }
    else {
        m_fsm_state = MIN_LEVEL_PASSED;
    }

    if (m_size && m_depth) {
        p_fifo = new GBuffer*[m_depth];

        for (decltype(m_depth) i{0}; i < m_depth; ++i) {
            p_fifo[i] = new GBuffer(m_size);
        }
    }
}

GFiFo::~GFiFo() {
    for (decltype(m_depth) i{0}; i < m_depth; ++i) {
        delete p_fifo[i];
    }

    delete[] p_fifo;
}

void GFiFo::Reset() {
    const std::lock_guard<std::mutex> lock(m_mutex);

    for (decltype(m_depth) i{0}; i < m_depth; ++i) {
        p_fifo[i]->Reset();
    }

    m_count = 0;
    m_iW    = 0;
    m_iR    = 0;

    if (m_fsm_state != TRANSITION_OFF) {
        m_fsm_state = MIN_LEVEL_PASSED;
    }
}

void GFiFo::Clear() {
    const std::lock_guard<std::mutex> lock(m_mutex);

    for (decltype(m_depth) i{0}; i < m_depth; ++i) {
        p_fifo[i]->Clear();
    }

    m_count = 0;
    m_iW    = 0;
    m_iR    = 0;

    if (m_fsm_state != TRANSITION_OFF) {
        m_fsm_state = MIN_LEVEL_PASSED;
    }
}

void GFiFo::SmartClear() {
    const std::lock_guard<std::mutex> lock(m_mutex);

    for (decltype(m_depth) i{0}; i < m_depth; ++i) {
        p_fifo[i]->SmartClear();
    }

    m_count = 0;
    m_iW    = 0;
    m_iR    = 0;

    if (m_fsm_state != TRANSITION_OFF) {
        m_fsm_state = MIN_LEVEL_PASSED;
    }
}

bool GFiFo::Push(const GBuffer* src_buff) {
    if (src_buff) {
        const std::lock_guard<std::mutex> lock(m_mutex);

        if (!IsFull()) {
            GBuffer* _item = p_fifo[m_iW];

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

bool GFiFo::Push(const uint8_t* src_data, const uint32_t src_count) {
    if (src_data && src_count) {
        const std::lock_guard<std::mutex> lock(m_mutex);

        if (!IsFull()) {
            GBuffer* _item = p_fifo[m_iW];

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

bool GFiFo::Pop(GBuffer* dst_buff) {
    if (dst_buff) {
        const std::lock_guard<std::mutex> lock(m_mutex);

        if (!IsEmpty()) {
            GBuffer* _item = p_fifo[m_iR];

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

int32_t GFiFo::Pop(uint8_t* dst_data, const uint32_t dst_size) {
    if (dst_data && dst_size) {
        const std::lock_guard<std::mutex> lock(m_mutex);

        if (!IsEmpty()) {
            GBuffer* _item = p_fifo[m_iR];
            uint32_t bytes = _item->count();

            if (dst_size >= bytes) {
                memcpy((void*)dst_data, (void*)_item->data(), bytes);

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

bool GFiFo::IsStateChanged(fsm_state_t* new_state, fsm_state_t* old_state) {
    auto _state_changed{false};

    if (old_state != nullptr) {
        *old_state = m_fsm_state;
    }

    if (m_fsm_state != TRANSITION_OFF) {
        auto _current_level{static_cast<int>(m_count)};

        if (m_max_level > 0 && _current_level >= m_max_level) {
            _state_changed = m_fsm_state != MAX_LEVEL_PASSED;
            m_fsm_state    = MAX_LEVEL_PASSED;
        }

        if (m_min_level > 0 && _current_level <= m_min_level) {
            _state_changed = m_fsm_state != MIN_LEVEL_PASSED;
            m_fsm_state    = MIN_LEVEL_PASSED;
        }
    }

    if (new_state != nullptr) {
        *new_state = m_fsm_state;
    }

    return _state_changed;
}
