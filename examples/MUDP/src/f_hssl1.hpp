
////////////////////////////////////////////////////////////////////////////////
/// \file      f_hssl1.hpp
/// \version   0.1
/// \date      February, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef F_HSSL1_HPP
#define F_HSSL1_HPP

#include "GLogger.hpp"
#include "GMessage.hpp"
#include "GUdpClient.hpp"

#include <any>

namespace f_hssl1 {

    typedef struct {
        GUdpClient* client;

    } WorkerArgs;

    bool decode_packet(std::any data, std::any args) {
        auto* _packet  = std::any_cast<packet_t*>(data);
        auto  _args    = std::any_cast<WorkerArgs>(args);
        auto* _client  = _args.client;
        auto  _type_id = _packet->head.packet_type;

        switch (_type_id) {

            case packet_type_t::wake_up_query: {
                _packet->head.packet_type = packet_type_t::wake_up_reply;
                _client->Send(_packet, GPacket::PACKET_HEAD_SIZE);
                LOG_FORMAT(info, "%s message: WAKE_UP (%s)", _client->TagName(), __func__);
            } break;

            default: {
                LOG_FORMAT(warning, "Invalid packet type [%d] (%s)", _type_id, __func__);
            } break;
        };

        return false;
    }

    bool decode_message(std::any data, std::any args) {
        auto* _message = std::any_cast<GMessage*>(data);
        auto  _args    = std::any_cast<WorkerArgs>(args);
        auto* _client  = _args.client;
        auto  _type_id = _message->head()->packet_type;

        switch (_type_id) {

            case packet_type_t::packet_from_hssl_1: {
                packet_t _packet;
                memcpy(&_packet.head, _message->head(), GPacket::PACKET_HEAD_SIZE);
                memcpy(&_packet.data, _message->data(), _message->used());

                _packet.head.packet_type = packet_type_t::packet_to_hssl_1;

                _client->Send(&_packet, GPacket::PACKET_HEAD_SIZE + _packet.head.data_length);
            } break;

            default: {
                LOG_FORMAT(warning, "Invalid message type [%d] (%s)", _type_id, __func__);
            } break;
        }

        return false;
    }

} // namespace f_hssl1

#endif // F_HSSL1_HPP
