
////////////////////////////////////////////////////////////////////////////////
/// \file      GArrays.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GARRAYROLLER_HPP
#define GARRAYROLLER_HPP

#include "GArray.hpp"

#include <mutex>       // mutex, lock_guard
#include <type_traits> // is_fundamental_v

template <typename T> class GArrayRoller {
    public:
    typedef enum {
        IS_UNCLAIMED, // Finite-State Machine
        IS_READING,
        IS_WRITING,
        IS_READING_AND_WRITING
    } fsm_state_t;

    GArrayRoller(size_t array_length, size_t arrays_number) {
        static_assert(std::is_fundamental_v<T>, "Type not supported.");

        m_length = array_length;
        m_number = arrays_number;

        if (m_length > 0 && m_number > 0) {
            m_arrays = new GArray<T>*[m_number];
            for (decltype(m_number) i{0}; i < m_number; ++i) {
                m_arrays[i] = new GArray<T>(m_length);
            }
        }
        Reset();
    }

    GArrayRoller(const GArrayRoller& array_roller) {
        *this = array_roller;
    }

    ~GArrayRoller() {
        release_resources();
    }

    GArrayRoller& operator=(const GArrayRoller& array_roller) {
        if (this != &array_roller) {
            release_resources();

            m_length = array_roller.m_length;
            m_number = array_roller.m_number;

            if (m_length > 0 && m_number > 0) {
                m_arrays = new GArray<T>*[m_number];
                for (decltype(m_number) i{0}; i < m_number; ++i) {
                    m_arrays[i] = new GArray<T>(*array_roller.m_arrays[i]);
                }
                m_status = array_roller.m_status;
                m_errors = array_roller.m_errors;
                m_used   = array_roller.m_used;
                m_iR     = array_roller.m_iR;
                m_iW     = array_roller.m_iW;
            }
        }
        return *this;
    }

    void Reset() {
        std::lock_guard<std::mutex> _lock(m_mutex);

        if (m_arrays != nullptr) {
            for (decltype(m_number) i{0}; i < m_number; ++i) {
                m_arrays[i]->Reset();
            }
        }

        m_status = IS_UNCLAIMED;
        m_errors = 0;
        m_used   = 0;
        m_iR     = 0;
        m_iW     = 0;
    }

    auto Reading_Start(bool& error) {
        std::lock_guard<std::mutex> _lock(m_mutex);

        error = (m_used == 0);

        if (!error) {
            switch (m_status) {
                case IS_UNCLAIMED:
                    m_status = IS_READING;
                    break;

                case IS_READING:
                case IS_READING_AND_WRITING:
                    error = true;
                    break;

                case IS_WRITING:
                    m_status = IS_READING_AND_WRITING;
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

        switch (m_status) {
            case IS_UNCLAIMED:
            case IS_WRITING:
                error = true;
                break;

            case IS_READING:
                if (++m_iR == m_number) {
                    m_iR = 0;
                }
                m_used--;
                m_status = IS_UNCLAIMED;
                break;

            case IS_READING_AND_WRITING:
                if (++m_iR == m_number) {
                    m_iR = 0;
                }
                m_used--;
                m_status = IS_WRITING;
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
            switch (m_status) {
                case IS_UNCLAIMED:
                    m_status = IS_WRITING;
                    break;

                case IS_WRITING:
                case IS_READING_AND_WRITING:
                    error = true;
                    break;

                case IS_READING:
                    m_status = IS_READING_AND_WRITING;
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

        switch (m_status) {
            case IS_UNCLAIMED:
            case IS_READING:
                error = true;
                break;

            case IS_WRITING:
                if (++m_iW == m_number) {
                    m_iW = 0;
                }
                m_used++;
                m_status = IS_UNCLAIMED;
                break;

            case IS_READING_AND_WRITING:
                if (++m_iW == m_number) {
                    m_iW = 0;
                }
                m_used++;
                m_status = IS_READING;
                break;

            default:
                error = true;
                break;
        }

        m_errors += (unsigned)error;
    }

    [[nodiscard]] auto length() const {
        return m_length;
    }

    [[nodiscard]] auto number() const {
        return m_number;
    }

    // WARNING: thread unsafe
    [[nodiscard]] auto status() const {
        return m_status;
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
    void release_resources() {
        if (m_arrays != nullptr) {
            Reset();
            for (decltype(m_number) i{0}; i < m_number; ++i) {
                delete m_arrays[i];
                m_arrays[i] = nullptr;
            }
            delete[] m_arrays;
            m_arrays = nullptr;
        }
    }

    size_t      m_length;
    size_t      m_number;
    fsm_state_t m_status;
    size_t      m_errors;

    GArray<T>** m_arrays{nullptr};
    size_t      m_used;
    size_t      m_iR;
    size_t      m_iW;

    std::mutex m_mutex;
};

#endif // GARRAYROLLER_HPP
