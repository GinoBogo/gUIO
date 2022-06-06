
////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      March, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GArrayRoller.hpp"
#include "GDefine.hpp"
#include "GFIFOdevice.hpp"
#include "GLogger.hpp"
#include "GProfile.hpp"
#include "GString.hpp"
#include "GWorksCoupler.hpp"
#include "globals.hpp"
#include "streams.hpp"

#include <filesystem> // path

typedef struct parameters_t {
    unsigned int            current_loop = 0;
    unsigned int            total_loops  = 0;
    unsigned long           total_bytes  = 0;
    GUdpClient*             client       = nullptr;
    GUdpServer*             server       = nullptr;
    GFIFOdevice*            device       = nullptr;
    GArrayRoller<uint16_t>* roller       = nullptr;
    GProfile*               profile      = nullptr;

} parameters_t;

// ============================================================================
// RX WAITER functions
// ============================================================================

static void rx_waiter_preamble(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STARTED (PS -> OUT)");
}

static void rx_waiter_consumer(bool& _quit, std::any& _args) {
    auto* parameters = std::any_cast<parameters_t*>(_args);
    auto* client     = parameters->client;
    auto* roller     = parameters->roller;

    auto _line  = 0;
    auto _error = false;

    auto* src_buf{roller->Reading_Start(_error)};
    GOTO_IF(_error, _exit_label, _line = __LINE__);

    _error = !stream_writer_for_rx_words(*src_buf, *client);
    GOTO_IF(_error, _exit_label, _line = __LINE__);

    roller->Reading_Stop(_error);
    GOTO_IF(_error, _exit_label, _line = __LINE__);
    return;

_exit_label:
    if (_line != 0) {
        LOG_FORMAT(error, "@ LINE %d -> reading errors: %lu", _line, roller->errors());
    }
    _quit = true;
}

static void rx_waiter_epilogue(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STOPPED (PS -> OUT)");
}

// ============================================================================
// RX MASTER functions
// ============================================================================

static void rx_master_preamble(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STARTED (PL -> PS)");

    auto* parameters = std::any_cast<parameters_t*>(_args);
    auto* device     = parameters->device;
    auto* profile    = parameters->profile;

    if (device->Open()) {
        device->Reset();
        device->ClearEvent();

        profile->Start();
        return;
    }
    _quit = true;
}

static void rx_master_producer(bool& _quit, std::any& _args) {
    auto* parameters   = std::any_cast<parameters_t*>(_args);
    auto  current_loop = parameters->current_loop;
    auto  total_loops  = parameters->total_loops;
    auto* device       = parameters->device;
    auto* roller       = parameters->roller;

    auto     _line  = 0;
    auto     _error = false;
    uint32_t _level = 0;
    uint32_t _words = 0;

    if (!_quit && (current_loop < total_loops)) {
        _error = !device->WaitThenClearEvent();
        GOTO_IF(_error, _exit_label, _line = __LINE__);

        _level = device->GetRxLengthLevel(_error);
        GOTO_IF(_error || (_level == 0), _exit_label, _line = __LINE__);

        _words = device->GetRxPacketWords(_error);
        GOTO_IF(_error || (_words <= 7), _exit_label, _line = __LINE__);

        auto* dst_buf{roller->Writing_Start(_error)};
        GOTO_IF(_error, _exit_label, _line = __LINE__);

        _error = !dst_buf->used(_words);
        GOTO_IF(_error, _exit_label, _line = __LINE__);

        _error = !device->ReadPacket(dst_buf->data(), _words);
        GOTO_IF(_error, _exit_label, _line = __LINE__);

        roller->Writing_Stop(_error);
        GOTO_IF(_error, _exit_label, _line = __LINE__);

        parameters->current_loop++;
        parameters->total_bytes += FIFO_WORD_SIZE * _words;
        return;
    }

_exit_label:
    if (_line != 0) {
        LOG_FORMAT(error, "@ LINE %d -> error: %d, level: %u, words: %u", _line, _error, _level, _words);
    }
    _quit = true;
}

static void rx_master_epilogue(bool& _quit, std::any& _args) {
    auto* parameters  = std::any_cast<parameters_t*>(_args);
    auto* device      = parameters->device;
    auto* roller      = parameters->roller;
    auto* profile     = parameters->profile;
    auto  total_bytes = parameters->total_bytes;

    profile->Stop();
    device->ClearEvent(); // WARNING: a not served interrupt may happen
    device->Close();

    auto delta_time{profile->us() / 1e6};
    auto data_speed{GString::value_scaler((8 * total_bytes) / delta_time, "bps")};

    LOG_FORMAT(info, "[STATS] RX roller errors: %lu", roller->errors());
    LOG_FORMAT(info, "[STATS] RX data speed: %0.3f %s", data_speed.first, data_speed.second.c_str());

    LOG_WRITE(trace, "Thread STOPPED (PL -> PS)");
}

// ============================================================================
// TX WAITER functions
// ============================================================================

static void tx_waiter_preamble(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STARTED (PS <- OUT)");
}

static void tx_waiter_producer(bool& _quit, std::any& _args) {
}

static void tx_waiter_epilogue(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STOPPED (PS <- OUT)");
}

// ============================================================================
// TX MASTER functions
// ============================================================================

static void tx_master_preamble(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STARTED (PL <- PS)");
}

static void tx_master_consumer(bool& _quit, std::any& _args) {
}

static void tx_master_epilogue(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STOPPED (PL <- PS)");
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char* argv[]) {
    auto exec     = std::filesystem::path(argv[0]);
    auto exec_log = exec.stem().concat(".log");
    auto exec_cfg = exec.stem().concat(".cfg");

    GLogger::Initialize(exec_log.c_str());
    LOG_FORMAT(trace, "Process STARTED (%s)", exec.stem().c_str());

    load_options(exec_cfg.c_str());

    // SECTION: functions handles

    GWorksCoupler::work_func_t work_func_rx;
    work_func_rx.waiter_preamble = rx_waiter_preamble;
    work_func_rx.waiter_calculus = rx_waiter_consumer;
    work_func_rx.waiter_epilogue = rx_waiter_epilogue;
    work_func_rx.master_preamble = rx_master_preamble;
    work_func_rx.master_calculus = rx_master_producer;
    work_func_rx.master_epilogue = rx_master_epilogue;

    GWorksCoupler::work_func_t work_func_tx;
    work_func_tx.waiter_preamble = tx_waiter_preamble;
    work_func_tx.waiter_calculus = tx_waiter_producer;
    work_func_tx.waiter_epilogue = tx_waiter_epilogue;
    work_func_tx.master_preamble = tx_master_preamble;
    work_func_tx.master_calculus = tx_master_consumer;
    work_func_tx.master_epilogue = tx_master_epilogue;

    // SECTION: shared parameters

    auto quit{false};

    auto rx_client{GUdpClient(RX_CLIENT_ADDR.c_str(), RX_CLIENT_PORT, RX_FIFO_TAG_NAME.c_str())};
    auto tx_server{GUdpServer(TX_SERVER_ADDR.c_str(), TX_SERVER_PORT, TX_FIFO_TAG_NAME.c_str())};

    auto rx_device{GFIFOdevice(RX_FIFO_DEV_ADDR, RX_FIFO_DEV_SIZE, RX_FIFO_UIO_NUM, RX_FIFO_UIO_MAP, RX_FIFO_TAG_NAME)};
    auto tx_device{GFIFOdevice(TX_FIFO_DEV_ADDR, TX_FIFO_DEV_SIZE, TX_FIFO_UIO_NUM, TX_FIFO_UIO_MAP, TX_FIFO_TAG_NAME)};

    auto rx_roller{GArrayRoller<uint16_t>(RX_PACKET_WORDS, 40)};
    auto tx_roller{GArrayRoller<uint16_t>(TX_PACKET_WORDS, 40)};

    GProfile rx_profile;
    GProfile tx_profile;

    parameters_t parameters_rx;
    parameters_rx.total_loops = RX_MODE_LOOPS;
    parameters_rx.client      = &rx_client;
    parameters_rx.device      = &rx_device;
    parameters_rx.roller      = &rx_roller;
    parameters_rx.profile     = &rx_profile;

    parameters_t parameters_tx;
    parameters_tx.total_loops = TX_MODE_LOOPS;
    parameters_tx.server      = &tx_server;
    parameters_tx.device      = &tx_device;
    parameters_tx.roller      = &tx_roller;
    parameters_tx.profile     = &tx_profile;

    auto args_rx = std::any(&parameters_rx);
    auto args_tx = std::any(&parameters_tx);

    // SECTION: works couplers

    auto works_coupler_rx{GWorksCoupler(work_func_rx, quit, args_rx, RX_MODE_ENABLED)};
    auto works_coupler_tx{GWorksCoupler(work_func_tx, quit, args_tx, TX_MODE_ENABLED)};

    works_coupler_rx.Wait();
    works_coupler_tx.Wait();

    LOG_FORMAT(trace, "Process STOPPED (%s)", exec.stem().c_str());
    return 0;
}
