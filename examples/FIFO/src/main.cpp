////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      March, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GArray.hpp"
#include "GFIFOdevice.hpp"
#include "GLogger.hpp"
#include "GProfile.hpp"
#include "GString.hpp"

#include <cstring>
#include <fstream>

#define FIFO_WORD_SIZE sizeof(uint16_t)

// SECTION: from PL to PS
#define RX_FIFO_DEV_ADDR 0xA0070000
#define RX_FIFO_DEV_SIZE 65536
#define RX_FIFO_UIO_NUM  2
#define RX_FIFO_UIO_MAP  0

// SECTION: from PS to PL
#define TX_FIFO_DEV_ADDR 0xA0010000
#define TX_FIFO_DEV_SIZE 4096
#define TX_FIFO_UIO_NUM  3
#define TX_FIFO_UIO_MAP  0

#define CADU_PACKET_BYTES 2044
#define CADU_PACKET_WORDS (CADU_PACKET_BYTES / FIFO_WORD_SIZE)

#define SPACE_PACKET_BYTES 65536
#define SPACE_PACKET_WORDS (SPACE_PACKET_BYTES / FIFO_WORD_SIZE)

const auto ENABLE_TX_MODE = 0;
const auto ENABLE_RX_MODE = 1;

static auto load_packet_words(uint16_t* buffer, uint32_t words, const char* filename = nullptr) {
    auto _ret{false};

    if (buffer != nullptr && words > 0) {
        if (filename != nullptr) {
            std::ifstream fs;
            fs.open(filename);

            if (fs.is_open()) {
                std::string line;

                auto _bytes{(uint8_t*)buffer};

                for (auto i{0}; !fs.eof(); ++i) {
                    std::getline(fs, line);
                    _bytes[i] = GString::strtouc(line);
                }

                fs.close();
                _ret = true;
            }
        }
        else {
            for (decltype(words) i{0}; i < words; ++i) {
                buffer[i] = (uint16_t)(1 + i);
            }
            _ret = true;
        }
    }
    return _ret;
}

int p_num = 0; // dump progressive number
int p_val = 0; // dump previous byte value

static auto dump_packet_words(uint16_t* buffer, uint32_t words, const char* filename = nullptr) {
    auto _ret{false};

    if (buffer != nullptr && words > 0) {
        if (filename != nullptr) {
            std::ofstream fs;
            fs.open(filename);

            if (fs.is_open()) {
                char line[64];

                for (decltype(words) i{0}; i < words; ++i) {
                    sprintf(line, "0x%04X\n", buffer[i]);
                    fs.write(line, (signed)strlen(line));
                }

                fs.close();
            }
        }
        else {
            for (decltype(words) i{0}; i < words; ++i) {
                auto h_val{(0xFF00 & buffer[i]) >> 8};
                auto l_val{(0x00FF & buffer[i]) >> 0};

                auto check_1{(h_val == 0) && (l_val == 247)};
                auto check_2{(p_val == 0) && (h_val == 247)};

                if (check_1 || check_2) {
                    p_num = 0;
                }

                p_val = l_val;

                LOG_FORMAT(debug, " (%4d) %3d", p_num++, h_val);
                LOG_FORMAT(debug, " (%4d) %3d", p_num++, l_val);
            }
        }
    }

    return _ret;
}

int main(int argc, char* argv[]) {
    GLogger::Initialize("MUST_FIFO.log");
    LOG_FORMAT(trace, "Process STARTED (%s)", __func__);

    if (ENABLE_TX_MODE) {
        auto tx_fifo = GFIFOdevice(TX_FIFO_DEV_ADDR, TX_FIFO_DEV_SIZE, TX_FIFO_UIO_NUM, TX_FIFO_UIO_MAP);
        auto tx_buff = GArray<uint16_t>(CADU_PACKET_WORDS);

        if (tx_fifo.Open()) {
            if (argc > 1) {
                load_packet_words(tx_buff.ptr, tx_buff.len, argv[1]);
            }
            else {
                load_packet_words(tx_buff.ptr, tx_buff.len);
            }

            if (tx_fifo.SetTxPacketWords(tx_buff.len)) {
                bool _err, _res{true};
                auto _loop  = 2000;
                auto _bytes = 0UL;

                LOG_FORMAT(debug, "[TX] Packet words %d", tx_fifo.GetTxPacketWords(&_err));
                LOG_FORMAT(debug, "[TX] Unused words %d", tx_fifo.GetTxUnusedWords(&_err));

                tx_fifo.Reset();
                tx_fifo.ClearEvent();

                tx_fifo.EnableReader();
                tx_fifo.WritePacket(tx_buff.ptr, tx_buff.len);

                GProfile profile;
                profile.Start();
                // -------------------------------------------------------------------
                for (decltype(_loop) i{0}; (i < _loop) && _res; ++i) {
                    _res = tx_fifo.WritePacket(tx_buff.ptr, tx_buff.len);
                    if (_res) {
                        _res = tx_fifo.WaitThenClearEvent();
                        if (_res) {
                            _bytes += FIFO_WORD_SIZE * tx_buff.len;
                        }
                    }
                }
                // -------------------------------------------------------------------
                profile.Stop();
                LOG_FORMAT(info, "[TX] Data speed %0.3f Mbps", double(8 * _bytes) / profile.us());
            }

            tx_fifo.Close();
        }
    }

    if (ENABLE_RX_MODE) {
        auto rx_fifo = GFIFOdevice(RX_FIFO_DEV_ADDR, RX_FIFO_DEV_SIZE, RX_FIFO_UIO_NUM, RX_FIFO_UIO_MAP);
        auto rx_buff = GArray<uint16_t>(SPACE_PACKET_WORDS);

        if (rx_fifo.Open()) {
            bool _err, _res{true};
            auto _loop  = 4;
            auto _bytes = 0UL;

            rx_fifo.Reset();
            rx_fifo.ClearEvent();

            GProfile profile;
            profile.Start();
            // -------------------------------------------------------------------
            for (decltype(_loop) i{0}; (i < _loop) && _res; ++i) {
                _res = rx_fifo.WaitThenClearEvent();
                if (_res) {
                    auto level{rx_fifo.GetRxLengthLevel(&_err)};
                    if (!_err && level > 0) {
                        auto words{rx_fifo.GetRxPacketWords(&_err)};
                        if (!_err && words > 7) {
                            _res = rx_fifo.ReadPacket(rx_buff.ptr, words);
                            if (_res) {
                                dump_packet_words(rx_buff.ptr, words);
                                _bytes += FIFO_WORD_SIZE * words;
                            }
                        }
                    }
                }
            }
            // -------------------------------------------------------------------
            profile.Stop();
            LOG_FORMAT(info, "[RX] Data speed %0.3f Mbps", double(8 * _bytes) / profile.us());

            rx_fifo.Close();
        }
    }

    LOG_FORMAT(trace, "Process STOPPED (%s)", __func__);
    return 0;
}
