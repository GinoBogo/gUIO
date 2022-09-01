
////////////////////////////////////////////////////////////////////////////////
/// \file      GEncoder.hpp
/// \version   0.1
/// \date      February, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GENCODER_HPP
#define GENCODER_HPP

#include "GFiFo.hpp"
#include "GLogger.hpp"
#include "GMessage.hpp"

class GEncoder {
    public:
    GEncoder(uint32_t file_id = 0, uint32_t fifo_depth = 20) : m_fifo{GPacket::PACKET_FULL_SIZE, fifo_depth} {
        memset(&m_packet, 0, GPacket::PACKET_FULL_SIZE);
        SetFileID(file_id);
    }

    void SetFileID(uint32_t file_id) {
        m_packet_counter = 1;
        m_file_id        = file_id;
    }

    void Reset() {
        m_fifo.Reset();
    }

    bool Process(uint8_t packet_type, uint8_t* message_data = nullptr, uint32_t message_length = 0) {
        memset(&m_packet, 0, GPacket::PACKET_HEAD_SIZE);

        auto result{false};

        m_packet.head.packet_type     = packet_type;
        m_packet.head.packet_counter  = m_packet_counter++;
        m_packet.head.current_segment = 1;

        if (message_length == 0) {
            m_packet.head.total_segments = 1;

            result = m_fifo.Push((uint8_t*)&m_packet, GPacket::PACKET_HEAD_SIZE);
        }
        else {
            if (message_data != nullptr) {
                auto _num = (uint16_t)(message_length / GPacket::PACKET_DATA_SIZE);
                auto _rem = (uint16_t)(message_length % GPacket::PACKET_DATA_SIZE);

                m_packet.head.file_id        = m_file_id;
                m_packet.head.total_segments = _num + (uint16_t)(_rem != 0);

                auto _loop{true};

                while (_loop && (_num-- > 0)) {
                    m_packet.head.data_length = GPacket::PACKET_DATA_SIZE;
                    memcpy(m_packet.data.bytes, message_data, GPacket::PACKET_DATA_SIZE);
                    _loop = m_fifo.Push((uint8_t*)&m_packet, GPacket::PACKET_FULL_SIZE);

                    message_data += GPacket::PACKET_DATA_SIZE;
                    m_packet.head.current_segment += 1;
                }

                if (_loop && _rem != 0) {
                    m_packet.head.data_length = _rem;
                    memcpy(m_packet.data.bytes, message_data, _rem);
                    _loop = m_fifo.Push((uint8_t*)&m_packet, GPacket::PACKET_HEAD_SIZE + _rem);
                }

                result = _loop;
            }
        }

        if (!result) {
            LOG_WRITE(error, "Unable to encode message");
        }

        return result;
    }

    auto IsEmpty() {
        return m_fifo.IsEmpty();
    }

    auto IsFull() {
        return m_fifo.IsFull();
    }

    auto Pop(uint8_t* dst_data, uint32_t dst_size) {
        return m_fifo.Pop(dst_data, dst_size);
    }

    private:
    uint32_t m_packet_counter;
    uint32_t m_file_id;
    packet_t m_packet;
    GFiFo    m_fifo;
};

#endif // GENCODER_HPP
