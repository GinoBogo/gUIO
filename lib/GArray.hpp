
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
#include <cstdint>     // uint8_t
#include <type_traits> // is_fundamental_v

template <typename T> class GArray {
    public:
    GArray(size_t size) {
        static_assert(std::is_fundamental_v<T>, "Type not supported.");

        m_data = new T[size];
        m_size = size;
        Reset();
    }

    GArray(const GArray& array) {
        *this = array;
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

            for (decltype(m_used) i{0}; i < m_used; ++i) {
                m_data[i] = array.m_data[i];
            }
        }
        return *this;
    }

    void Reset() {
        m_used = 0;
    }

    [[nodiscard]] auto data() const {
        return m_data;
    }

    [[nodiscard]] auto data_bytes() const {
        return reinterpret_cast<uint8_t*>(m_data);
    }

    [[nodiscard]] auto size() const {
        return m_size;
    }

    [[nodiscard]] auto size_bytes() const {
        return m_size * sizeof(T);
    }

    [[nodiscard]] auto used() const {
        return m_used;
    }

    [[nodiscard]] auto used_bytes() const {
        return m_used * sizeof(T);
    }

    auto used(const size_t words) {
        auto _res{words <= m_size};
        if (_res) {
            m_used = words;
        }
        return _res;
    }

    [[nodiscard]] auto free() const {
        return m_size - m_used;
    }

    [[nodiscard]] auto free_bytes() const {
        return (m_size - m_used) * sizeof(T);
    }

    auto free(const size_t words) {
        auto _res{words <= m_size};
        if (_res) {
            m_used = m_size - words;
        }
        return _res;
    }

    private:
    auto release_resources() {
        if (m_data != nullptr) {
            delete[] m_data;
            m_data = nullptr;
            m_size = 0;
            Reset();
        }
    }

    T*     m_data{nullptr};
    size_t m_size{0};
    size_t m_used{0};
};

#endif // GARRAY_HPP
