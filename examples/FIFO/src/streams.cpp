
////////////////////////////////////////////////////////////////////////////////
/// \file      streams.cpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "streams.hpp"

#include "GString.hpp"

#include <filesystem> // path
#include <fstream>    // ifstream, ofstream

struct decoder_args_t {
    g_array_t*      array  = nullptr;
    g_udp_client_t* client = nullptr;
    g_udp_server_t* server = nullptr;
};

GDecoder* stream_decoder{nullptr};
GEncoder* stream_encoder{nullptr};

static bool decode_short_msg(std::any data, std::any args) {
    auto* _packet = std::any_cast<packet_t*>(data);
    auto  _args   = std::any_cast<decoder_args_t>(args);
    auto* _client = _args.client;

    const auto _packet_type{_packet->head.packet_type};

    switch (_packet_type) {
        case packet_type_t::wake_up_query: {
            _packet->head.packet_type = packet_type_t::wake_up_reply;
            _client->Send(_packet, GPacket::PACKET_HEAD_SIZE);
            LOG_FORMAT(info, "WAKE_UP packet sent (%s)", __func__);
            return true;
        }

        case packet_type_t::signal_start_flow: {
            LOG_FORMAT(warning, "START_FLOW packet ignored (%s)", __func__);
            return true;
        }

        case packet_type_t::signal_stop_flow: {
            LOG_FORMAT(warning, "STOP_FLOW packet ignored (%s)", __func__);
            return true;
        }

        case packet_type_t::signal_reset_all: {
            Global::reset_all();
            LOG_FORMAT(info, "RESET_ALL packet received (%s)", __func__);
            return true;
        }

        case packet_type_t::signal_quit_deamon: {
            Global::quit_deamon();
            LOG_FORMAT(info, "QUIT_DEAMON packet received (%s)", __func__);
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
    auto* _array   = std::any_cast<g_array_t*>(_args.array);

    const auto _packet_type{_message->head()->packet_type};

    if (_packet_type == TX_STREAM_TYPE) {
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

bool stream_reader_for_tx_words(g_array_t* array, g_udp_client_t* client, g_udp_server_t* server) {
    // SECTION: UDP streaming

    if (TX_FILE_NAME.empty()) {
        if (stream_decoder == nullptr) {
            stream_decoder = new GDecoder(decode_short_msg, decode_large_msg);
        }

        auto _is_ready_msg = false;
        auto _is_large_msg = false;
        auto _error        = false;
        auto _bytes        = 0UL;

        stream_decoder->SetArgs(decoder_args_t{array, client, server});

        while (!_is_ready_msg) {
            BREAK_IF(_error, _error = !server->Receive(stream_decoder->packet_ptr(), &_bytes));

            BREAK_IF(_error, _error = _bytes < GPacket::PACKET_HEAD_SIZE);

            BREAK_IF(_error, _error = !stream_decoder->Process(&_is_ready_msg, &_is_large_msg));
        }

        return !_error;
    }

    // SECTION: FILE streaming

    std::ifstream ifs;
    ifs.open(TX_FILE_NAME, std::ios::binary);

    if (ifs.is_open()) {
        ifs.seekg(0, std::ifstream::end);
        auto bytes{ifs.tellg()};
        ifs.seekg(0, std::ifstream::beg);

        auto words{array->size()};
        words = bytes > 0 ? std::min(words, (size_t)bytes / FIFO_WORD_SIZE) : 0;
        array->used(words);

        auto* __s{array->data_bytes()};
        auto  __n{array->used_bytes()};
        ifs.read((char*)__s, (std::streamsize)__n);
        ifs.close();
        return true;
    }
    return false;
}

bool stream_writer_for_rx_words(g_array_t* array, g_udp_client_t* client, g_udp_server_t* server) {
    // SECTION: UDP streaming

    if (RX_FILE_NAME.empty()) {
        if (stream_encoder == nullptr) {
            stream_encoder = new GEncoder(RX_STREAM_ID);
        }

        auto _line  = 0;
        auto _error = false;

        if (stream_encoder->Process(RX_STREAM_TYPE, array->data_bytes(), array->used_bytes())) {
            packet_t packet;

            while (!stream_encoder->IsEmpty() && !_error) {
                auto* src_buffer = packet.ptr();
                auto  src_bytes  = stream_encoder->Pop(src_buffer, sizeof(packet));

                _error = src_bytes < 0;
                GOTO_IF(_error, _exit_label, _line = __LINE__);

                client->Send(src_buffer, (unsigned)src_bytes);
            }
            return !_error;
        }
_exit_label:
        LOG_IF(_line != 0, error, "FAILURE @ LINE %d (%s)", _line, __func__);
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

    std::ofstream ofs;
    ofs.open(_name, std::ios::binary);

    if (ofs.is_open()) {
        auto* __s{array->data_bytes()};
        auto  __n{array->used_bytes()};
        ofs.write((char*)__s, (std::streamsize)__n);
        ofs.close();
        return true;
    }
    return false;
}

// packet_type
// spare_0
// spare_1
// spare_2
// packet_counter
// data_length
// file_id
// total_segments
// current_segment

void evaluate_stream_reader_start(g_array_roller_t* roller, g_udp_client_t* client) {
    RETURN_IF(!roller->IsLevelChanged(&new_fsm_level), g_array_roller_t::fsm_levels_t new_fsm_level);
    RETURN_IF(new_fsm_level != g_array_roller_t::MIN_LEVEL_PASSED);

    static packet_head_t packet{packet_type_t::signal_start_flow, 0, 0, 0, 0, 0, 0, 1, 1};
    client->Send(&packet, GPacket::PACKET_HEAD_SIZE);
    packet.packet_counter++;
    LOG_FORMAT(info, "START_FLOW message sent (%s)", __func__);
}

void evaluate_stream_reader_stop(g_array_roller_t* roller, g_udp_client_t* client) {
    RETURN_IF(!roller->IsLevelChanged(&new_fsm_level), g_array_roller_t::fsm_levels_t new_fsm_level);
    RETURN_IF(new_fsm_level != g_array_roller_t::MAX_LEVEL_PASSED);

    static packet_head_t packet{packet_type_t::signal_stop_flow, 0, 0, 0, 0, 0, 0, 1, 1};
    client->Send(&packet, GPacket::PACKET_HEAD_SIZE);
    packet.packet_counter++;
    LOG_FORMAT(info, "STOP_FLOW message sent (%s)", __func__);
}