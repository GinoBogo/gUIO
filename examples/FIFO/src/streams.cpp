
////////////////////////////////////////////////////////////////////////////////
/// \file      streams.cpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "streams.hpp"

#include "GDecoder.hpp"
#include "GDefine.hpp"
#include "GEncoder.hpp"
#include "GLogger.hpp"
#include "GString.hpp"
#include "globals.hpp"

#include <cstdint>    // uint16_t
#include <filesystem> // path
#include <fstream>    // ifstream, ofstream

typedef GArray<uint16_t>* array_ptr_t;

struct decoder_args_t {
    array_ptr_t array    = nullptr;
    GUdpClient* client   = nullptr;
    GUdpServer* server   = nullptr;
    bool*       is_large = nullptr;
};

static bool decode_short_msg(std::any data, std::any args) {
    auto* _packet = std::any_cast<packet_t*>(data);
    auto  _args   = std::any_cast<decoder_args_t>(args);
    auto* _client = _args.client;

    *_args.is_large = false;

    const auto _packet_type{_packet->head.packet_type};

    switch (_packet_type) {
        case packet_type_t::wake_up_query: {
            _packet->head.packet_type = packet_type_t::wake_up_reply;
            _client->Send(_packet, GPacket::PACKET_HEAD_SIZE);
            LOG_FORMAT(info, "WAKE_UP message (%s)", __func__);
            return true;
        }

        case packet_type_t::signal_start_flow: {
            LOG_FORMAT(warning, "START_FLOW message ignored (%s)", __func__);
            return true;
        }

        case packet_type_t::signal_stop_flow: {
            LOG_FORMAT(warning, "STOP_FLOW message ignored (%s)", __func__);
            return true;
        }

        default:
            break;
    }

    LOG_FORMAT(error, "Invalid packet type: %d (%s)", _packet_type, __func__);
    return false;
}

static bool decode_large_msg(std::any data, std::any args) {
    auto* _message = std::any_cast<GMessage*>(data);
    auto  _args    = std::any_cast<decoder_args_t>(args);
    auto* _array   = std::any_cast<array_ptr_t>(_args.array);

    *_args.is_large = true;

    const auto _packet_type{_message->head()->packet_type};

    if (_packet_type == RX_STREAM_TYPE) {
        auto* src_data = _message->data();
        auto  src_used = _message->used();
        memcpy(_array->data_bytes(), src_data, src_used);

        auto words_num = src_used / FIFO_WORD_SIZE;
        auto words_rem = src_used % FIFO_WORD_SIZE;

        return _array->used(words_num) && (words_rem == 0);
    }

    LOG_FORMAT(error, "Invalid packet type: %d (%s)", _packet_type, __func__);
    return false;
}

bool stream_reader_for_tx_words(GArray<uint16_t>* array, GUdpClient* client, GUdpServer* server) {
    // SECTION: UDP streaming

    if (TX_FILE_NAME.empty()) {
        auto _is_ready = false;
        auto _is_large = false;
        auto _error    = false;
        auto _bytes    = 0UL;

        static auto decoder_args{decoder_args_t{array, client, server, &_is_large}};
        static auto decoder{GDecoder(decode_short_msg, decode_large_msg, std::any(decoder_args))};

        while (!_is_ready || !_is_large) {
            BREAK_IF(_error, _error = !server->Receive(decoder.packet_ptr(), &_bytes));

            BREAK_IF(_error, _error = _bytes < GPacket::PACKET_HEAD_SIZE);

            BREAK_IF(_error, _error = !decoder.Process(&_is_ready));
        }

        return !_error;
    }

    // SECTION: FILE streaming

    std::ifstream fs;
    fs.open(TX_FILE_NAME, std::ios::binary);

    if (fs.is_open()) {
        fs.seekg(0, std::ifstream::end);
        auto bytes{fs.tellg()};
        fs.seekg(0, std::ifstream::beg);

        auto words{array->size()};
        words = bytes > 0 ? std::min(words, (size_t)bytes / FIFO_WORD_SIZE) : 0;
        array->used(words);

        auto* __s{array->data_bytes()};
        auto  __n{array->used_bytes()};
        fs.read((char*)__s, (std::streamsize)__n);
        fs.close();
        return true;
    }
    return false;
}

bool stream_writer_for_rx_words(GArray<uint16_t>* array, GUdpClient* client, GUdpServer* server) {
    // SECTION: UDP streaming

    if (RX_FILE_NAME.empty()) {
        static auto encoder{GEncoder(RX_STREAM_ID)};

        if (encoder.Process(RX_STREAM_TYPE, array->data_bytes(), array->used_bytes())) {
            packet_t packet;

            auto _error{false};
            while (!encoder.IsEmpty() && !_error) {
                auto* src_buffer = packet.ptr();
                auto  src_bytes  = encoder.Pop(src_buffer, sizeof(packet));

                BREAK_IF(_error, _error = src_bytes < 0);

                _error = !client->Send(src_buffer, (unsigned)src_bytes);
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
        auto* __s{array->data_bytes()};
        auto  __n{array->used_bytes()};
        fs.write((char*)__s, (std::streamsize)__n);
        fs.close();
        return true;
    }
    return false;
}
