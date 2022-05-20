
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

typedef struct {
    uint8_t  packet_type;
    uint8_t  spare_0;
    uint8_t  spare_1;
    uint8_t  spare_2;
    uint32_t packet_counter;
    uint32_t data_length;
    uint32_t file_id;
    uint16_t total_segments;
    uint16_t current_segment;

} TPacketHead;

typedef union {
    uint8_t  bytes[GPacket::MAX_DATA_BYTES];
    uint32_t words[GPacket::MAX_DATA_WORDS];

} TPacketData;

typedef struct {
    TPacketHead head;
    TPacketData data;

} TPacket;

typedef enum {
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
    quit_process       = 255

} TPacketType;

namespace GPacket {
    const auto PACKET_HEAD_SIZE = sizeof(TPacketHead);
    const auto PACKET_DATA_SIZE = sizeof(TPacketData);
    const auto PACKET_FULL_SIZE = sizeof(TPacket);

    bool IsValid(uint8_t *buffer, size_t bytes);
    bool IsSingle(TPacket *packet);
    bool IsShort(TPacket *packet);
    bool IsFirst(TPacket *packet);
    bool IsMiddle(TPacket *packet);
    bool IsLast(TPacket *packet);
} // namespace GPacket

#endif // GPACKET_HPP
