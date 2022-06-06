
////////////////////////////////////////////////////////////////////////////////
/// \file      streams.cpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "streams.hpp"

#include "GEncoder.hpp"
#include "GLogger.hpp"
#include "GString.hpp"
#include "globals.hpp"

#include <cstdint>    // uint16_t
#include <filesystem> // path
#include <fstream>    // ifstream, ofstream

bool stream_reader_for_tx_words(GArray<uint16_t>& array, GUdpServer& server) {

    auto words{array.size()};

    if (!TX_FILE_NAME.empty()) {
        std::ifstream fs;
        fs.open(TX_FILE_NAME, std::ios::binary);

        if (fs.is_open()) {
            fs.seekg(0, std::ifstream::end);
            auto bytes{fs.tellg()};
            fs.seekg(0, std::ifstream::beg);

            words = bytes > 0 ? std::min(words, (size_t)bytes / sizeof(uint16_t)) : 0;

            auto* __s{reinterpret_cast<char*>(array.data())};
            auto  __n{static_cast<std::streamsize>(words)};
            fs.read(__s, __n);
            fs.close();

            array.used(words);
            LOG_FORMAT(info, "Words from \"%s\" file (%s)", TX_FILE_NAME.c_str(), __func__);
            return true;
        }
        return false;
    }

    // SECTION: internal-ramp generator
    for (decltype(words) i{0}; i < words; ++i) {
        array.data()[i] = (uint16_t)(1 + i);
    }

    array.used(words);
    LOG_FORMAT(info, "Words from internal-ramp generator (%s)", __func__);
    return true;
}

bool stream_writer_for_rx_words(GArray<uint16_t>& array, GUdpClient& client) {
    // SECTION: UDP streaming

    if (RX_FILE_NAME.empty()) {
        static auto encoder{GEncoder(RX_STREAM_ID)};

        if (encoder.Process(RX_STREAM_TYPE, array.data_bytes(), array.used_bytes())) {
            TPacket packet;

            auto _error{false};
            while (!encoder.IsEmpty() && !_error) {
                auto* src_buffer = packet.ptr();
                auto  src_bytes  = encoder.Pop(src_buffer, sizeof(packet));

                if (src_bytes > 0) {
                    _error |= !client.Send(src_buffer, (unsigned)src_bytes);
                }
            }
            return !_error;
        }
        return false;
    }

    // SECTION: FILE streaming

    static auto _index{0U};
    static auto _path{std::filesystem::path(RX_FILE_NAME)};

    char _tail[32];
    snprintf(_tail, sizeof(_tail), "_%06u%s", _index++, _path.extension().c_str());

    auto _name{_path.parent_path()};
    _name /= _path.stem();
    _name += _tail;

    std::ofstream fs;
    fs.open(_name, std::ios::binary);

    if (fs.is_open()) {
        auto* __s{reinterpret_cast<char*>(array.data())};
        auto  __n{static_cast<std::streamsize>(FIFO_WORD_SIZE * array.used())};
        fs.write(__s, __n);
        fs.close();
        return true;
    }
    return false;
}
