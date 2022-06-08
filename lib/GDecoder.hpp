
////////////////////////////////////////////////////////////////////////////////
/// \file      GDecoder.hpp
/// \version   0.1
/// \date      Jan, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GDECODER_HPP
#define GDECODER_HPP

#include "GLogger.hpp"
#include "GMessage.hpp"

#include <any>

class GDecoder {
    public:
    typedef bool (*WorkerFunc)(std::any data, std::any args);

    GDecoder(WorkerFunc decode_short_msg, WorkerFunc decode_large_msg) {
        m_decode_short_msg = decode_short_msg;
        m_decode_large_msg = decode_large_msg;
    }

    GDecoder(WorkerFunc decode_short_msg, WorkerFunc decode_large_msg, std::any args) {
        m_decode_short_msg = decode_short_msg;
        m_decode_large_msg = decode_large_msg;
        m_args             = std::move(args);
    }

    void SetArgs(std::any args) {
        m_args = std::move(args);
    }

    bool Process(bool* is_ready = nullptr) {
        auto set_ready = [&](bool value) {
            if (is_ready != nullptr) {
                *is_ready = value;
            }
        };

        set_ready(false);
        if (GPacket::IsSingle(&packet)) {
            if (GPacket::IsShort(&packet)) {
                set_ready(true);
                return m_decode_short_msg(&packet, m_args);
            }

            message.Initialize(&packet);
            message.Append(&packet);
            if (message.IsValid()) {
                set_ready(true);
                return m_decode_large_msg(&message, m_args);
            }

            return false;
        }

        if (GPacket::IsFirst(&packet)) {
            message.Initialize(&packet);
            message.Append(&packet);
            return true;
        }

        if (GPacket::IsMiddle(&packet)) {
            message.Append(&packet);
            return true;
        }

        if (GPacket::IsLast(&packet)) {
            message.Append(&packet);
            if (message.IsValid()) {
                set_ready(true);
                return m_decode_large_msg(&message, m_args);
            }
        }

        return false;
    }

    packet_t packet;
    GMessage message = GMessage();

    inline auto packet_ptr() {
        return (uint8_t*)&packet;
    }

    inline auto packet_len() {
        return sizeof(packet);
    }

    private:
    std::any   m_args;
    WorkerFunc m_decode_short_msg;
    WorkerFunc m_decode_large_msg;
};

#endif // GDECODER_HPP
