////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      March, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GFIFOdevice.hpp"
#include "GLogger.hpp"
#include "GProfile.hpp"

#define FIFO_DEV_ADDR  0xA0010000
#define FIFO_DEV_SIZE  4096
#define FIFO_UIO_NUM   3
#define FIFO_UIO_MAP   0
#define FIFO_WORD_SIZE sizeof(uint16_t)

#define SPACE_PACKET_BYTES 2040
#define SPACE_PACKET_WORDS (SPACE_PACKET_BYTES / FIFO_WORD_SIZE)

void load_space_packet(uint16_t* buffer) {
    if (buffer != nullptr) {
        for (auto i{0U}; i < SPACE_PACKET_WORDS; ++i) {
            buffer[i] = (uint16_t)(1 + i);
        }
    }
}

int main(void) {
    GLogger::Initialize("MUST_FIFO.log");
    LOG_WRITE(trace, "Process STARTED");

    GFIFOdevice tx_fifo(FIFO_DEV_ADDR, FIFO_DEV_SIZE, FIFO_UIO_NUM, FIFO_UIO_MAP);

    if (tx_fifo.Open()) {
        auto buffer = new uint16_t[SPACE_PACKET_WORDS];

        load_space_packet(buffer);

        GProfile profile;

        if (tx_fifo.SetPacketWords(SPACE_PACKET_WORDS)) {
            bool     _err, _res{true};
            uint32_t _val;
            auto     _loop = 10000;

            _val = tx_fifo.GetPacketWords(&_err);
            LOG_FORMAT(debug, "Packet words %d", _val);

            _val = tx_fifo.GetUnusedWords(&_err);
            LOG_FORMAT(debug, "Unused words %d", _val);

            _res &= tx_fifo.ClearEvent();
            _res &= tx_fifo.EnableReader();
            _res &= tx_fifo.WritePacket(buffer, SPACE_PACKET_WORDS);

            profile.Start();
            for (auto i{0}; (i < _loop) && _res; ++i) {
                _res = tx_fifo.WritePacket(buffer, SPACE_PACKET_WORDS);
                if (_res) {
                    _res = tx_fifo.WaitEvent();
                    if (_res) {
                        // load_space_packet(buffer);
                        _res = tx_fifo.ClearEvent();
                    }
                }
            }
            profile.Stop();

            LOG_FORMAT(info, "Speed %f", double(_loop * 8 * SPACE_PACKET_BYTES) / profile.us());
        }

        delete[] buffer;

        tx_fifo.Close();
    }

    LOG_WRITE(trace, "Process STOPPED");
    return 0;
}
