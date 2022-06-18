
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

        std::regex                 _rgx{regex};
        std::sregex_token_iterator next{_str.begin(), _str.end(), _rgx, -1};
        std::sregex_token_iterator last;

        while (next != last) {
            tokens.push_back(next->str());
            ++next;
        }

        auto filter = [](const std::string& _str) {
            return _str.empty();
        };
        auto junks{std::remove_if(tokens.begin(), tokens.end(), filter)};
        tokens.erase(junks, tokens.end());
        return tokens;
    }

    template <typename T = double> static std::pair<T, std::string> value_scaler(T value, const std::string& unit) {
        auto _modulus{std::abs(value)};

        const auto __G{static_cast<T>(1e+9)};
        const auto __M{static_cast<T>(1e+6)};
        const auto __K{static_cast<T>(1e+3)};
        const auto __m{static_cast<T>(1e-3)};
        const auto __u{static_cast<T>(1e-6)};
        const auto __n{static_cast<T>(1e-9)};

        if (_modulus >= __G) {
            return std::make_pair(value / __G, "G" + unit);
        }
        if (_modulus >= __M) {
            return std::make_pair(value / __M, "M" + unit);
        }
        if (_modulus >= __K) {
            return std::make_pair(value / __K, "K" + unit);
        }
        if (_modulus <= __n) {
            return std::make_pair(value / __n, "n" + unit);
        }
        if (_modulus <= __u) {
            return std::make_pair(value / __u, "µ" + unit);
        }
        if (_modulus <= __m) {
            return std::make_pair(value / __m, "m" + unit);
        }

        return std::make_pair(value, unit);
    }
};

#endif // GSTRING_HPP
