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
#include <type_traits> // is_fundamental_v

template <typename T> class GArray {
    public:
    GArray(size_t size) {
        static_assert(std::is_fundamental_v<T>, "Type not supported.");

        m_data = new T[size];
        m_size = size;
        m_used = 0;
    }

    GArray(const GArray& array) {
        if (this != &array) {
            m_data = new T[array.m_size];
            m_size = array.m_size;
            m_used = array.m_used;

            for (decltype(m_used) i{0}; i < m_used; ++i) m_data[i] = array.m_data[i];
        }
    }

    ~GArray() {
        release_resources();
    }

    GArray& operator=(const GArray& array) {
        if (this != &array) {
            release_resources();

            m_data = new T[array.m_size];
            m_size = array.m_size;
            m_used = array.m_used;

            for (decltype(m_used) i{0}; i < m_used; ++i) m_data[i] = array.m_data[i];
        }
        return *this;
    }

    auto data() {
        return m_data;
    }

    auto size() {
        return m_size;
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
    auto release_resources() {
        if (m_data != nullptr) {
            delete[] m_data;
            m_data = nullptr;
        }
    }

    T*     m_data{nullptr};
    size_t m_size{0};
    size_t m_used{0};
};

#endif // GARRAY_HPP
