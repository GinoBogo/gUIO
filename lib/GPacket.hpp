
////////////////////////////////////////////////////////////////////////////////
/// \file      GPacket.hpp
/// \version   0.1
/// \date      January, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GPACKET_HPP
#define GPACKET_HPP

#include <cstddef> // size_t
#include <cstdint> // uint8_t, uint16_t, uint32_t

namespace GPacket {
    static const auto MAX_DATA_WORDS = 4091;
    static const auto MAX_DATA_BYTES = MAX_DATA_WORDS * 4;
} // namespace GPacket

typedef struct packet_head_t {
    uint8_t  packet_type;
    uint8_t  spare_0;
    uint8_t  spare_1;
    uint8_t  spare_2;
    uint32_t packet_counter;
    uint32_t data_length;
    uint32_t file_id;
    uint16_t total_segments;
    uint16_t current_segment;

    auto* ptr() {
        return reinterpret_cast<uint8_t*>(this);
    }
    static auto len() {
        return sizeof(packet_head_t);
    }

} packet_head_t;

typedef union packet_data_t {
    uint8_t  bytes[GPacket::MAX_DATA_BYTES];
    uint32_t words[GPacket::MAX_DATA_WORDS];

} packet_data_t;

typedef struct packet_t {
    packet_head_t head;
    packet_data_t data;

    auto* ptr() {
        return reinterpret_cast<uint8_t*>(this);
    }
    auto len() {
        return sizeof(head) + head.data_length;
    }

} packet_t;

typedef enum packet_type_t {
    wake_up_query      = 0,
    wake_up_reply      = 1,
    signal_stop_flow   = 2,
    signal_start_flow  = 3,
    packet_to_hssl_1   = 13,
    packet_from_hssl_1 = 14,
    packet_to_hssl_2   = 15,
    packet_from_hssl_2 = 16,
    packet_to_gm_dh    = 17,
    packet_from_gm_dh  = 18,
    packet_to_gm_mc    = 19,
    packet_from_gm_mc  = 20,
    signal_reset_all   = 254,
    signal_quit_deamon = 255

} packet_type_t;

namespace GPacket {
    const auto PACKET_HEAD_SIZE = sizeof(packet_head_t);
    const auto PACKET_DATA_SIZE = sizeof(packet_data_t);
    const auto PACKET_FULL_SIZE = sizeof(packet_t);

    bool IsValid(uint8_t* buffer, size_t bytes);
    bool IsSingle(packet_t* packet);
    bool IsShort(packet_t* packet);
    bool IsFirst(packet_t* packet);
    bool IsMiddle(packet_t* packet);
    bool IsLast(packet_t* packet);
} // namespace GPacket

#endif // GPACKET_HPP
