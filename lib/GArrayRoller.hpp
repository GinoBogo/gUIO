
////////////////////////////////////////////////////////////////////////////////
/// \file      GArrays.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GARRAYROLLER_HPP
#define GARRAYROLLER_HPP

#include "GArray.hpp"  // GArray
#include "GDefine.hpp" // DO_IF, GOTO_IF
#include "GLogger.hpp" // LOG_FORMAT, debug

#include <algorithm> // min
#include <mutex>     // lock_guard, mutex

template <typename T> class GArrayRoller {
  public:
    typedef enum {
        IS_UNCLAIMED,
        IS_READING,
        IS_WRITING,
        IS_READING_AND_WRITING

    } fsm_states_t;

    typedef enum {
        TRANSITION_OFF,
        REGULAR_LEVEL,
        MAX_LEVEL_PASSED,
        MIN_LEVEL_PASSED

    } fsm_levels_t;

    GArrayRoller(size_t array_length, size_t arrays_number, const std::string& tag_name = "", int max_level = -1, int min_level = -1) {
        static_assert(std::is_fundamental_v<T>, "Type not supported.");

        m_length    = array_length;
        m_number    = arrays_number;
        m_tag_name  = tag_name.empty() ? "Array Roller" : "\"" + tag_name + "\" Array Roller";
        m_max_level = max_level < 1 ? -1 : std::min(max_level, static_cast<int>(m_number));
        m_min_level = min_level < 0 ? -1 : std::min(min_level, static_cast<int>(m_number));

        if (m_length > 0 && m_number > 0) {
            m_arrays = new GArray<T>*[m_number];
            for (decltype(m_number) i{0}; i < m_number; ++i) {
                m_arrays[i] = new GArray<T>(m_length);
            }
        }
        Reset();
        LOG_FORMAT(debug, "%s constructor [%lu, %lu, %d, %d]", m_tag_name.c_str(), m_length, m_number, m_max_level, m_min_level);
    }

    GArrayRoller(const GArrayRoller& array_roller) = delete;

    ~GArrayRoller() {
        if (m_arrays != nullptr) {
            Reset();
            for (decltype(m_number) i{0}; i < m_number; ++i) {
                delete m_arrays[i];
                m_arrays[i] = nullptr;
            }
            delete[] m_arrays;
            m_arrays = nullptr;
        }

        LOG_FORMAT(debug, "%s destructor", m_tag_name.c_str());
    }

    GArrayRoller& operator=(const GArrayRoller& array_roller) = delete;

    void Reset() {
        std::lock_guard<std::mutex> _lock(m_mutex);

        if (m_arrays != nullptr) {
            for (decltype(m_number) i{0}; i < m_number; ++i) {
                m_arrays[i]->Reset();
            }
        }

        m_fsm_state = IS_UNCLAIMED;
        m_max_used  = 0;
        m_errors    = 0;
        m_used      = 0;
        m_iR        = 0;
        m_iW        = 0;

        if (m_max_level >= 1 && m_min_level >= 0 && m_max_level > m_min_level) {
            m_fsm_level = MIN_LEVEL_PASSED;
        }
        else {
            m_fsm_level = TRANSITION_OFF;
        }
    }

    auto Reading_Start(bool& error) {
        std::lock_guard<std::mutex> _lock(m_mutex);

        error = (m_used == 0);

        if (!error) {
            switch (m_fsm_state) {
                case IS_UNCLAIMED:
                    m_fsm_state = IS_READING;
                    break;

                case IS_WRITING:
                    m_fsm_state = IS_READING_AND_WRITING;
                    break;

                default:
                    error = true;
                    break;
            }
        }

        m_errors += (unsigned)error;

        return m_arrays[m_iR];
    }

    auto Reading_Stop(bool& error) {
        std::lock_guard<std::mutex> _lock(m_mutex);

        error = false;

        switch (m_fsm_state) {
            case IS_READING:
                if (++m_iR == m_number) {
                    m_iR = 0;
                }
                m_used--;
                m_fsm_state = IS_UNCLAIMED;
                break;

            case IS_READING_AND_WRITING:
                if (++m_iR == m_number) {
                    m_iR = 0;
                }
                m_used--;
                m_fsm_state = IS_WRITING;
                break;

            default:
                error = true;
                break;
        }

        m_errors += (unsigned)error;
    }

    auto Writing_Start(bool& error) {
        std::lock_guard<std::mutex> _lock(m_mutex);

        error = !(m_used < m_number);

        if (!error) {
            switch (m_fsm_state) {
                case IS_UNCLAIMED:
                    m_fsm_state = IS_WRITING;
                    break;

                case IS_READING:
                    m_fsm_state = IS_READING_AND_WRITING;
                    break;

                default:
                    error = true;
                    break;
            }
        }

        m_errors += (unsigned)error;

        return m_arrays[m_iW];
    }

    auto Writing_Stop(bool& error) {
        std::lock_guard<std::mutex> _lock(m_mutex);

        error = false;

        switch (m_fsm_state) {
            case IS_WRITING:
                if (++m_iW == m_number) {
                    m_iW = 0;
                }
                m_used++;
                m_fsm_state = IS_UNCLAIMED;
                break;

            case IS_READING_AND_WRITING:
                if (++m_iW == m_number) {
                    m_iW = 0;
                }
                m_used++;
                m_fsm_state = IS_READING;
                break;

            default:
                error = true;
                break;
        }

        m_errors += (unsigned)error;
    }

    bool IsLevelChanged(fsm_levels_t* new_fsm_level = nullptr, fsm_levels_t* old_fsm_level = nullptr) {
        std::lock_guard<std::mutex> _lock(m_mutex);

        auto _state_changed{false};

        auto _current_level{static_cast<int>(m_used)};

        DO_IF(old_fsm_level != nullptr, *old_fsm_level = m_fsm_level);

        GOTO_IF(m_fsm_level == TRANSITION_OFF, label_exit, );

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

    [[nodiscard]] auto length() const {
        return m_length;
    }

    [[nodiscard]] auto number() const {
        return m_number;
    }

    [[nodiscard]] auto max_level() const {
        return m_max_level;
    }

    [[nodiscard]] auto min_level() const {
        return m_min_level;
    }

    // WARNING: thread unsafe
    [[nodiscard]] auto fsm_state() const {
        return m_fsm_state;
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
    [[nodiscard]] auto errors() const {
        return m_errors;
    }

    // WARNING: thread unsafe
    [[nodiscard]] auto used() const {
        return m_used;
    }

    // WARNING: thread unsafe
    [[nodiscard]] auto free() const {
        return m_number - m_used;
    }

  private:
    size_t      m_length;
    size_t      m_number;
    std::string m_tag_name;
    int         m_max_level;
    int         m_min_level;

    fsm_states_t m_fsm_state;
    fsm_levels_t m_fsm_level;
    size_t       m_max_used;
    size_t       m_errors;

    GArray<T>** m_arrays{nullptr};
    size_t      m_used;
    size_t      m_iR;
    size_t      m_iW;

    std::mutex m_mutex;
};

#endif // GARRAYROLLER_HPP
