////////////////////////////////////////////////////////////////////////////////
/// \file      GDecoder.hpp
/// \version   0.1
/// \date      Jan, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GDECODER_HPP_
#define GDECODER_HPP_

#include "GMessage.hpp"

#include <any>

class GDecoder {
    public:
    typedef bool (*WorkerFunc)(std::any data, std::any args);

    GDecoder(WorkerFunc decode_packet, WorkerFunc decode_message, std::any args) {
        m_decode_packet  = decode_packet;
        m_decode_message = decode_message;
        m_args           = args;
    }

    void Process() {
        if (GPacket::IsSingle(&packet)) {
            if (GPacket::IsShort(&packet)) {
                DecodePacket();
            }
            else {
                message.Initialize(&packet);
                message.Append(&packet);
                if (message.IsValid()) {
                    DecodeMessage();
                }
            }
            return;
        }

        if (GPacket::IsFirst(&packet)) {
            message.Initialize(&packet);
            message.Append(&packet);
            return;
        }

        if (GPacket::IsMiddle(&packet)) {
            message.Append(&packet);
            return;
        }

        if (GPacket::IsLast(&packet)) {
            message.Append(&packet);
            if (message.IsValid()) {
                DecodeMessage();
            }
        }
    }

    TPacket  packet;
    GMessage message = GMessage();

    inline auto packet_ptr() {
        return (uint8_t *)&packet;
    }

    inline auto packet_len() {
        return sizeof(packet);
    }

    private:
    bool DecodePacket() {
        return m_decode_packet(&packet, m_args);
    }

    bool DecodeMessage() {
        return m_decode_message(&message, m_args);
    }

    std::any   m_args;
    WorkerFunc m_decode_packet;
    WorkerFunc m_decode_message;
};

#endif /* GDECODER_HPP_ */