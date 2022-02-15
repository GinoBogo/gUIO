////////////////////////////////////////////////////////////////////////////////
/// \file      GProfile.hpp
/// \version   0.1
/// \date      December, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GPROFILE_HPP
#define GPROFILE_HPP

#include <chrono>

class GProfile {
    public:
    void Start() {
        m_t0 = std::chrono::system_clock::now();
    }

    void Stop() {
        m_t1 = std::chrono::system_clock::now();
    }

    auto ms() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_t1 - m_t0).count();
    }

    auto us() {
        return std::chrono::duration_cast<std::chrono::microseconds>(m_t1 - m_t0).count();
    }

    auto ns() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(m_t1 - m_t0).count();
    }

    private:
    std::chrono::time_point<std::chrono::system_clock> m_t0;
    std::chrono::time_point<std::chrono::system_clock> m_t1;
};

#endif // GPROFILE_HPP