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
        bool       *quit;
        GUdpClient *client;

    } WorkerArgs;

    bool decode_packet(std::any data, std::any args) {
        auto _packet = std::any_cast<TPacket *>(data);
        auto _args   = std::any_cast<WorkerArgs>(args);

        if (_packet->head.packet_type == 0xFF) {
            *_args.quit = true;
        }

        return false;
    }

    bool decode_message(std::any data, std::any args) {
        // auto _args = std::any_cast<WorkerArgs>(args);

        return false;
    }

} // namespace f_gm_mc

#endif // F_GM_MC_HPP