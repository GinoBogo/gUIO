////////////////////////////////////////////////////////////////////////////////
/// \file      GPacket.hpp
/// \version   0.1
/// \date      January, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GPACKET_HPP
#define GPACKET_HPP

#include <cstdint>  // uint8_t, uint16_t, uint32_t
#include <stddef.h> // size_t

namespace GPacket {
    static const auto MAX_DATA_WORDS = 4091;
    static const auto MAX_DATA_BYTES = MAX_DATA_WORDS * 4;
} // namespace GPacket

typedef struct {
    uint8_t  spare_0;
    uint8_t  spare_1;
    uint8_t  spare_2;
    uint8_t  packet_type;
    uint32_t packet_counter;
    uint32_t data_length;
    uint32_t file_id;
    uint16_t current_segment;
    uint16_t total_segments;

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
    wake_up_query,
    wake_up_reply,
    packet_stop_flow,
    packet_start_flow,
    packet_to_spacewire,
    packet_from_spacewire,
    canbus_add_event,
    canbus_remove_event,
    canbus_configure_interface,
    canbus_event_reply,
    packet_from_hssl_1,
    packet_to_hssl_1,
    packet_from_hssl_2,
    packet_to_hssl_2,
    packet_from_multi_io,
    packet_to_multi_io,
    packet_from_power_supply,
    packet_to_power_supply,
    packet_quit = 0xFF

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
