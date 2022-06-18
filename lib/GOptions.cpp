
////////////////////////////////////////////////////////////////////////////////
/// \file      GOptions.cpp
/// \version   0.2
/// \date      November, 2020
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GOptions.hpp"

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

auto sanitize = [](std::string& line) {
    auto remark = false;
    auto filter = [&remark](char __c) {
        remark |= (__c == '#') || (__c == ';');
        return remark || (bool)std::isspace(__c);
    };

    line.erase(std::remove_if(line.begin(), line.end(), filter), line.end());
};

auto split = [](const std::string& data, const std::string& regex) {
    std::vector<std::string> tokens;

    std::regex                 _rgx{regex};
    std::sregex_token_iterator next{data.begin(), data.end(), _rgx, -1};
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
};

auto join = [](const std::vector<std::string>& data, const std::string& delimiter) {
    std::stringstream result;

    auto size{data.size() - 1};
    for (decltype(size) i{0}; i <= size; ++i) {
        result << data[i];
        if (i != size) {
            result << delimiter;
        }
    }
    return result.str();
};

template <typename T, int B> auto expand_and_check(const std::string& value, T& type) {
    if (std::is_signed_v<T>) {
        auto _n1{std::strtoll(value.c_str(), nullptr, B)};
        auto _n2{static_cast<T>(_n1)};
        auto _n3{static_cast<long long>(_n2)};
        if (_n1 == _n3 && errno != ERANGE) {
            type = _n2;
            return true;
        }
    }
    else {
        auto _n1{std::strtoull(value.c_str(), nullptr, B)};
        auto _n2{static_cast<T>(_n1)};
        auto _n3{static_cast<unsigned long long>(_n2)};
        if (_n1 == _n3 && errno != ERANGE) {
            type = _n2;
            return true;
        }
    }
    return false;
}

template <typename T> auto try_string_to_boolean(const std::string& _value, T& _type) {

    static auto _filter_0 = std::regex(R"(^[0]|(\bfalse\b|\blow\b|\boff\b)$)", std::regex::icase);
    static auto _filter_1 = std::regex(R"(^[1]|(\btrue\b|\bhigh\b|\bon\b)$)", std::regex::icase);

    if (std::regex_match(_value, _filter_0)) {
        _type = false;
        return true;
    }
    if (std::regex_match(_value, _filter_1)) {
        _type = true;
        return true;
    }

    return false;
}

template <typename T> auto try_string_to_integer(const std::string& _value, T& _type) {

    static auto _filter = std::regex(R"(^([+-])?\d+((u|U)?(l|U)?(l|L)?)$)");

    if (std::regex_match(_value, _filter)) {
        if constexpr (std::is_same_v<T, char>) {
            return expand_and_check<T, 10>(_value, _type);
        }

        if constexpr (std::is_same_v<T, unsigned char>) {
            return expand_and_check<T, 10>(_value, _type);
        }

        if constexpr (std::is_same_v<T, short>) {
            return expand_and_check<T, 10>(_value, _type);
        }

        if constexpr (std::is_same_v<T, unsigned short>) {
            return expand_and_check<T, 10>(_value, _type);
        }

        if constexpr (std::is_same_v<T, int>) {
            return expand_and_check<T, 10>(_value, _type);
        }

        if constexpr (std::is_same_v<T, unsigned int>) {
            return expand_and_check<T, 10>(_value, _type);
        }

        if constexpr (std::is_same_v<T, long>) {
            return expand_and_check<T, 10>(_value, _type);
        }

        if constexpr (std::is_same_v<T, unsigned long>) {
            return expand_and_check<T, 10>(_value, _type);
        }

        if constexpr (std::is_same_v<T, long long>) {
            _type = std::strtoll(_value.c_str(), nullptr, 10);
            return true;
        }

        if constexpr (std::is_same_v<T, unsigned long long>) {
            _type = std::strtoull(_value.c_str(), nullptr, 10);
            return true;
        }
    }
    return false;
}

template <typename T> auto try_string_to_decimal(const std::string& _value, T& _type) {

    static auto _filter = std::regex(R"(^([+-])?((\d+\.\d*)|(\d*\.\d+))(((e)|(E))((-?)|(\+?))0?\d+|)$)");

    if (std::regex_match(_value, _filter)) {
        if constexpr (std::is_same_v<T, float>) {
            try {
                _type = std::stof(_value);
                return true;
            }
            catch (const std::invalid_argument&) {
            }
            catch (const std::out_of_range&) {
            };
        }

        if constexpr (std::is_same_v<T, double>) {
            try {
                _type = std::stod(_value);
                return true;
            }
            catch (const std::invalid_argument&) {
            }
            catch (const std::out_of_range&) {
            };
        }

        if constexpr (std::is_same_v<T, long double>) {
            try {
                _type = std::stold(_value);
                return true;
            }
            catch (const std::invalid_argument&) {
            }
            catch (const std::out_of_range&) {
            };
        }
    }
    return false;
}

template <typename T> auto try_string_to_hexadecimal(const std::string& _value, T& _type) {

    static auto _filter = std::regex(R"(^0(x|X)((\d?)|([a-f]?|[A-F]?))+$)");

    if (std::regex_match(_value, _filter)) {
        if constexpr (std::is_same_v<T, char>) {
            return expand_and_check<T, 16>(_value, _type);
        }

        if constexpr (std::is_same_v<T, unsigned char>) {
            return expand_and_check<T, 16>(_value, _type);
        }

        if constexpr (std::is_same_v<T, short>) {
            return expand_and_check<T, 16>(_value, _type);
        }

        if constexpr (std::is_same_v<T, unsigned short>) {
            return expand_and_check<T, 16>(_value, _type);
        }

        if constexpr (std::is_same_v<T, int>) {
            return expand_and_check<T, 16>(_value, _type);
        }

        if constexpr (std::is_same_v<T, unsigned int>) {
            return expand_and_check<T, 16>(_value, _type);
        }

        if constexpr (std::is_same_v<T, long>) {
            return expand_and_check<T, 16>(_value, _type);
        }

        if constexpr (std::is_same_v<T, unsigned long>) {
            return expand_and_check<T, 16>(_value, _type);
        }

        if constexpr (std::is_same_v<T, long long>) {
            _type = strtoll(_value.c_str(), nullptr, 16);
            return true;
        }

        if constexpr (std::is_same_v<T, unsigned long long>) {
            _type = std::strtoull(_value.c_str(), nullptr, 16);
            return true;
        }
    }
    return false;
}

template <typename T> auto try_string_to_text(const std::string& _value, T& _type) {
    if constexpr (std::is_same_v<T, std::string>) {
        try {
            if constexpr (!std::is_arithmetic_v<T>) {
                _type = _value;
                return true;
            }
        }
        catch (const std::invalid_argument&) {
        };
    }
    return false;
}

template <typename T> auto string_to_type(const std::string& value, bool& is_valid) {
    T type{};

    is_valid = try_string_to_boolean(value, type);
    if (is_valid) {
        return type;
    }

    is_valid = try_string_to_integer(value, type);
    if (is_valid) {
        return type;
    }

    is_valid = try_string_to_decimal(value, type);
    if (is_valid) {
        return type;
    }

    is_valid = try_string_to_hexadecimal(value, type);
    if (is_valid) {
        return type;
    }

    is_valid = try_string_to_text(value, type);

    return type;
}

static auto decode_type_then_push_pair(GOptions::Pairs& pairs, const std::string& label, const std::any& value) {
    if (value.type() == typeid(bool)) {
        GOptions::Pair pair{label, (std::any_cast<bool>(value)) ? "true" : "false"};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(char)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<char>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(unsigned char)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<unsigned char>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(short)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<short>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(unsigned short)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<unsigned short>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(int)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<int>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(unsigned int)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<unsigned int>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(long)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<long>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(unsigned long)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<unsigned long>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(long long)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<long long>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(unsigned long long)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<unsigned long long>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(float)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<float>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(double)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<double>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(long double)) {
        GOptions::Pair pair{label, std::to_string(std::any_cast<long double>(value))};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(std::string)) {
        GOptions::Pair pair{label, std::any_cast<std::string>(value)};
        pairs.push_back(pair);
        return;
    }
    if (value.type() == typeid(const char*)) {
        GOptions::Pair pair{label, std::string(std::any_cast<const char*>(value))};
        pairs.push_back(pair);
        return;
    }
}

GOptions::Pairs GOptions::ToPairs() {
    Pairs pairs{};

    for (const auto& _it : *this) {
        const auto& label{_it.first};
        const auto& value{_it.second};

        decode_type_then_push_pair(pairs, label, value);
    }

    return pairs;
}

GOptions::Sections GOptions::ToSections() {
    Sections sections{};

    auto pairs = ToPairs();
    for (const auto& pair : pairs) {
        auto tokens      = split(pair.label, "R([. \t])");
        auto tokens_size = tokens.size();
        if (tokens_size >= 1U) {
            auto                     index(static_cast<ssize_t>(tokens_size - 1));
            std::vector<std::string> upper(tokens.begin(), tokens.begin() + index);
            std::vector<std::string> lower(tokens.begin() + index, tokens.end());

            const auto title = join(upper, ".");
            const auto label = join(lower, ".");
            const auto _pair = Pair(label, pair.value);

            auto filter = [title](const Section& _sec) {
                return (_sec.title == title);
            };
            auto found = std::find_if(sections.begin(), sections.end(), filter);
            if (found == sections.end()) {
                auto _section = Section(title);
                _section.pairs.push_back(_pair);
                sections.push_back(_section);
            }
            else {
                found->pairs.push_back(_pair);
            }
        }
    }

    return sections;
}

static auto populate_sections(const std::string& filename, GOptions::Sections& sections) {
    std::ifstream stream(filename);
    std::string   line;

    while (std::getline(stream, line)) {
        sanitize(line);

        auto check_1{line.find('[')};
        auto check_2{line.rfind(']')};
        if (check_1 < check_2) {
            auto tokens = split(line, "[\\[\\]]");
            auto title  = tokens[0];

            auto filter = [title](const GOptions::Section& _sec) {
                return (_sec.title == title);
            };
            auto found = std::find_if(sections.begin(), sections.end(), filter);
            if (found == sections.end()) {
                auto section = GOptions::Section(title);
                sections.push_back(section);
            }
        }
        else {
            auto tokens = split(line, "[=\"]");
            switch (tokens.size()) {
                case 1: {
                    auto pair = GOptions::Pair(tokens[0], "");
                    auto last = --sections.end();
                    last->pairs.push_back(pair);
                } break;

                case 2: {
                    auto pair = GOptions::Pair(tokens[0], tokens[1]);
                    auto last = --sections.end();
                    last->pairs.push_back(pair);
                } break;

                default:
                    break;
            }
        }
    }
    stream.close();
}

static auto find_minimal_type(const std::string& _value, std::any& _type) {
    auto is_valid{false};

    _type = string_to_type<bool>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<char>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<unsigned char>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<short>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<unsigned short>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<int>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<unsigned int>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<long>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<unsigned long>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<long long>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<unsigned long long>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<float>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<double>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<long double>(_value, is_valid);
    if (is_valid) {
        return true;
    }
    _type = string_to_type<std::string>(_value, is_valid);

    return is_valid;
}

bool GOptions::Read(const std::string& filename) {
    const auto filepath{std::filesystem::path(filename)};

    if (std::filesystem::exists(filepath) && std::filesystem::is_regular_file(filepath)) {
        auto bytes{std::filesystem::file_size(filepath)};
        if (bytes > 0) {
            Sections sections;

            populate_sections(filename, sections);

            for (const auto& section : sections) {
                for (const auto& pair : section.pairs) {
                    std::any value;

                    if (find_minimal_type(pair.value, value)) {
                        auto label = section.title + "." + pair.label;
                        this->insert_or_assign(label, value);
                    }
                }
            }
            return true;
        }
    }
    return false;
}

bool GOptions::Write(const std::string& filename) {
    const auto filepath{std::filesystem::path(filename)};

    if (!std::filesystem::is_directory(filepath)) {
        auto stream   = std::ofstream(filename);
        auto sections = ToSections();

        for (const auto& section : sections) {
            stream << "[" << section.title << "]" << std::endl;
            for (const auto& pair : section.pairs) {
                stream << pair.label << " = " << pair.value << std::endl;
            }
            stream << std::endl;
        }
        stream.close();
        return true;
    }

    return false;
}

GOptions& GOptions::operator+=(const GOptions& options) {
    if (this != &options) {
        for (const auto& pair : options) {
            this->insert_or_assign(pair.first, pair.second);
        }
    }
    return *this;
}
