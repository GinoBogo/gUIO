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

#include <mutex>

template <typename T> class GArrayRoller {
    public:
    typedef enum {
        IS_UNCLAIMED, // Finite-State Machine
        IS_READING,
        IS_WRITING,
        IS_READING_AND_WRITING
    } fsm_state_t;

    GArrayRoller(size_t array_length, size_t arrays_number) : m_length{array_length}, m_number{arrays_number} {
        m_arrays = nullptr;
        if (m_length > 0 && m_number > 0) {
            m_arrays = new GArray<T>*[m_number];
            for (auto i{0U}; i < m_number; ++i) {
                m_arrays[i] = new GArray<T>(m_length);
            }
        }
        Reset();
    }

    ~GArrayRoller() {
        if (m_arrays != nullptr) {
            for (auto i{0U}; i < m_number; ++i) {
                delete m_arrays[i];
            }
            delete[] m_arrays;
        }
    }

    void Reset() {
        std::lock_guard<std::mutex> _lock(m_mutex);

        m_status = IS_UNCLAIMED;
        m_errors = 0;
        m_count  = 0;
        m_iR     = 0;
        m_iW     = 0;
    }

    auto Reading_Start(bool& error) {
        std::lock_guard<std::mutex> _lock(m_mutex);

        error = m_count == 0;

        if (!error) {
            switch (m_status) {
                case IS_UNCLAIMED:
                    m_status = IS_READING;
                    m_count--;
                    break;

                case IS_READING:
                case IS_READING_AND_WRITING:
                    error = true;
                    break;

                case IS_WRITING:
                    m_status = IS_READING_AND_WRITING;
                    m_count--;
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
                if (++m_iR == m_number) m_iR = 0;
                m_status = IS_UNCLAIMED;
                break;

            case IS_READING_AND_WRITING:
                if (++m_iR == m_number) m_iR = 0;
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

        error = m_count < m_number;

        if (!error) {
            switch (m_status) {
                case IS_UNCLAIMED:
                    m_status = IS_WRITING;
                    m_count++;
                    break;

                case IS_WRITING:
                case IS_READING_AND_WRITING:
                    error = true;
                    break;

                case IS_READING:
                    m_status = IS_READING_AND_WRITING;
                    m_count++;
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
                if (++m_iW == m_number) m_iW = 0;
                m_status = IS_UNCLAIMED;
                break;

            case IS_READING_AND_WRITING:
                if (++m_iW == m_number) m_iW = 0;
                m_status = IS_READING;
                break;

            default:
                error = true;
                break;
        }

        m_errors += (unsigned)error;
    }

    auto length() const {
        return m_length;
    }

    auto number() const {
        return m_number;
    }

    auto status() const {
        return m_status;
    }

    auto errors() const {
        return m_errors;
    }

    private:
    const size_t m_length;
    const size_t m_number;
    fsm_state_t  m_status;
    size_t       m_errors;

    GArray<T>** m_arrays;
    size_t      m_count;
    size_t      m_iR;
    size_t      m_iW;

    std::mutex m_mutex;
};

#endif // GARRAYROLLER_HPP
