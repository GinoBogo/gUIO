////////////////////////////////////////////////////////////////////////////////
/// \file      f_gm_mc.hpp
/// \version   0.1
/// \date      Jan, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef F_GM_MC_HPP
#define F_GM_MC_HPP

#include "GMessage.hpp"
#include "GUdpClient.hpp"

#include <any>

namespace f_gm_mc {

    typedef struct {
        bool *      quit;
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

            case TPacketType::packet_quit: {
                *_args.quit = true;
            } break;

            default: {
                LOG_FORMAT(warning, "Invalid packet type [%d]", _packet->head.packet_type);
            } break;
        };

        return false;
    }

    bool decode_message(std::any data, std::any args) {
        // auto _args = std::any_cast<WorkerArgs>(args);

        return false;
    }

} // namespace f_gm_mc

#endif // F_GM_MC_HPP