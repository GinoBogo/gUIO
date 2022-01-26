////////////////////////////////////////////////////////////////////////////////
/// \file      f_gm_mc.hpp
/// \version   0.1
/// \date      Jan, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef F_GM_MC_HPP_
#define F_GM_MC_HPP_

#include "GMessage.hpp"
#include "GUdpClient.hpp"

#include <any>

namespace f_gm_mc {

    bool decode_packet(std::any data, std::any args) {
        auto packet = std::any_cast<TPacket *>(data);
        auto client = std::any_cast<GUdpClient *>(args);

        return false;
    }

    bool decode_message(std::any data, std::any args) {
        auto message = std::any_cast<GMessage *>(data);
        auto client  = std::any_cast<GUdpClient *>(args);

        return false;
    }

} // namespace f_gm_mc

#endif /* F_GM_MC_HPP_ */