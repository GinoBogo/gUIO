
////////////////////////////////////////////////////////////////////////////////
/// \file      GFiFo.cpp
/// \version   0.1
/// \date      May, 2016
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GFiFo.hpp"

#include "GDefine.hpp"

#include <algorithm> // std::min

GFiFo::GFiFo(const uint32_t item_size, const uint32_t fifo_depth, const int max_level, const int min_level) {
    m_size      = item_size;
    m_depth     = fifo_depth;
    m_max_level = max_level < 1 ? -1 : std::min(max_level, static_cast<int>(fifo_depth));
    m_min_level = min_level < 0 ? -1 : std::min(min_level, static_cast<int>(fifo_depth));

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

void GFiFo::wipe_resources() {
    m_max_level = 0;
    m_used      = 0;
    m_iW        = 0;
    m_iR        = 0;

    if (m_max_level >= 1 && m_min_level >= 0 && m_max_level > m_min_level) {
        m_fsm_level = MIN_LEVEL_PASSED;
    }
    else {
        m_fsm_level = TRANSITION_OFF;
    }
}

void GFiFo::Reset() {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    RETURN_IF(p_fifo == nullptr, wipe_resources());

    for (decltype(m_depth) i{0}; i < m_depth; ++i) {
        p_fifo[i]->Reset();
    }
}

void GFiFo::Clear() {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    RETURN_IF(p_fifo == nullptr, wipe_resources());

    for (decltype(m_depth) i{0}; i < m_depth; ++i) {
        p_fifo[i]->Clear();
    }
}

void GFiFo::SmartClear() {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    RETURN_IF(p_fifo == nullptr, wipe_resources());

    for (decltype(m_depth) i{0}; i < m_depth; ++i) {
        p_fifo[i]->SmartClear();
    }
}

bool GFiFo::Push(const GBuffer* src_buff) {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    const bool error_1 = src_buff == nullptr;
    const bool error_2 = IsFull();

    if (error_1 || error_2) {
        return false;
    }

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

    return false;
}

bool GFiFo::Push(const uint8_t* src_data, const uint32_t src_count) {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    const bool error_1 = src_data == nullptr;
    const bool error_2 = src_count == 0;
    const bool error_3 = IsFull();

    if (error_1 || error_2 || error_3) {
        return false;
    }

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

    return false;
}

bool GFiFo::Pop(GBuffer* dst_buff) {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    const bool error_1 = dst_buff == nullptr;
    const bool error_2 = IsEmpty();

    if (error_1 || error_2) {
        return false;
    }

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

    return false;
}

int32_t GFiFo::Pop(uint8_t* dst_data, const uint32_t dst_size) {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    const bool error_1 = dst_data == nullptr;
    const bool error_2 = dst_size == 0;
    const bool error_3 = IsEmpty();

    if (error_1 || error_2 || error_3) {
        return false;
    }

    GBuffer* _item = p_fifo[m_iR];
    uint32_t bytes = _item->used();

    if (dst_size >= bytes) {
        memcpy(static_cast<void*>(dst_data), static_cast<void*>(_item->data()), bytes);

        ++m_iR;
        --m_used;

        if (m_iR == m_depth) {
            m_iR = 0;
        }

        return (int32_t)bytes;
    }

    return -1;
}

bool GFiFo::IsLevelChanged(fsm_levels_t* new_fsm_level, fsm_levels_t* old_fsm_level) {
#ifdef GFIFO_THREAD_SAFE
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    auto _state_changed{false};

    auto _current_level{static_cast<int>(m_used)};

    DO_IF(old_fsm_level != nullptr, *old_fsm_level = m_fsm_level);

    GOTO_IF(m_fsm_level == TRANSITION_OFF, label_exit);

    if (m_min_level < _current_level && _current_level < m_max_level) {
        _state_changed = m_fsm_level != REGULAR_LEVEL;
        m_fsm_level    = REGULAR_LEVEL;
        goto label_exit;
    }

    if (_current_level >= m_max_level) {
        _state_changed = m_fsm_level != MAX_LEVEL_PASSED;
        m_fsm_level    = MAX_LEVEL_PASSED;
        goto label_exit;
    }

    if (_current_level <= m_min_level) {
        _state_changed = m_fsm_level != MIN_LEVEL_PASSED;
        m_fsm_level    = MIN_LEVEL_PASSED;
    }

label_exit:
    DO_IF(new_fsm_level != nullptr, *new_fsm_level = m_fsm_level);

    return _state_changed;
}
