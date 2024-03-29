
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
#include <cstring> // strlen
#include <regex>   // regex, sregex_token_iterator
#include <sstream> // stringstream
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

    static auto intrcpy(char* _dst, int __n, int __r) {
        while (true) {
            auto _div = __n / 10;
            auto _rem = __n % 10;

            if ((_div == 0) && (_rem == 0)) {
                break;
            }
            _dst[__r--] = '0' + _rem;

            __n = _div;
        }
    }

    static auto strrcpy(char* _dst, const char* _src, int __r) {
        auto __n = (int)strlen(_src);
        auto __s = __r - __n + 1;

        for (auto i{0}; i < __n; ++i) {
            _dst[__s + i] = _src[i];
        }
    }

    static auto sanitize(std::string& line) {
        auto remark = false;
        auto filter = [&remark](char __c) {
            remark |= (__c == '!') || (__c == '#') || (__c == ';');
            return remark || (bool)std::isspace(__c);
        };

        line.erase(std::remove_if(line.begin(), line.end(), filter), line.end());
    }

    static auto sanitize(std::vector<std::string>& items) {
        for (auto _it{items.begin()}; _it != items.end(); ++_it) {
            sanitize(*_it);
        }

        auto remark{false};

        auto filter = [&remark](std::string& __s) {
            if (!remark) {
                remark = __s.empty();
            }
            return remark;
        };
        auto junks{std::remove_if(items.begin(), items.end(), filter)};
        items.erase(junks, items.end());
    }

    static auto split(const std::string& _str, const std::string& regex) {
        std::vector<std::string> items;

        std::regex                 _rgx{regex};
        std::sregex_token_iterator next{_str.begin(), _str.end(), _rgx, -1};
        std::sregex_token_iterator last;

        while (next != last) {
            items.push_back(next->str());
            ++next;
        }

        auto filter = [](const std::string& __s) {
            return __s.empty();
        };
        auto junks{std::remove_if(items.begin(), items.end(), filter)};
        items.erase(junks, items.end());
        return items;
    }

    static auto join(const std::vector<std::string>& data, const std::string& delimiter) {
        std::stringstream result;

        auto size{data.size()};
        if (size-- > 0) {
            for (decltype(size) i{0}; i <= size; ++i) {
                result << data[i];
                if (i != size) {
                    result << delimiter;
                }
            }
        }
        return result.str();
    };

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
