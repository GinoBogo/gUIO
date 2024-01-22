
////////////////////////////////////////////////////////////////////////////////
/// \file      GDecoder.hpp
/// \version   0.1
/// \date      January, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GDECODER_HPP
#define GDECODER_HPP

#include "GMessage.hpp" // GMessage

#include <any>     // any
#include <utility> // move

class GDecoder {
  public:
    typedef bool (*WorkerFunc)(std::any data, std::any args);

    GDecoder() {
        SetWorkerFunc(nullptr, nullptr);
        SetArgs(0);
    }

    GDecoder(WorkerFunc decode_short_msg, WorkerFunc decode_large_msg) {
        SetWorkerFunc(decode_short_msg, decode_large_msg);
        SetArgs(0);
    }

    GDecoder(WorkerFunc decode_short_msg, WorkerFunc decode_large_msg, std::any args) {
        SetWorkerFunc(decode_short_msg, decode_large_msg);
        SetArgs(std::move(args));
    }

    void SetWorkerFunc(WorkerFunc decode_short_msg, WorkerFunc decode_large_msg) {
        memset(&packet, 0, GPacket::PACKET_FULL_SIZE);
        m_decode_short_msg = decode_short_msg;
        m_decode_large_msg = decode_large_msg;
    }

    void SetArgs(std::any args) {
        m_args = std::move(args);
    }

    bool Process(bool* is_ready = nullptr, bool* is_large = nullptr) {
        auto set_ready = [&](bool value) {
            if (is_ready != nullptr) {
                *is_ready = value;
            }
        };

        auto set_large = [&](bool value) {
            if (is_large != nullptr) {
                *is_large = value;
            }
        };

        set_ready(false);
        set_large(false);

        if (GPacket::IsSingle(&packet)) {
            if (GPacket::IsShort(&packet)) {
                set_ready(true);
                return m_decode_short_msg(&packet, m_args);
            }

            message.Initialize(&packet);
            message.Append(&packet);
            if (message.IsValid()) {
                set_ready(true);
                set_large(true);
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
                set_large(true);
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
