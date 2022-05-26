
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
#include "streams.hpp"

#include <condition_variable>
#include <mutex>
#include <thread>

#define FIFO_WORD_SIZE sizeof(uint16_t)

// SECTION: PL_to_PS global variables
bool         RX_MODE_ENABLED  = true;
unsigned int RX_MODE_LOOPS    = 20;
std::string  RX_FILE_NAME     = "rx_words.bin";
unsigned int RX_PACKET_WORDS  = 1024;
std::string  RX_FIFO_TAG_NAME = "RX";
unsigned int RX_FIFO_DEV_ADDR = 0xA0010000;
unsigned int RX_FIFO_DEV_SIZE = 4096;
int          RX_FIFO_UIO_NUM  = 1;
int          RX_FIFO_UIO_MAP  = 1;

// SECTION: PS_to_PL global variables
bool         TX_MODE_ENABLED  = true;
unsigned int TX_MODE_LOOPS    = 20;
std::string  TX_FILE_NAME     = "tx_words.bin";
unsigned int TX_PACKET_WORDS  = 1024;
std::string  TX_FIFO_TAG_NAME = "TX";
unsigned int TX_FIFO_DEV_ADDR = 0xA0020000;
unsigned int TX_FIFO_DEV_SIZE = 4096;
int          TX_FIFO_UIO_NUM  = 2;
int          TX_FIFO_UIO_MAP  = 2;

// SECTION: threads race control
volatile auto           rx_total{0};
std::mutex              rx_mutex;
std::condition_variable rx_event;

volatile auto           tx_total{0};
std::mutex              tx_mutex;
std::condition_variable tx_event;

static auto worker_for_rx_words(bool& quit, GArrayRoller<uint16_t>& roller) {
    RETURN_IF_ELSE_LOG(!RX_MODE_ENABLED, trace, "Thread STARTED (%s)", __func__);

    auto _error{false};

    while (!quit && !_error) {
        std::unique_lock rx_guard(rx_mutex);
        rx_event.wait(rx_guard, [] { return rx_total > 0; });

        BREAK_IF(quit || _error);

        rx_total--;
        rx_guard.unlock();

        auto* src_buf{roller.Reading_Start(_error)}; // INFO: read ->
        if (!_error) {
            writer_for_rx_words(*src_buf, RX_FILE_NAME);
        }
        roller.Reading_Stop(_error); // INFO: read <-
    }

    LOG_FORMAT(info, "[STATS] RX read/write errors: %d (%s)", roller.errors(), __func__);

    LOG_FORMAT(trace, "Thread STOPPED (%s)", __func__);
}

static auto worker_for_rx_board(bool& quit, GArrayRoller<uint16_t>& roller) {
    RETURN_IF_ELSE_LOG(!RX_MODE_ENABLED, trace, "Thread STARTED (%s)", __func__);

    GProfile _profile;
    auto     _device = GFIFOdevice(RX_FIFO_DEV_ADDR, RX_FIFO_DEV_SIZE, RX_FIFO_UIO_NUM, RX_FIFO_UIO_MAP, RX_FIFO_TAG_NAME);
    auto     _error  = false;
    auto     _bytes  = 0UL;

    GOTO_IF(!_device.Open(), _exit_label);
    _device.Reset();
    _device.ClearEvent();

    _profile.Start();
    // -------------------------------------------------------------------
    for (decltype(RX_MODE_LOOPS) i{0}; (i < RX_MODE_LOOPS) && !_error && !quit; ++i) {
        _error = !_device.WaitThenClearEvent();
        BREAK_IF(_error);

        auto level{_device.GetRxLengthLevel(_error)};
        BREAK_IF(_error || (level == 0));

        auto words{_device.GetRxPacketWords(_error)};
        BREAK_IF(_error || (words <= 7));

        auto* dst_buf{roller.Writing_Start(_error)}; // INFO: write ->
        BREAK_IF(_error);

        _error = !dst_buf->used(words);
        BREAK_IF(_error);

        _error = !_device.ReadPacket(dst_buf->data(), words);
        BREAK_IF(_error);

        roller.Writing_Stop(_error); // INFO: write <-
        BREAK_IF(_error);

        std::lock_guard rx_guard(rx_mutex);
        rx_total++;
        rx_event.notify_one();

        _bytes += FIFO_WORD_SIZE * words;
    }
    // -------------------------------------------------------------------
    _profile.Stop();
    LOG_FORMAT(info, "[STATS] RX data speed %0.3f Mbps", double(8 * _bytes) / _profile.us());

    _device.Close();

_exit_label:
    quit     = true;
    rx_total = 1;
    rx_event.notify_one();

    LOG_FORMAT(trace, "Thread STOPPED (%s)", __func__);
}

static auto worker_for_tx_words(bool& quit, GArrayRoller<uint16_t>& roller) {
    RETURN_IF_ELSE_LOG(!TX_MODE_ENABLED, trace, "Thread STARTED (%s)", __func__);

    auto _error{false};

    while (!quit && !_error) {
        std::unique_lock tx_guard(tx_mutex);
        tx_event.wait(tx_guard, [] { return tx_total > 0; });

        BREAK_IF(quit || _error);

        tx_total--;
        tx_guard.unlock();

        auto* dst_buf{roller.Writing_Start(_error)}; // INFO: write ->
        if (!_error) {
            reader_for_tx_words(*dst_buf, TX_FILE_NAME);
        }
        roller.Writing_Stop(_error); // INFO: write <-
    }

    LOG_FORMAT(info, "[STATS] TX read/write errors: %d (%s)", roller.errors(), __func__);

    LOG_FORMAT(trace, "Thread STOPPED (%s)", __func__);
}

static auto worker_for_tx_board(bool& quit, GArrayRoller<uint16_t>& roller) {
    RETURN_IF_ELSE_LOG(!TX_MODE_ENABLED, trace, "Thread STARTED (%s)", __func__);

    GProfile _profile;
    auto     _device = GFIFOdevice(TX_FIFO_DEV_ADDR, TX_FIFO_DEV_SIZE, TX_FIFO_UIO_NUM, TX_FIFO_UIO_MAP, TX_FIFO_TAG_NAME);
    auto     _array  = GArray<uint16_t>(TX_PACKET_WORDS);
    auto     _error  = false;
    auto     _bytes  = 0UL;

    GOTO_IF(!_device.Open(), _exit_label);
    _device.Reset();
    _device.ClearEvent();

    reader_for_tx_words(_array, TX_FILE_NAME);

    _error = !_device.SetTxPacketWords(TX_PACKET_WORDS);
    if (!_error) {
        LOG_FORMAT(debug, "[TX] Packet words %d", _device.GetTxPacketWords(_error));
        LOG_FORMAT(debug, "[TX] Unused words %d", _device.GetTxUnusedWords(_error));

        // _fifo.EnableReader();

        _profile.Start();
        // -------------------------------------------------------------------
        for (decltype(TX_MODE_LOOPS) i{0}; (i < TX_MODE_LOOPS) && !_error && !quit; ++i) {
            _error = !_device.WritePacket(_array.data(), _array.size());
            BREAK_IF(_error);

            _error = !_device.WaitThenClearEvent();
            BREAK_IF(_error);

            _bytes += FIFO_WORD_SIZE * _array.size();
        }
        // -------------------------------------------------------------------
        _profile.Stop();
        LOG_FORMAT(info, "[STATS] TX data speed %0.3f Mbps", double(8 * _bytes) / _profile.us());
    }

    _device.Close();

_exit_label:
    quit     = true;
    tx_total = 1;
    tx_event.notify_one();

    LOG_FORMAT(trace, "Thread STOPPED (%s)", __func__);
}

static void load_options(const char* filename) {
    auto opts = GOptions();

    // clang-format off
    opts.Insert<bool        >("PL_to_PS.RX_MODE_ENABLED" , RX_MODE_ENABLED );
    opts.Insert<unsigned int>("PL_to_PS.RX_MODE_LOOPS"   , RX_MODE_LOOPS   );
    opts.Insert<std::string >("PL_to_PS.RX_FILE_NAME"    , RX_FILE_NAME    );
    opts.Insert<unsigned int>("PL_to_PS.RX_PACKET_WORDS" , RX_PACKET_WORDS );
    opts.Insert<std::string >("PL_to_PS.RX_FIFO_TAG_NAME", RX_FIFO_TAG_NAME);
    opts.Insert<unsigned int>("PL_to_PS.RX_FIFO_DEV_ADDR", RX_FIFO_DEV_ADDR);
    opts.Insert<unsigned int>("PL_to_PS.RX_FIFO_DEV_SIZE", RX_FIFO_DEV_SIZE);
    opts.Insert<int         >("PL_to_PS.RX_FIFO_UIO_NUM" , RX_FIFO_UIO_NUM );
    opts.Insert<int         >("PL_to_PS.RX_FIFO_UIO_MAP" , RX_FIFO_UIO_MAP );
    
    opts.Insert<bool        >("PS_to_PL.TX_MODE_ENABLED" , TX_MODE_ENABLED );
    opts.Insert<unsigned int>("PS_to_PL.TX_MODE_LOOPS"   , TX_MODE_LOOPS   );
    opts.Insert<std::string >("PS_to_PL.TX_FILE_NAME"    , TX_FILE_NAME    );
    opts.Insert<unsigned int>("PS_to_PL.TX_PACKET_WORDS" , TX_PACKET_WORDS );
    opts.Insert<std::string >("PS_to_PL.TX_FIFO_TAG_NAME", TX_FIFO_TAG_NAME);
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
        RX_FIFO_TAG_NAME = opts.Get<std::string >("PL_to_PS.RX_FIFO_TAG_NAME");
        RX_FIFO_DEV_ADDR = opts.Get<unsigned int>("PL_to_PS.RX_FIFO_DEV_ADDR"); 
        RX_FIFO_DEV_SIZE = opts.Get<unsigned int>("PL_to_PS.RX_FIFO_DEV_SIZE"); 
        RX_FIFO_UIO_NUM  = opts.Get<int         >("PL_to_PS.RX_FIFO_UIO_NUM" ); 
        RX_FIFO_UIO_MAP  = opts.Get<int         >("PL_to_PS.RX_FIFO_UIO_MAP" ); 

        TX_MODE_ENABLED  = opts.Get<bool        >("PS_to_PL.TX_MODE_ENABLED" ); 
        TX_MODE_LOOPS    = opts.Get<unsigned int>("PS_to_PL.TX_MODE_LOOPS"   ); 
        TX_FILE_NAME     = opts.Get<std::string >("PS_to_PL.TX_FILE_NAME"    );
        TX_PACKET_WORDS  = opts.Get<unsigned int>("PS_to_PL.TX_PACKET_WORDS" ); 
        TX_FIFO_TAG_NAME = opts.Get<std::string >("PS_to_PL.TX_FIFO_TAG_NAME");
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

    auto quit{false};

    auto rx_roller{GArrayRoller<uint16_t>(RX_PACKET_WORDS, 40)};
    auto tx_roller{GArrayRoller<uint16_t>(TX_PACKET_WORDS, 40)};

    std::thread thread_for_rx_board(worker_for_rx_board, std::ref(quit), std::ref(rx_roller));
    std::thread thread_for_rx_words(worker_for_rx_words, std::ref(quit), std::ref(rx_roller));
    std::thread thread_for_tx_board(worker_for_tx_board, std::ref(quit), std::ref(tx_roller));
    std::thread thread_for_tx_words(worker_for_tx_words, std::ref(quit), std::ref(tx_roller));

    thread_for_rx_board.join();
    thread_for_rx_words.join();
    thread_for_tx_board.join();
    thread_for_tx_words.join();

    LOG_FORMAT(trace, "Process STOPPED (%s)", EXE_NAME);
    return 0;
}
