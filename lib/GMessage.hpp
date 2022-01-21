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

    void Initialize(TPacket *packet);

    bool Append(TPacket *packet);

    private:
    bool     m_is_valid;
    uint16_t m_file_id;
    uint16_t m_current_segment;
};

#endif /* GMESSAGE_HPP_ */