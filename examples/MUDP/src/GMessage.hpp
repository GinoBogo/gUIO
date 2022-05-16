////////////////////////////////////////////////////////////////////////////////
/// \file      GMessage.hpp
/// \version   0.1
/// \date      January, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GMESSAGE_HPP
#define GMESSAGE_HPP

#include "GBuffer.hpp"
#include "GPacket.hpp"

class GMessage : public GBuffer {
    public:
    static const size_t MAX_MESSAGE_SIZE = 64 * 1024;

    GMessage(uint32_t max_size = GMessage::MAX_MESSAGE_SIZE);

    void Clear();
    void Initialize(TPacket *packet);
    bool Append(TPacket *packet);

    [[nodiscard]] auto IsValid() const {
        return m_no_error && (m_message_head.current_segment == m_message_head.total_segments);
    }

    [[nodiscard]] auto head() const {
        return &m_message_head;
    }

    [[nodiscard]] auto PacketCounter() const {
        return m_packet_counter;
    }

    [[nodiscard]] auto MissedCounter() const {
        return m_missed_counter;
    }

    [[nodiscard]] auto ErrorsCounter() const {
        return m_errors_counter;
    }

    private:
    bool        m_no_error;
    bool        m_is_valid;
    uint32_t    m_packet_counter;
    uint32_t    m_missed_counter;
    uint32_t    m_errors_counter;
    TPacketHead m_message_head;
};

#endif // GMESSAGE_HPP
