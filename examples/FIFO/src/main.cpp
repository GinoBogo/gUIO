
////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      March, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GString.hpp"
#include "streams.hpp"

#include <filesystem> // path

// ============================================================================
// RX WAITER functions
// ============================================================================

static void rx_waiter_preamble(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STARTED (PS -> STREAM)");
}

static void rx_waiter_consumer(bool& _quit, std::any& _args) {
    auto* worker_args = std::any_cast<worker_args_t*>(_args);
    auto* client      = worker_args->client;
    auto* server      = worker_args->server;
    auto* roller      = worker_args->roller;

    auto _line  = 0;
    auto _error = false;

    // #region [critical]
    auto* src_buf{roller->Reading_Start(_error)};
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    _error = !stream_writer_for_rx_words(src_buf, client, server);
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    roller->Reading_Stop(_error);
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);
    // #endregion
    return;

_exit_label:
    LOG_IF(_line != 0, error, "FAILURE @ LINE %d (%s)", _line, __func__);
    Global::quit_process();
}

static void rx_waiter_epilogue(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STOPPED (PS -> STREAM)");
}

// ============================================================================
// RX MASTER functions
// ============================================================================

static void rx_master_preamble(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STARTED (PL -> PS)");

    auto* worker_args = std::any_cast<worker_args_t*>(_args);
    auto* device      = worker_args->device;
    auto* profile     = worker_args->profile;

    auto _line  = 0;
    auto _error = false;

    _error = !device->Open();
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    _error = !device->Reset();
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    _error = !device->ClearEvent();
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    profile->Start();
    return;

_exit_label:
    LOG_IF(_line != 0, error, "FAILURE @ LINE %d (%s)", _line, __func__);
    Global::quit_process();
}

static void rx_master_producer(bool& _quit, std::any& _args) {
    auto* worker_args   = std::any_cast<worker_args_t*>(_args);
    auto  loops_counter = worker_args->loops_counter;
    auto  total_loops   = worker_args->total_loops;
    auto* device        = worker_args->device;
    auto* roller        = worker_args->roller;

    auto     _line  = 0;
    auto     _error = false;
    uint32_t _level = 0;
    uint32_t _words = 0;

    if (!_quit && (loops_counter < total_loops)) {
        _level = device->GetRxLengthLevel(_error);
        GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

        GOTO_IF_BUT(_level > 0, _read_label);

        _error = !device->WaitThenClearEvent();
        GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

        _level = device->GetRxLengthLevel(_error);
        GOTO_IF_BUT(_error || (_level == 0), _exit_label, _line = __LINE__);

_read_label:
        _words = device->GetRxPacketWords(_error);
        GOTO_IF_BUT(_error || (_words <= 7), _exit_label, _line = __LINE__);

        // #region [critical]
        auto* dst_buf{roller->Writing_Start(_error)};
        GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

        _error = !dst_buf->used(_words);
        GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

        _error = !device->ReadPacket(dst_buf->data(), _words);
        GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

        roller->Writing_Stop(_error);
        GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);
        // #endregion

        worker_args->loops_counter++;
        worker_args->total_bytes += FIFO_WORD_SIZE * _words;
        return;
    }

_exit_label:
    LOG_IF(_line != 0, error, "FAILURE @ LINE %d -> level: %u, words: %u (%s)", _line, _level, _words, __func__);
    Global::quit_process();
}

static void rx_master_epilogue(bool& _quit, std::any& _args) {
    auto* worker_args   = std::any_cast<worker_args_t*>(_args);
    auto* device        = worker_args->device;
    auto* roller        = worker_args->roller;
    auto* profile       = worker_args->profile;
    auto  loops_counter = worker_args->loops_counter;
    auto  total_bytes   = worker_args->total_bytes;

    profile->Stop();
    device->ClearEvent(); // WARNING: a not served interrupt may happen
    device->Close();

    auto _speed{GString::value_scaler((8 * total_bytes) / profile->us_to_sec(), "bps")};

    LOG_FORMAT(info, "[STATS] RX maximum level: %lu", roller->max_used());
    LOG_FORMAT(info, "[STATS] RX roller errors: %lu", roller->errors());
    LOG_FORMAT(info, "[STATS] RX loops counter: %lu", loops_counter);
    LOG_FORMAT(info, "[STATS] RX average speed: %0.3f %s", _speed.first, _speed.second.c_str());

    LOG_WRITE(trace, "Thread STOPPED (PL -> PS)");
}

// ============================================================================
// TX WAITER functions
// ============================================================================

static void tx_waiter_preamble(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STARTED (PL <- PS)");

    auto* worker_args = std::any_cast<worker_args_t*>(_args);
    auto* device      = worker_args->device;
    auto* profile     = worker_args->profile;

    auto _line  = 0;
    auto _error = false;

    _error = !device->Open();
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    _error = !device->Reset();
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    _error = !device->SetTxPacketWords(TX_PACKET_WORDS);
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    _error = !device->SetTxAutoReader(true);
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    _error = !device->ClearEvent();
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    profile->Start();
    return;

_exit_label:
    LOG_IF(_line != 0, error, "FAILURE @ LINE %d (%s)", _line, __func__);
    Global::quit_process();
}

static void tx_waiter_consumer(bool& _quit, std::any& _args) {
    auto* worker_args = std::any_cast<worker_args_t*>(_args);
    auto* client      = worker_args->client;
    auto* device      = worker_args->device;
    auto* roller      = worker_args->roller;

    auto _line  = 0;
    auto _error = false;

    // #region [critical]
    auto* src_buf{roller->Reading_Start(_error)};
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    _error = !device->WritePacket(src_buf->data(), src_buf->used());
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    _error = !device->WaitThenClearEvent();
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

    roller->Reading_Stop(_error);
    GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);
    // #endregion

    // std::this_thread::sleep_for(std::chrono::microseconds(888));

    worker_args->total_bytes += src_buf->used_bytes();
    evaluate_stream_reader_start(roller, client);
    return;

_exit_label:
    LOG_IF(_line != 0, error, "FAILURE @ LINE %d -> roller used: %lu (%s)", _line, roller->used(), __func__);
    Global::quit_process();
}

static void tx_waiter_epilogue(bool& _quit, std::any& _args) {
    auto* worker_args   = std::any_cast<worker_args_t*>(_args);
    auto* device        = worker_args->device;
    auto* roller        = worker_args->roller;
    auto* profile       = worker_args->profile;
    auto  loops_counter = worker_args->loops_counter;
    auto  total_bytes   = worker_args->total_bytes;

    profile->Stop();
    device->ClearEvent(); // WARNING: a not served interrupt may happen
    device->Close();

    auto _speed{GString::value_scaler((8 * total_bytes) / profile->us_to_sec(), "bps")};

    LOG_FORMAT(info, "[STATS] TX maximum level: %lu", roller->max_used());
    LOG_FORMAT(info, "[STATS] TX roller errors: %lu", roller->errors());
    LOG_FORMAT(info, "[STATS] TX loops counter: %lu", loops_counter);
    LOG_FORMAT(info, "[STATS] TX average speed: %0.3f %s", _speed.first, _speed.second.c_str());

    LOG_WRITE(trace, "Thread STOPPED (PL <- PS)");
}

// ============================================================================
// TX MASTER functions
// ============================================================================

static void tx_master_preamble(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STARTED (PS <- STREAM)");
}

static void tx_master_producer(bool& _quit, std::any& _args) {
    auto* worker_args   = std::any_cast<worker_args_t*>(_args);
    auto  loops_counter = worker_args->loops_counter;
    auto  total_loops   = worker_args->total_loops;
    auto* client        = worker_args->client;
    auto* server        = worker_args->server;
    auto* roller        = worker_args->roller;

    auto _line  = 0;
    auto _error = false;

    if (!_quit && (loops_counter < total_loops)) {
        // #region [critical]
        auto* dst_buf{roller->Writing_Start(_error)};
        GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

        _error = !stream_reader_for_tx_words(dst_buf, client, server);
        GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);

        roller->Writing_Stop(_error);
        GOTO_IF_BUT(_error, _exit_label, _line = __LINE__);
        // #endregion

        // std::this_thread::sleep_for(std::chrono::nanoseconds(2));

        worker_args->loops_counter++;
        evaluate_stream_reader_stop(roller, client);
        return;
    }

_exit_label:
    LOG_IF(_line != 0, error, "FAILURE @ LINE %d -> roller free: %lu (%s)", _line, roller->free(), __func__);
    Global::quit_process();
}

static void tx_master_epilogue(bool& _quit, std::any& _args) {
    LOG_WRITE(trace, "Thread STOPPED (PS <- STREAM)");
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

    Global::load_options(exec_cfg.c_str());

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
    work_func_tx.waiter_calculus = tx_waiter_consumer;
    work_func_tx.waiter_epilogue = tx_waiter_epilogue;
    work_func_tx.master_preamble = tx_master_preamble;
    work_func_tx.master_calculus = tx_master_producer;
    work_func_tx.master_epilogue = tx_master_epilogue;

    // SECTION: worker parameters

    auto rx_client{g_udp_client_t(RX_CLIENT_ADDR.c_str(), RX_CLIENT_PORT, RX_FIFO_TAG_NAME.c_str())};
    auto tx_server{g_udp_server_t(TX_SERVER_ADDR.c_str(), TX_SERVER_PORT, TX_FIFO_TAG_NAME.c_str())};

    auto rx_device{g_fifo_device_t(RX_FIFO_DEV_ADDR, RX_FIFO_DEV_SIZE, RX_FIFO_UIO_NUM, RX_FIFO_UIO_MAP, RX_FIFO_TAG_NAME)};
    auto tx_device{g_fifo_device_t(TX_FIFO_DEV_ADDR, TX_FIFO_DEV_SIZE, TX_FIFO_UIO_NUM, TX_FIFO_UIO_MAP, TX_FIFO_TAG_NAME)};

    auto rx_roller{g_array_roller_t(RX_PACKET_WORDS, RX_ROLLER_NUMBER, RX_FIFO_TAG_NAME, RX_ROLLER_MAX_LEVEL, RX_ROLLER_MIM_LEVEL)};
    auto tx_roller{g_array_roller_t(TX_PACKET_WORDS, TX_ROLLER_NUMBER, TX_FIFO_TAG_NAME, TX_ROLLER_MAX_LEVEL, TX_ROLLER_MIM_LEVEL)};

    g_profile_t rx_profile;
    g_profile_t tx_profile;

    worker_args_t worker_args_rx;
    worker_args_rx.total_loops = RX_MODE_LOOPS;
    worker_args_rx.client      = &rx_client;
    worker_args_rx.server      = &tx_server; // NOTE: inter-link
    worker_args_rx.device      = &rx_device;
    worker_args_rx.roller      = &rx_roller;
    worker_args_rx.profile     = &rx_profile;

    worker_args_t worker_args_tx;
    worker_args_tx.total_loops = TX_MODE_LOOPS;
    worker_args_tx.client      = &rx_client; // NOTE: inter-link
    worker_args_tx.server      = &tx_server;
    worker_args_tx.device      = &tx_device;
    worker_args_tx.roller      = &tx_roller;
    worker_args_tx.profile     = &tx_profile;

    auto args_rx = std::any(&worker_args_rx);
    auto args_tx = std::any(&worker_args_tx);

    // SECTION: global parameters

    auto quit{false};

    Global::args.quit      = &quit;
    Global::args.rx_client = &rx_client;
    Global::args.tx_server = &tx_server;
    Global::args.rx_device = &rx_device;
    Global::args.tx_device = &tx_device;
    Global::args.rx_roller = &rx_roller;
    Global::args.tx_roller = &tx_roller;

    // SECTION: works couplers

    auto rx_coupler{g_works_coupler_t(work_func_rx, quit, args_rx, RX_MODE_ENABLED)};
    auto tx_coupler{g_works_coupler_t(work_func_tx, quit, args_tx, TX_MODE_ENABLED)};

    rx_coupler.Wait();
    tx_coupler.Wait();

    LOG_FORMAT(trace, "Process STOPPED (%s)", exec.stem().c_str());
    return 0;
}
