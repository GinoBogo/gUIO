////////////////////////////////////////////////////////////////////////////////
/// \file      GPacket.hpp
/// \version   0.1
/// \date      Jan, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GPACKET_HPP_
#define GPACKET_HPP_

#include <cstdint> // uint8_t, uint16_t, uint32_t
#include <stddef.h>

static const auto MAX_DATA_WORDS = 4091;
static const auto MAX_DATA_BYTES = MAX_DATA_WORDS * 4;

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
    uint8_t  bytes[MAX_DATA_BYTES];
    uint32_t words[MAX_DATA_WORDS];

} TPacketData;

typedef struct {
    TPacketHead head;
    TPacketData data;

} TPacket;

static const auto PACKET_HEAD_SIZE = sizeof(TPacketHead);
static const auto PACKET_DATA_SIZE = sizeof(TPacketData);
static const auto PACKET_FULL_SIZE = sizeof(TPacket);

class GPacket {
    public:
    static bool IsValid(uint8_t *buffer, size_t bytes);
    static bool IsSingle(TPacket *packet);
    static bool IsFirst(TPacket *packet);
    static bool IsMiddle(TPacket *packet);
    static bool IsLast(TPacket *packet);
};

#endif /* GPACKET_HPP_ */