
////////////////////////////////////////////////////////////////////////////////
/// \file      GPacket.cpp
/// \version   0.1
/// \date      January, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GPacket.hpp"

bool GPacket::IsValid(uint8_t *buffer, size_t bytes) {
    if (buffer != nullptr && bytes >= GPacket::PACKET_HEAD_SIZE && bytes <= GPacket::PACKET_FULL_SIZE) {
        auto *packet  = (TPacket *)buffer;
        auto  check_1 = bytes == GPacket::PACKET_HEAD_SIZE + packet->head.data_length;
        auto  check_2 = packet->head.current_segment <= packet->head.total_segments;

        return check_1 && check_2;
    }
    return false;
}

bool GPacket::IsSingle(TPacket *packet) {
    auto check_1 = packet->head.file_id == 0;
    auto check_2 = packet->head.current_segment == 1;
    auto check_3 = packet->head.total_segments == 1;

    return check_1 && check_2 && check_3;
}

bool GPacket::IsShort(TPacket *packet) {
    return packet->head.data_length == 0;
}

bool GPacket::IsFirst(TPacket *packet) {
    auto check_1 = packet->head.file_id != 0;
    auto check_2 = packet->head.current_segment == 1;
    auto check_3 = packet->head.total_segments > 1;

    return check_1 && check_2 && check_3;
}

bool GPacket::IsMiddle(TPacket *packet) {
    auto check_1 = packet->head.file_id != 0;
    auto check_2 = packet->head.current_segment > 1;
    auto check_3 = packet->head.current_segment < packet->head.total_segments;

    return check_1 && check_2 && check_3;
}

bool GPacket::IsLast(TPacket *packet) {
    auto check_1 = packet->head.file_id != 0;
    auto check_2 = packet->head.current_segment > 1;
    auto check_3 = packet->head.current_segment == packet->head.total_segments;

    return check_1 && check_2 && check_3;
}
