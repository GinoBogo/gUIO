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
}

void GMessage::Initialize(TPacket *packet) {
    m_is_valid        = true;
    m_file_id         = packet->head.file_id;
    m_current_segment = 0;
    GBuffer::Reset();
}

bool GMessage::Append(TPacket *packet) {
    if (m_is_valid) {
        auto check_1 = packet->head.file_id == m_file_id;
        auto check_2 = packet->head.current_segment == 1 + m_current_segment;
        m_is_valid   = check_1 && check_2;

        if (m_is_valid) {
            auto src_data  = (uint8_t *)&packet->data;
            auto src_count = packet->head.data_length;
            return GBuffer::Append(src_data, src_count);
        }
    }
    return false;
}