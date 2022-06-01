
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
#include <regex>   // regex
#include <string>  // string
#include <utility> // pair
#include <vector>  // vector

class GString {
    public:
    static auto strtouc(std::string& _str) {
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

        return static_cast<uint8_t>(std::strtoul(_str.c_str(), nullptr, _base));
    }

    static auto strtous(std::string& _str) {
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

        return static_cast<uint16_t>(std::strtoul(_str.c_str(), nullptr, _base));
    }

    static auto strtoui(std::string& _str) {
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

        return static_cast<uint32_t>(std::strtoul(_str.c_str(), nullptr, _base));
    }

    static auto strtoul(std::string& _str) {
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

        return std::strtoul(_str.c_str(), nullptr, _base);
    }

    static auto split(const std::string& _str, const std::string& regex) {
        std::vector<std::string> tokens;

        std::regex                 re{regex};
        std::sregex_token_iterator next{_str.begin(), _str.end(), re, -1};
        std::sregex_token_iterator last;

        while (next != last) {
            tokens.push_back(next->str());
            ++next;
        }

        auto filter = [](const std::string& s) {
            return s.empty();
        };
        auto junks{std::remove_if(tokens.begin(), tokens.end(), filter)};
        tokens.erase(junks, tokens.end());
        return tokens;
    }

    template <typename T = double> static std::pair<T, std::string> value_scaler(T value, const std::string& unit) {
        auto _value{std::abs(value)};

        const auto _G{static_cast<T>(1e+9)};
        const auto _M{static_cast<T>(1e+6)};
        const auto _K{static_cast<T>(1e+3)};
        const auto _m{static_cast<T>(1e-3)};
        const auto _u{static_cast<T>(1e-6)};
        const auto _n{static_cast<T>(1e-9)};

        if (_value >= _G) {
            return std::make_pair(value / _G, "G" + unit);
        }
        if (_value >= _M) {
            return std::make_pair(value / _M, "M" + unit);
        }
        if (_value >= _K) {
            return std::make_pair(value / _K, "K" + unit);
        }
        if (_value <= _n) {
            return std::make_pair(value / _n, "n" + unit);
        }
        if (_value <= _u) {
            return std::make_pair(value / _u, "µ" + unit);
        }
        if (_value <= _m) {
            return std::make_pair(value / _m, "m" + unit);
        }

        return std::make_pair(value, unit);
    }
};

#endif // GSTRING_HPP
