////////////////////////////////////////////////////////////////////////////////
/// \file      f_hssl0.hpp
/// \version   0.1
/// \date      February, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef F_HSSL0_HPP
#define F_HSSL0_HPP

#include "GLogger.hpp"
#include "GMessage.hpp"
#include "GUdpClient.hpp"

#include <any>

namespace f_hssl0 {

    typedef struct {
        GUdpClient *client;

    } WorkerArgs;

    bool decode_packet(std::any data, std::any args) {
        auto _packet = std::any_cast<TPacket *>(data);
        auto _args   = std::any_cast<WorkerArgs>(args);
        auto _client = _args.client;

        switch (_packet->head.packet_type) {

            case TPacketType::wake_up_query: {
                _packet->head.packet_type = TPacketType::wake_up_reply;
                _client->Send(_packet, GPacket::PACKET_HEAD_SIZE);
            } break;

            default: {
                LOG_FORMAT(warning, "Invalid packet type [%d]", _packet->head.packet_type);
            } break;
        };

        return false;
    }

    bool decode_message(std::any data, std::any args) {
        auto _message = std::any_cast<GMessage *>(data);
        auto _args    = std::any_cast<WorkerArgs>(args);
        auto _client  = _args.client;

        switch (_message->packet_type()) {
            default: {
                LOG_FORMAT(warning, "Invalid message type [%d]", _message->packet_type());
            } break;
        }

        return false;
    }

} // namespace f_hssl0

#endif // F_HSSL0_HPP
