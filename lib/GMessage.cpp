
////////////////////////////////////////////////////////////////////////////////
/// \file      GMessage.cpp
/// \version   0.1
/// \date      January, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GMessage.hpp"

GMessage::GMessage(const uint32_t max_size) :
GBuffer(max_size) {
    Reset();
}

void GMessage::Reset() {
    m_is_first       = true;
    m_packet_counter = 0;
    m_missed_counter = 0;
    m_errors_counter = 0;
    GBuffer::Reset();
}

void GMessage::Initialize(packet_t* packet) {
    m_no_error = true;
    m_is_valid = false;

    m_message_head                 = packet->head;
    m_message_head.current_segment = 0;

    GBuffer::Reset();
}

bool GMessage::Append(packet_t* packet) {
    const auto _packet_counter{packet->head.packet_counter};

    if (m_is_first) {
        m_is_first       = false;
        m_packet_counter = _packet_counter;
    }

    if (m_packet_counter != _packet_counter) {
        if (m_packet_counter < _packet_counter) {
            m_missed_counter += _packet_counter - m_packet_counter;
        }
        m_packet_counter = _packet_counter;
    }

    ++m_packet_counter;
    ++m_message_head.current_segment;

    if (m_no_error) {
        // clang-format off
        auto check_1 = packet->head.packet_type     == m_message_head.packet_type;
        auto check_2 = packet->head.file_id         == m_message_head.file_id;
        auto check_3 = packet->head.current_segment == m_message_head.current_segment;
        auto check_4 = packet->head.total_segments  == m_message_head.total_segments;
        // clang-format on
        m_no_error = check_1 && check_2 && check_3 && check_4;

        if (m_no_error) {
            auto* src_data = packet->data.bytes;
            auto  src_used = packet->head.data_length;
            return GBuffer::Append(src_data, src_used);
        }
    }

    ++m_errors_counter;
    return false;
}
