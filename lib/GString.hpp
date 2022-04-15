////////////////////////////////////////////////////////////////////////////////
/// \file      GString.hpp
/// \version   0.1
/// \date      April, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GSTRING_HPP
#define GSTRING_HPP

#include <cstdint> // uint8_t, uint16_t, uint32_t
#include <cstdlib> // strtoul
#include <string>  // string

namespace GString {

    auto strtouc(std::string& _str) {
        auto _base{10};
        auto _last{_str.back()};

        if (_last == 'h' || _last == 'H') {
            _str.pop_back();
            _base = 16;
        }
        else if (_last == 'b' || _last == 'B') {
            _str.pop_back();
            _base = 2;
        }

        return static_cast<uint8_t>(std::strtoul(_str.c_str(), 0, _base));
    }

    auto strtous(std::string& _str) {
        auto _base{10};
        auto _last{_str.back()};

        if (_last == 'h' || _last == 'H') {
            _str.pop_back();
            _base = 16;
        }
        else if (_last == 'b' || _last == 'B') {
            _str.pop_back();
            _base = 2;
        }

        return static_cast<uint16_t>(std::strtoul(_str.c_str(), 0, _base));
    }

    auto strtoui(std::string& _str) {
        auto _base{10};
        auto _last{_str.back()};

        if (_last == 'h' || _last == 'H') {
            _str.pop_back();
            _base = 16;
        }
        else if (_last == 'b' || _last == 'B') {
            _str.pop_back();
            _base = 2;
        }

        return static_cast<uint32_t>(std::strtoul(_str.c_str(), 0, _base));
    }

    auto strtoul(std::string& value) {
        auto _base{10};
        auto _last{value.back()};

        if (_last == 'h' || _last == 'H') {
            value.pop_back();
            _base = 16;
        }
        else if (_last == 'b' || _last == 'B') {
            value.pop_back();
            _base = 2;
        }

        return std::strtoul(value.c_str(), 0, _base);
    }

} // namespace GString

#endif // GSTRING_HPP
