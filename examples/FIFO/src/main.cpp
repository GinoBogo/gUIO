
////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      March, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GArray.hpp"
#include "GArrayRoller.hpp"
#include "GDefine.hpp"
#include "GFIFOdevice.hpp"
#include "GLogger.hpp"
#include "GOptions.hpp"
#include "GProfile.hpp"
#include "GString.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <thread>

#define FIFO_WORD_SIZE sizeof(uint16_t)

// SECTION: PL_to_PS global variables
bool         RX_MODE_ENABLED  = true;
unsigned int RX_MODE_LOOPS    = 20;
std::string  RX_FILE_NAME     = "rx_data.txt";
unsigned int RX_PACKET_WORDS  = 1024;
unsigned int RX_FIFO_DEV_ADDR = 0xA0010000;
unsigned int RX_FIFO_DEV_SIZE = 4096;
int          RX_FIFO_UIO_NUM  = 1;
int          RX_FIFO_UIO_MAP  = 1;

// SECTION: PS_to_PL global variables
bool         TX_MODE_ENABLED  = true;
unsigned int TX_MODE_LOOPS    = 20;
std::string  TX_FILE_NAME     = "tx_data.txt";
unsigned int TX_PACKET_WORDS  = 1024;
unsigned int TX_FIFO_DEV_ADDR = 0xA0020000;
unsigned int TX_FIFO_DEV_SIZE = 4096;
int          TX_FIFO_UIO_NUM  = 2;
int          TX_FIFO_UIO_MAP  = 2;

static auto reader_of_packet_words(GArray<uint16_t>& array, const std::string& filename) {
    auto words{array.size()};

    if (!filename.empty()) {
        std::ifstream fs;
        fs.open(filename);

        if (fs.is_open()) {
            std::string line;

            for (decltype(words) i{0}; i < words; ++i) {
                uint16_t word{0x0000};

                std::getline(fs, line);
                word |= GString::strtous(line) << 8;
                std::getline(fs, line);
                word |= GString::strtous(line) << 0;

                array.data()[i] = word;
            }

            fs.close();

            array.free(0);
            LOG_FORMAT(info, "Words from \"%s\" file (%s)", filename, __func__);
            return;
        }
    }

    for (decltype(words) i{0}; i < words; ++i) {
        array.data()[i] = (uint16_t)(1 + i);
    }

    array.free(0);
    LOG_FORMAT(info, "Words from internal generator (%s)", __func__);
}

int p_num{0}; // dump progressive number
int p_val{0}; // dump previous byte value

static auto writer_of_packet_words(GArray<uint16_t>& array, const std::string& filename) {
    auto words{array.used()};

    if (!filename.empty()) {
        auto _path{std::filesystem::path(filename)};

        char _tail[32];
        snprintf(_tail, sizeof(_tail) - 1, "_%06d.%s", p_num++, _path.extension().c_str());

        auto _filename{_path.replace_extension(_tail)};

        std::ofstream fs;
        fs.open(_filename);

        if (fs.is_open()) {
            char h_line[16];
            char l_line[16];

            for (decltype(words) i{0}; i < words; ++i) {
                auto word = array.data()[i];

                auto h_val{(0xFF00 & word) >> 8};
                auto l_val{(0x00FF & word) >> 0};

                sprintf(h_line, "0x%02X\n", h_val);
                sprintf(l_line, "0x%02X\n", l_val);

                fs.write(h_line, (signed)strlen(h_line));
                fs.write(l_line, (signed)strlen(l_line));
            }

            fs.close();
            return;
        }
    }

    for (decltype(words) i{0}; i < words; ++i) {
        auto word = array.data()[i];

        auto h_val{(0xFF00 & word) >> 8};
        auto l_val{(0x00FF & word) >> 0};

        auto h_dif{h_val - p_val};
        auto l_dif{l_val - h_val};

        auto check_1{(h_val == 0) && (l_val == 247)};
        auto check_2{(p_val == 0) && (h_val == 247)};

        if (check_1 || check_2) {
            p_num = 0;
        }

        p_val = l_val;

        LOG_FORMAT(debug, " %4d | %3d | %+4d", p_num++, h_val, h_dif);
        LOG_FORMAT(debug, " %4d | %3d | %+4d", p_num++, l_val, l_dif);
    }
}

static auto worker_for_rx_mode() {
    RETURN_IF_ELSE_LOG(!RX_MODE_ENABLED, trace, "Thread STARTED (%s)", __func__);

    GProfile _profile;
    auto     _device = GFIFOdevice(RX_FIFO_DEV_ADDR, RX_FIFO_DEV_SIZE, RX_FIFO_UIO_NUM, RX_FIFO_UIO_MAP);
    auto     _roller = GArrayRoller<uint16_t>(RX_PACKET_WORDS, RX_MODE_LOOPS);
    auto     _error  = false;
    auto     _bytes  = 0UL;

    GOTO_IF(!_device.Open(), _exit_label);
    _device.Reset();
    _device.ClearEvent();

    _profile.Start();
    // -------------------------------------------------------------------
    for (decltype(RX_MODE_LOOPS) i{0}; (i < RX_MODE_LOOPS) && !_error; ++i) {
        _error = !_device.WaitThenClearEvent();
        BREAK_IF(_error);

        auto level{_device.GetRxLengthLevel(_error)};
        BREAK_IF(_error || (level == 0));

        auto words{_device.GetRxPacketWords(_error)};
        BREAK_IF(_error || (words <= 7));

        auto* dst_buf{_roller.Writing_Start(_error)};
        BREAK_IF(_error);

        _error = !_device.ReadPacket(dst_buf->data(), words);
        BREAK_IF(_error);

        _error = !dst_buf->used(words);
        _bytes += FIFO_WORD_SIZE * words;

        _roller.Writing_Stop(_error);
    }
    // -------------------------------------------------------------------
    _profile.Stop();
    LOG_FORMAT(info, "[RX] Data speed %0.3f Mbps", double(8 * _bytes) / _profile.us());

    for (decltype(RX_MODE_LOOPS) i{0}; (i < RX_MODE_LOOPS) && !_error; ++i) {
        auto* src_buf{_roller.Reading_Start(_error)};
        if (!_error) {
            writer_of_packet_words(*src_buf, RX_FILE_NAME);
        }
        _roller.Reading_Stop(_error);
    }

    _device.Close();

_exit_label:
    LOG_FORMAT(trace, "Thread STOPPED (%s)", __func__);
}

static auto worker_for_tx_mode() {
    RETURN_IF_ELSE_LOG(!TX_MODE_ENABLED, trace, "Thread STARTED (%s)", __func__);

    GProfile _profile;
    auto     _device = GFIFOdevice(TX_FIFO_DEV_ADDR, TX_FIFO_DEV_SIZE, TX_FIFO_UIO_NUM, TX_FIFO_UIO_MAP);
    auto     _array  = GArray<uint16_t>(TX_PACKET_WORDS);
    auto     _error  = false;
    auto     _bytes  = 0UL;

    GOTO_IF(!_device.Open(), _exit_label);
    _device.Reset();
    _device.ClearEvent();

    reader_of_packet_words(_array, TX_FILE_NAME);

    _error = !_device.SetTxPacketWords(_array.size());
    if (!_error) {
        LOG_FORMAT(debug, "[TX] Packet words %d", _device.GetTxPacketWords(_error));
        LOG_FORMAT(debug, "[TX] Unused words %d", _device.GetTxUnusedWords(_error));

        // _fifo.EnableReader();
        // _fifo.WritePacket(tx_buff.data(), tx_buff.size());

        _profile.Start();
        // -------------------------------------------------------------------
        for (decltype(TX_MODE_LOOPS) i{0}; (i < TX_MODE_LOOPS) && !_error; ++i) {
            _error = !_device.WritePacket(_array.data(), _array.size());
            BREAK_IF(_error);

            _error = !_device.WaitThenClearEvent();
            BREAK_IF(_error);

            _bytes += FIFO_WORD_SIZE * _array.size();
        }
        // -------------------------------------------------------------------
        _profile.Stop();
        LOG_FORMAT(info, "[TX] Data speed %0.3f Mbps", double(8 * _bytes) / _profile.us());
    }

    _device.Close();

_exit_label:
    LOG_FORMAT(trace, "Thread STOPPED (%s)", __func__);
}

static void load_options(const char* filename) {
    auto opts = GOptions();

    // clang-format off
    opts.Insert<bool        >("PL_to_PS.RX_MODE_ENABLED" , RX_MODE_ENABLED );
    opts.Insert<unsigned int>("PL_to_PS.RX_MODE_LOOPS"   , RX_MODE_LOOPS   );
    opts.Insert<std::string >("PL_to_PS.RX_FILE_NAME"    , RX_FILE_NAME    );
    opts.Insert<unsigned int>("PL_to_PS.RX_PACKET_WORDS" , RX_PACKET_WORDS );
    opts.Insert<unsigned int>("PL_to_PS.RX_FIFO_DEV_ADDR", RX_FIFO_DEV_ADDR);
    opts.Insert<unsigned int>("PL_to_PS.RX_FIFO_DEV_SIZE", RX_FIFO_DEV_SIZE);
    opts.Insert<int         >("PL_to_PS.RX_FIFO_UIO_NUM" , RX_FIFO_UIO_NUM );
    opts.Insert<int         >("PL_to_PS.RX_FIFO_UIO_MAP" , RX_FIFO_UIO_MAP );
    
    opts.Insert<bool        >("PS_to_PL.TX_MODE_ENABLED" , TX_MODE_ENABLED );
    opts.Insert<unsigned int>("PS_to_PL.TX_MODE_LOOPS"   , TX_MODE_LOOPS   );
    opts.Insert<std::string >("PS_to_PL.TX_FILE_NAME"    , TX_FILE_NAME    );
    opts.Insert<unsigned int>("PS_to_PL.TX_PACKET_WORDS" , TX_PACKET_WORDS );
    opts.Insert<unsigned int>("PS_to_PL.TX_FIFO_DEV_ADDR", TX_FIFO_DEV_ADDR);
    opts.Insert<unsigned int>("PS_to_PL.TX_FIFO_DEV_SIZE", TX_FIFO_DEV_SIZE);
    opts.Insert<int         >("PS_to_PL.TX_FIFO_UIO_NUM" , TX_FIFO_UIO_NUM );
    opts.Insert<int         >("PS_to_PL.TX_FIFO_UIO_MAP" , TX_FIFO_UIO_MAP );
    // clang-format on

    if (opts.Read(filename)) {
        // clang-format off
        RX_MODE_ENABLED  = opts.Get<bool        >("PL_to_PS.RX_MODE_ENABLED" ); 
        RX_MODE_LOOPS    = opts.Get<unsigned int>("PL_to_PS.RX_MODE_LOOPS"   ); 
        RX_FILE_NAME     = opts.Get<std::string >("PL_to_PS.RX_FILE_NAME"    );
        RX_PACKET_WORDS  = opts.Get<unsigned int>("PL_to_PS.RX_PACKET_WORDS" ); 
        RX_FIFO_DEV_ADDR = opts.Get<unsigned int>("PL_to_PS.RX_FIFO_DEV_ADDR"); 
        RX_FIFO_DEV_SIZE = opts.Get<unsigned int>("PL_to_PS.RX_FIFO_DEV_SIZE"); 
        RX_FIFO_UIO_NUM  = opts.Get<int         >("PL_to_PS.RX_FIFO_UIO_NUM" ); 
        RX_FIFO_UIO_MAP  = opts.Get<int         >("PL_to_PS.RX_FIFO_UIO_MAP" ); 

        TX_MODE_ENABLED  = opts.Get<bool        >("PS_to_PL.TX_MODE_ENABLED" ); 
        TX_MODE_LOOPS    = opts.Get<unsigned int>("PS_to_PL.TX_MODE_LOOPS"   ); 
        TX_FILE_NAME     = opts.Get<std::string >("PS_to_PL.TX_FILE_NAME"    );
        TX_PACKET_WORDS  = opts.Get<unsigned int>("PS_to_PL.TX_PACKET_WORDS" ); 
        TX_FIFO_DEV_ADDR = opts.Get<unsigned int>("PS_to_PL.TX_FIFO_DEV_ADDR"); 
        TX_FIFO_DEV_SIZE = opts.Get<unsigned int>("PS_to_PL.TX_FIFO_DEV_SIZE"); 
        TX_FIFO_UIO_NUM  = opts.Get<int         >("PS_to_PL.TX_FIFO_UIO_NUM" ); 
        TX_FIFO_UIO_MAP  = opts.Get<int         >("PS_to_PL.TX_FIFO_UIO_MAP" );
        // clang-format on
    }
}

#define EXE_NAME "_fifo"

int main(int argc, char* argv[]) {
    GLogger::Initialize(EXE_NAME ".log");
    LOG_FORMAT(trace, "Process STARTED (%s)", EXE_NAME);

    load_options(EXE_NAME ".cfg");

    std::thread thread_for_rx_mode(worker_for_rx_mode);
    std::thread thread_for_tx_mode(worker_for_tx_mode);

    thread_for_rx_mode.join();
    thread_for_tx_mode.join();

    LOG_FORMAT(trace, "Process STOPPED (%s)", EXE_NAME);
    return 0;
}
