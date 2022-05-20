
////////////////////////////////////////////////////////////////////////////////
/// \file      f_gm_mc.hpp
/// \version   0.1
/// \date      January, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef F_GM_MC_HPP
#define F_GM_MC_HPP

#include "GLogger.hpp"
#include "GMessage.hpp"
#include "GUdpClient.hpp"

#include <any>

namespace f_gm_mc {

    typedef struct {
        bool*       quit;
        GUdpClient* client;

    } WorkerArgs;

    bool decode_packet(std::any data, std::any args) {
        auto* _packet  = std::any_cast<TPacket*>(data);
        auto  _args    = std::any_cast<WorkerArgs>(args);
        auto* _client  = _args.client;
        auto  _type_id = _packet->head.packet_type;

        switch (_type_id) {

            case TPacketType::wake_up_query: {
                _packet->head.packet_type = TPacketType::wake_up_reply;
                _client->Send(_packet, GPacket::PACKET_HEAD_SIZE);
                LOG_FORMAT(info, "%s message: WAKE_UP (%s)", _client->TagName(), __func__);
            } break;

            case TPacketType::quit_process: {
                *_args.quit = true;
                LOG_FORMAT(info, "%s message: QUIT (%s)", _client->TagName(), __func__);
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
            default: {
                LOG_FORMAT(warning, "Invalid message type [%d] (%s)", _type_id, __func__);
            } break;
        }

        return false;
    }

} // namespace f_gm_mc

#endif // F_GM_MC_HPP
