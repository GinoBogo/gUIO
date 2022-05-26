////////////////////////////////////////////////////////////////////////////////
/// \file      streams.cpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "streams.hpp"

#include "GLogger.hpp"
#include "GString.hpp"

#include <cstdint>    // uint16_t
#include <filesystem> // path
#include <fstream>    // ifstream, ofstream

void reader_for_tx_words(GArray<uint16_t>& array, const std::string& filename) {
    auto words{array.size()};

    if (!filename.empty()) {
        std::ifstream fs;
        fs.open(filename, std::ios::binary);

        if (fs.is_open()) {
            fs.seekg(0, std::ifstream::end);
            auto bytes{fs.tellg()};
            fs.seekg(0, std::ifstream::beg);

            words = bytes > 0 ? std::min(words, (size_t)bytes / sizeof(uint16_t)) : 0;

            auto* __s{reinterpret_cast<char*>(array.data())};
            auto  __n{static_cast<std::streamsize>(words)};
            fs.read(__s, __n);

            // std::string line;

            // for (decltype(words) i{0}; i < words; ++i) {
            //     uint16_t word{0x0000};

            //     std::getline(fs, line);
            //     word |= GString::strtous(line) << 8;
            //     std::getline(fs, line);
            //     word |= GString::strtous(line) << 0;

            //     array.data()[i] = word;
            // }

            fs.close();

            array.used(words);
            LOG_FORMAT(info, "Words from \"%s\" file (%s)", filename.c_str(), __func__);
            return;
        }
    }

    // SECTION: internal-ramp generator
    for (decltype(words) i{0}; i < words; ++i) {
        array.data()[i] = (uint16_t)(1 + i);
    }

    array.used(words);
    LOG_FORMAT(info, "Words from internal-ramp generator (%s)", __func__);
}

int p_num{0}; // progressive number
int p_val{0}; // previous byte value

void writer_for_rx_words(GArray<uint16_t>& array, const std::string& filename) {
    auto words{array.used()};

    if (!filename.empty()) {
        auto _path{std::filesystem::path(filename)};

        char _tail[32];
        snprintf(_tail, sizeof(_tail), "_%06d%s", p_num++, _path.extension().c_str());

        auto _name{_path.parent_path()};
        _name /= _path.stem();
        _name += _tail;

        std::ofstream fs;
        fs.open(_name, std::ios::binary);

        if (fs.is_open()) {
            auto* __s{reinterpret_cast<char*>(array.data())};
            auto  __n{static_cast<std::streamsize>(words * sizeof(uint16_t))};
            fs.write(__s, __n);

            // char h_line[16];
            // char l_line[16];

            // for (decltype(words) i{0}; i < words; ++i) {
            //     auto word = array.data()[i];

            //     auto h_val{(0xFF00 & word) >> 8};
            //     auto l_val{(0x00FF & word) >> 0};

            //     sprintf(h_line, "0x%02X\n", h_val);
            //     sprintf(l_line, "0x%02X\n", l_val);

            //     fs.write(h_line, (signed)strlen(h_line));
            //     fs.write(l_line, (signed)strlen(l_line));
            // }

            fs.close();
            return;
        }
    }

    // SECTION: internal-ramp generator
    for (decltype(words) i{0}; i < words; ++i) {
        auto word = array.data()[i];

        auto h_val{(0xFF00 & word) >> 8};
        auto l_val{(0x00FF & word) >> 0};

        auto h_dif{h_val - p_val};
        auto l_dif{l_val - h_val};

        auto check_1{(h_val == 0) && (l_val == 247)};
        auto check_2{(p_val == 0) && (h_val == 247)};

        if (check_1 || check_2) {
            p_num = 0;
        }

        p_val = l_val;

        LOG_FORMAT(debug, " %4d | %3d | %+4d", p_num++, h_val, h_dif);
        LOG_FORMAT(debug, " %4d | %3d | %+4d", p_num++, l_val, l_dif);
    }
}
