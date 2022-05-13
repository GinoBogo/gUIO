////////////////////////////////////////////////////////////////////////////////
/// \file      GArray.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GARRAY_HPP
#define GARRAY_HPP

#include <cstddef>     // size_t
#include <type_traits> // is_pointer_v

template <typename T> class GArray {
    public:
    GArray(size_t size) : m_size{size} {
        m_data = new T[m_size];
        m_used = 0;
    }

    ~GArray() {
        if constexpr (std::is_pointer_v<T>) {
            for (decltype(m_used) i{0}; i < m_size;) delete m_data[i++];
        }
        delete[] m_data;
    }

    auto size() {
        return m_size;
    }

    auto data() {
        return m_data;
    }

    auto used() {
        return m_used;
    }

    auto used(const size_t value) {
        auto _res{value <= m_size};
        if (_res) m_used = value;
        return _res;
    }

    auto free() {
        return m_size - m_used;
    }

    auto free(const size_t value) {
        auto _res{value <= m_size};
        if (_res) m_used = m_size - value;
        return _res;
    }

    private:
    const size_t m_size;
    T*           m_data;
    size_t       m_used;
};

#endif // GARRAY_HPP
