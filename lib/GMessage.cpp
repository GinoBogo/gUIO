////////////////////////////////////////////////////////////////////////////////
/// \file      GMessage.cpp
/// \version   0.1
/// \date      January, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GMessage.hpp"

#include "GLogger.hpp"

GMessage::GMessage(const uint32_t max_size) : GBuffer(max_size) {
    Clear();
}

void GMessage::Clear() {
    m_packet_counter = 0;
    m_errors_counter = 0;
    GBuffer::Clear();
}

void GMessage::Initialize(TPacket *packet) {
    m_no_error        = true;
    m_is_valid        = false;
    m_packet_type     = packet->head.packet_type;
    m_file_id         = packet->head.file_id;
    m_current_segment = 0;
    m_total_segments  = packet->head.total_segments;
    GBuffer::Reset();
}

bool GMessage::Append(TPacket *packet) {
    ++m_packet_counter;

    if (m_no_error) {
        auto check_1 = packet->head.packet_type == m_packet_type;
        auto check_2 = packet->head.file_id == m_file_id;
        auto check_3 = packet->head.current_segment == ++m_current_segment;
        m_no_error   = check_1 && check_2 && check_3;

        if (m_no_error) {
            auto src_data  = (uint8_t *)&packet->data;
            auto src_count = packet->head.data_length;
            return GBuffer::Append(src_data, src_count);
        }
    }

    ++m_errors_counter;
    return false;
}