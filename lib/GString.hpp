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
#include <vector>  // vector

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

    auto strtoul(std::string& _str) {
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

        return std::strtoul(_str.c_str(), 0, _base);
    }

    auto split(const std::string& _str, const std::string& regex) {
        std::vector<std::string> tokens;

        std::regex                 re{regex};
        std::sregex_token_iterator next{_str.begin(), _str.end(), re, -1};
        std::sregex_token_iterator last;

        while (next != last) {
            tokens.push_back(next->str());
            ++next;
        }

        auto filter = [](const std::string s) {
            return (s.size() == 0);
        };
        auto junks{std::remove_if(tokens.begin(), tokens.end(), filter)};
        tokens.erase(junks, tokens.end());
        return tokens;
    }

} // namespace GString

#endif // GSTRING_HPP
