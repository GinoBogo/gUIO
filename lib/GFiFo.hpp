
////////////////////////////////////////////////////////////////////////////////
/// \file      GFiFo.hpp
/// \version   0.1
/// \date      May, 2016
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GFIFO_HPP
#define GFIFO_HPP

#include "GBuffer.hpp" // GBuffer

// #define GFIFO_THREAD_SAFE

#ifdef GFIFO_THREAD_SAFE
#include <mutex>
#else
#define GFIFO_LOCK_GUARD
#endif

class GFiFo {
  public:
    typedef enum {
        TRANSITION_OFF, // Disable the Finite-State Machine
        REGULAR_LEVEL,
        MAX_LEVEL_PASSED,
        MIN_LEVEL_PASSED

    } fsm_levels_t;

    GFiFo(uint32_t item_size, uint32_t fifo_depth, int max_level = -1, int min_level = -1);

    GFiFo(const GFiFo& other) = delete;

    ~GFiFo();

    GFiFo& operator=(const GFiFo& other) = delete;

    void Reset();

    void Clear();

    void SmartClear();

    bool Push(const GBuffer* src_buff);

    bool Push(const uint8_t* src_data, uint32_t src_count);

    bool Pop(GBuffer* dst_buff);

    int32_t Pop(uint8_t* dst_data, uint32_t dst_size);

    bool IsLevelChanged(fsm_levels_t* new_fsm_level = nullptr, fsm_levels_t* old_fsm_level = nullptr);

    // WARNING: thread unsafe
    [[nodiscard]] auto IsEmpty() const {
        return (m_used == 0);
    }

    // WARNING: thread unsafe
    [[nodiscard]] auto IsFull() const {
        return (m_used == m_depth);
    }

    [[nodiscard]] auto size() const {
        return m_size;
    }

    [[nodiscard]] auto depth() const {
        return m_depth;
    }

    [[nodiscard]] auto max_level() const {
        return m_max_level;
    }

    [[nodiscard]] auto min_level() const {
        return m_min_level;
    }

    // WARNING: thread unsafe
    [[nodiscard]] auto fsm_level() const {
        return m_fsm_level;
    }

    // WARNING: thread unsafe
    [[nodiscard]] auto max_used() {
        if (m_max_used < m_used) {
            m_max_used = m_used;
        }
        return m_max_used;
    }

    // WARNING: thread unsafe
    [[nodiscard]] auto used() const {
        return m_used;
    }

    // WARNING: thread unsafe
    [[nodiscard]] auto free() const {
        return m_depth - m_used;
    }

  private:
    uint32_t m_size;
    uint32_t m_depth;
    int      m_max_level;
    int      m_min_level;

    fsm_levels_t m_fsm_level;
    uint32_t     m_max_used;

    GBuffer** p_fifo{nullptr};
    uint32_t  m_used;
    uint32_t  m_iR;
    uint32_t  m_iW;

#ifdef GFIFO_THREAD_SAFE
    std::mutex m_mutex;
#endif

    void wipe_resources();
};

#endif // GFIFO_HPP
