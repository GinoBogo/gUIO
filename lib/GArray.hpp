////////////////////////////////////////////////////////////////////////////////
/// \file      GArray.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GARRAY_HPP
#define GARRAY_HPP

#include <cstddef> // size_t

template <typename T> class GArray {
    public:
    GArray(size_t length) {
        len = length;
        ptr = new T[length];
    }

    ~GArray() {
        delete[] ptr;
    }

    size_t len;
    T*     ptr;
};

#endif // GARRAY_HPP
