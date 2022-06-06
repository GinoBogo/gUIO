
////////////////////////////////////////////////////////////////////////////////
/// \file      GPacket.cpp
/// \version   0.1
/// \date      January, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GPacket.hpp"

bool GPacket::IsValid(uint8_t* buffer, size_t bytes) {
    if (buffer != nullptr && bytes >= GPacket::PACKET_HEAD_SIZE && bytes <= GPacket::PACKET_FULL_SIZE) {
        auto* packet  = (packet_t*)buffer;
        auto  check_1 = bytes == GPacket::PACKET_HEAD_SIZE + packet->head.data_length;
        auto  check_2 = packet->head.current_segment <= packet->head.total_segments;

        return check_1 && check_2;
    }
    return false;
}

bool GPacket::IsSingle(packet_t* packet) {
    auto check_1 = packet->head.file_id == 0;
    auto check_2 = packet->head.current_segment == 1;
    auto check_3 = packet->head.total_segments == 1;

    return check_1 && check_2 && check_3;
}

bool GPacket::IsShort(packet_t* packet) {
    return packet->head.data_length == 0;
}

bool GPacket::IsFirst(packet_t* packet) {
    auto check_1 = packet->head.file_id != 0;
    auto check_2 = packet->head.current_segment == 1;
    auto check_3 = packet->head.total_segments > 1;

    return check_1 && check_2 && check_3;
}

bool GPacket::IsMiddle(packet_t* packet) {
    auto check_1 = packet->head.file_id != 0;
    auto check_2 = packet->head.current_segment > 1;
    auto check_3 = packet->head.current_segment < packet->head.total_segments;

    return check_1 && check_2 && check_3;
}

bool GPacket::IsLast(packet_t* packet) {
    auto check_1 = packet->head.file_id != 0;
    auto check_2 = packet->head.current_segment > 1;
    auto check_3 = packet->head.current_segment == packet->head.total_segments;

    return check_1 && check_2 && check_3;
}
