////////////////////////////////////////////////////////////////////////////////
/// \file      GMessage.hpp
/// \version   0.1
/// \date      January, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GMESSAGE_HPP_
#define GMESSAGE_HPP_

#include "GBuffer.hpp"
#include "GPacket.hpp"

class GMessage : public GBuffer {
    public:
    static const size_t MAX_MESSAGE_SIZE = 64 * 1024;

    GMessage(const uint32_t max_size = GMessage::MAX_MESSAGE_SIZE);

    void Clear();
    void Initialize(TPacket *packet);
    bool Append(TPacket *packet);

    auto IsValid() {
        return m_no_error && (m_current_segment == m_total_segments);
    }

    auto PacketCounter() const {
        return m_packet_counter;
    }

    auto ErrorsCounter() const {
        return m_errors_counter;
    }

    private:
    bool     m_no_error;
    bool     m_is_valid;
    uint8_t  m_packet_type;
    uint16_t m_file_id;
    uint32_t m_packet_counter;
    uint32_t m_errors_counter;
    uint16_t m_current_segment;
    uint16_t m_total_segments;
};

#endif /* GMESSAGE_HPP_ */