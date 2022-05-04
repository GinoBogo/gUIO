////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GDecoder.hpp"
#include "GFiFo.hpp"
#include "GLogger.hpp"
#include "GOptions.hpp"
#include "GUdpClient.hpp"
#include "GUdpServer.hpp"
#include "f_ad9361.hpp"
#include "f_gm_dh.hpp"
#include "f_gm_mc.hpp"
#include "f_hssl1.hpp"
#include "f_hssl2.hpp"

#include <condition_variable>
#include <mutex>  // mutex, lock_guard
#include <thread> // thread

std::string  GM_MC_SERVER_ADDR   = "127.0.0.1";
int          GM_MC_SERVER_PORT   = 30001;
std::string  GM_MC_CLIENT_ADDR   = "127.0.0.1";
int          GM_MC_CLIENT_PORT   = 30101;
std::string  GM_DH_SERVER_ADDR   = "127.0.0.1";
int          GM_DH_SERVER_PORT   = 40001;
std::string  GM_DH_CLIENT_ADDR   = "127.0.0.1";
int          GM_DH_CLIENT_PORT   = 40101;
std::string  HSSL1_SERVER_ADDR   = "127.0.0.1";
int          HSSL1_SERVER_PORT   = 50001;
std::string  HSSL1_CLIENT_ADDR   = "127.0.0.1";
int          HSSL1_CLIENT_PORT   = 50101;
std::string  HSSL2_SERVER_ADDR   = "127.0.0.1";
int          HSSL2_SERVER_PORT   = 60001;
std::string  HSSL2_CLIENT_ADDR   = "127.0.0.1";
int          HSSL2_CLIENT_PORT   = 60101;
unsigned int LINK_FIFO_DEPTH     = 40;
int          LINK_FIFO_MAX_LEVEL = 20;
int          LINK_FIFO_MIN_LEVEL = 2;

static void load_options(const char* filename) {
    auto opts = GOptions();

    // clang-format off
    opts.Insert<std::string >("socket.GM_MC_SERVER_ADDR", GM_MC_SERVER_ADDR  );
    opts.Insert<int         >("socket.GM_MC_SERVER_PORT", GM_MC_SERVER_PORT  );
    opts.Insert<std::string >("socket.GM_MC_CLIENT_ADDR", GM_MC_CLIENT_ADDR  );
    opts.Insert<int         >("socket.GM_MC_CLIENT_PORT", GM_MC_CLIENT_PORT  );
    opts.Insert<std::string >("socket.GM_DH_SERVER_ADDR", GM_DH_SERVER_ADDR  );
    opts.Insert<int         >("socket.GM_DH_SERVER_PORT", GM_DH_SERVER_PORT  );
    opts.Insert<std::string >("socket.GM_DH_CLIENT_ADDR", GM_DH_CLIENT_ADDR  );
    opts.Insert<int         >("socket.GM_DH_CLIENT_PORT", GM_DH_CLIENT_PORT  );
    opts.Insert<std::string >("socket.HSSL1_SERVER_ADDR", HSSL1_SERVER_ADDR  );
    opts.Insert<int         >("socket.HSSL1_SERVER_PORT", HSSL1_SERVER_PORT  );
    opts.Insert<std::string >("socket.HSSL1_CLIENT_ADDR", HSSL1_CLIENT_ADDR  );
    opts.Insert<int         >("socket.HSSL1_CLIENT_PORT", HSSL1_CLIENT_PORT  );
    opts.Insert<std::string >("socket.HSSL2_SERVER_ADDR", HSSL2_SERVER_ADDR  );
    opts.Insert<int         >("socket.HSSL2_SERVER_PORT", HSSL2_SERVER_PORT  );
    opts.Insert<std::string >("socket.HSSL2_CLIENT_ADDR", HSSL2_CLIENT_ADDR  );
    opts.Insert<int         >("socket.HSSL2_CLIENT_PORT", HSSL2_CLIENT_PORT  );
    opts.Insert<unsigned int>("fifo.LINK_FIFO_DEPTH"    , LINK_FIFO_DEPTH    );
    opts.Insert<int         >("fifo.LINK_FIFO_MAX_LEVEL", LINK_FIFO_MAX_LEVEL);
    opts.Insert<int         >("fifo.LINK_FIFO_MIN_LEVEL", LINK_FIFO_MIN_LEVEL);
    // clang-format on

    if (opts.Read(filename)) {
        // clang-format off
        GM_MC_SERVER_ADDR   = opts.Get<std::string >("socket.GM_MC_SERVER_ADDR");
        GM_MC_SERVER_PORT   = opts.Get<int         >("socket.GM_MC_SERVER_PORT");
        GM_MC_CLIENT_ADDR   = opts.Get<std::string >("socket.GM_MC_CLIENT_ADDR");
        GM_MC_CLIENT_PORT   = opts.Get<int         >("socket.GM_MC_CLIENT_PORT");
        GM_DH_SERVER_ADDR   = opts.Get<std::string >("socket.GM_DH_SERVER_ADDR");
        GM_DH_SERVER_PORT   = opts.Get<int         >("socket.GM_DH_SERVER_PORT");
        GM_DH_CLIENT_ADDR   = opts.Get<std::string >("socket.GM_DH_CLIENT_ADDR");
        GM_DH_CLIENT_PORT   = opts.Get<int         >("socket.GM_DH_CLIENT_PORT");
        HSSL1_SERVER_ADDR   = opts.Get<std::string >("socket.HSSL1_SERVER_ADDR");
        HSSL1_SERVER_PORT   = opts.Get<int         >("socket.HSSL1_SERVER_PORT");
        HSSL1_CLIENT_ADDR   = opts.Get<std::string >("socket.HSSL1_CLIENT_ADDR");
        HSSL1_CLIENT_PORT   = opts.Get<int         >("socket.HSSL1_CLIENT_PORT");
        HSSL2_SERVER_ADDR   = opts.Get<std::string >("socket.HSSL2_SERVER_ADDR");
        HSSL2_SERVER_PORT   = opts.Get<int         >("socket.HSSL2_SERVER_PORT");
        HSSL2_CLIENT_ADDR   = opts.Get<std::string >("socket.HSSL2_CLIENT_ADDR");
        HSSL2_CLIENT_PORT   = opts.Get<int         >("socket.HSSL2_CLIENT_PORT");
        LINK_FIFO_DEPTH     = opts.Get<unsigned int>("fifo.LINK_FIFO_DEPTH"    );
        LINK_FIFO_MAX_LEVEL = opts.Get<int         >("fifo.LINK_FIFO_MAX_LEVEL");
        LINK_FIFO_MIN_LEVEL = opts.Get<int         >("fifo.LINK_FIFO_MIN_LEVEL");
        // clang-format on
    }
}

static void send_signal_start_flow(GFiFo* fifo, GUdpClient* client) {
    GFiFo::fsm_state_t new_state, old_state;

    if (fifo->IsStateChanged(&new_state, &old_state)) {
        if (new_state == GFiFo::MIN_LEVEL_PASSED) {
            TPacketHead packet;
            packet.packet_type     = TPacketType::signal_start_flow;
            packet.file_id         = 0;
            packet.data_length     = 0;
            packet.current_segment = 1;
            packet.total_segments  = 1;

            client->Send(&packet, GPacket::PACKET_HEAD_SIZE);
        }
    }
}

static void send_signal_stop_flow(GFiFo* fifo, GUdpClient* client) {
    GFiFo::fsm_state_t new_state, old_state;

    if (fifo->IsStateChanged(&new_state, &old_state)) {
        if (new_state == GFiFo::MAX_LEVEL_PASSED) {
            TPacketHead packet;
            packet.packet_type     = TPacketType::signal_stop_flow;
            packet.file_id         = 0;
            packet.data_length     = 0;
            packet.current_segment = 1;
            packet.total_segments  = 1;

            client->Send(&packet, GPacket::PACKET_HEAD_SIZE);
        }
    }
}

static void log_server_statistics(const GDecoder* decoder, const char* func) {
    LOG_FORMAT(info, "[STAT] Message Packet Counter: %u (%s)", decoder->message.PacketCounter(), func);
    LOG_FORMAT(info, "[STAT] Message Errors Counter: %u (%s)", decoder->message.ErrorsCounter(), func);
    LOG_FORMAT(info, "[STAT] Message Missed Counter: %u (%s)", decoder->message.MissedCounter(), func);
}

static void f_gm_mc_server(bool& quit, GUdpServer& server, GUdpClient& client) {
    LOG_FORMAT(trace, "Thread STARTED (%s)", __func__);

    auto fifo{GFiFo(GPacket::PACKET_FULL_SIZE, LINK_FIFO_DEPTH, LINK_FIFO_MAX_LEVEL, LINK_FIFO_MIN_LEVEL)};

    volatile auto           _total{0};
    std::mutex              _mutex;
    std::condition_variable _event;

    // SECTION: decoder thread

    f_gm_mc::WorkerArgs args;
    args.quit   = &quit;
    args.client = &client;

    auto decoder{GDecoder(f_gm_mc::decode_packet, f_gm_mc::decode_message, args)};

    std::thread t_decoder([&] {
        while (!quit) {
            std::unique_lock _guard(_mutex);
            _event.wait(_guard, [&_total] { return _total > 0; });

            auto _new_data{fifo.Pop(decoder.packet_ptr(), decoder.packet_len()) > 0};
            send_signal_start_flow(&fifo, &client);

            _total--;
            _guard.unlock();

            if (_new_data) {
                decoder.Process();
            }
        }
        server.Stop();
    });

    // SECTION: socket thread

    uint8_t buffer[GUdpServer::MAX_DATAGRAM_SIZE];
    size_t  bytes;

    while (!quit) {
        if (server.Receive(buffer, &bytes)) {
            if (GPacket::IsValid(buffer, bytes)) {
                std::lock_guard _guard(_mutex);

                auto _new_data{fifo.Push(buffer, bytes)};
                send_signal_stop_flow(&fifo, &client);

                if (_new_data) {
                    _total++;
                    _event.notify_one();
                }
            }
            else {
                LOG_FORMAT(error, "Wrong packet format (%s)", __func__);
            }
        }
    }
    _total = 1;
    _event.notify_one();
    t_decoder.join();

    log_server_statistics(&decoder, __func__);
    LOG_FORMAT(trace, "Thread STOPPED (%s)", __func__);
}

static void f_gm_dh_server(const bool& quit, GUdpServer& server, GUdpClient& client) {
    LOG_FORMAT(trace, "Thread STARTED (%s)", __func__);

    auto fifo{GFiFo(GPacket::PACKET_FULL_SIZE, LINK_FIFO_DEPTH, LINK_FIFO_MAX_LEVEL, LINK_FIFO_MIN_LEVEL)};

    volatile auto           _total{0};
    std::mutex              _mutex;
    std::condition_variable _event;

    // SECTION: decoder thread

    f_gm_dh::WorkerArgs args;
    args.client = &client;

    auto decoder{GDecoder(f_gm_dh::decode_packet, f_gm_dh::decode_message, args)};

    std::thread t_decoder([&] {
        while (!quit) {
            std::unique_lock _guard(_mutex);
            _event.wait(_guard, [&_total] { return _total > 0; });

            auto _new_data{fifo.Pop(decoder.packet_ptr(), decoder.packet_len()) > 0};
            send_signal_start_flow(&fifo, &client);

            _total--;
            _guard.unlock();

            if (_new_data) {
                decoder.Process();
            }
        }
    });

    // SECTION: socket thread

    uint8_t buffer[GUdpServer::MAX_DATAGRAM_SIZE];
    size_t  bytes;

    while (!quit) {
        if (server.Receive(buffer, &bytes)) {
            if (GPacket::IsValid(buffer, bytes)) {
                std::lock_guard _guard(_mutex);

                auto _new_data{fifo.Push(buffer, bytes)};
                send_signal_stop_flow(&fifo, &client);

                if (_new_data) {
                    _total++;
                    _event.notify_one();
                }
            }
            else {
                LOG_FORMAT(error, "Wrong packet format (%s)", __func__);
            }
        }
    }
    _total = 1;
    _event.notify_one();
    t_decoder.join();

    log_server_statistics(&decoder, __func__);
    LOG_FORMAT(trace, "Thread STOPPED (%s)", __func__);
}

static void f_hssl1_server(const bool& quit, GUdpServer& server, GUdpClient& client) {
    LOG_FORMAT(trace, "Thread STARTED (%s)", __func__);

    auto fifo{GFiFo(GPacket::PACKET_FULL_SIZE, LINK_FIFO_DEPTH, LINK_FIFO_MAX_LEVEL, LINK_FIFO_MIN_LEVEL)};

    volatile auto           _total{0};
    std::mutex              _mutex;
    std::condition_variable _event;

    // SECTION: decoder thread

    f_hssl1::WorkerArgs args;
    args.client = &client;

    auto decoder{GDecoder(f_hssl1::decode_packet, f_hssl1::decode_message, args)};

    std::thread t_decoder([&] {
        while (!quit) {
            std::unique_lock _guard(_mutex);
            _event.wait(_guard, [&_total] { return _total > 0; });

            auto _new_data{fifo.Pop(decoder.packet_ptr(), decoder.packet_len()) > 0};
            send_signal_start_flow(&fifo, &client);

            _total--;
            _guard.unlock();

            if (_new_data) {
                decoder.Process();
            }
        }
    });

    // SECTION: socket thread

    uint8_t buffer[GUdpServer::MAX_DATAGRAM_SIZE];
    size_t  bytes;

    while (!quit) {
        if (server.Receive(buffer, &bytes)) {
            if (GPacket::IsValid(buffer, bytes)) {
                std::lock_guard _guard(_mutex);

                auto _new_data{fifo.Push(buffer, bytes)};
                send_signal_stop_flow(&fifo, &client);

                if (_new_data) {
                    _total++;
                    _event.notify_one();
                }
            }
            else {
                LOG_FORMAT(error, "Wrong packet format (%s)", __func__);
            }
        }
    }
    _total = 1;
    _event.notify_one();
    t_decoder.join();

    log_server_statistics(&decoder, __func__);
    LOG_FORMAT(trace, "Thread STOPPED (%s)", __func__);
}

static void f_hssl2_server(const bool& quit, GUdpServer& server, GUdpClient& client) {
    LOG_FORMAT(trace, "Thread STARTED (%s)", __func__);

    auto fifo{GFiFo(GPacket::PACKET_FULL_SIZE, LINK_FIFO_DEPTH, LINK_FIFO_MAX_LEVEL, LINK_FIFO_MIN_LEVEL)};

    volatile auto           _total{0};
    std::mutex              _mutex;
    std::condition_variable _event;

    // SECTION: decoder thread

    f_hssl2::WorkerArgs args;
    args.client = &client;

    auto decoder{GDecoder(f_hssl2::decode_packet, f_hssl2::decode_message, args)};

    std::thread t_decoder([&] {
        while (!quit) {
            std::unique_lock _guard(_mutex);
            _event.wait(_guard, [&_total] { return _total > 0; });

            auto _new_data{fifo.Pop(decoder.packet_ptr(), decoder.packet_len()) > 0};
            send_signal_start_flow(&fifo, &client);

            _total--;
            _guard.unlock();

            if (_new_data) {
                decoder.Process();
            }
        }
    });

    // SECTION: socket thread

    uint8_t buffer[GUdpServer::MAX_DATAGRAM_SIZE];
    size_t  bytes;

    while (!quit) {
        if (server.Receive(buffer, &bytes)) {
            if (GPacket::IsValid(buffer, bytes)) {
                std::lock_guard _guard(_mutex);

                auto _new_data{fifo.Push(buffer, bytes)};
                send_signal_stop_flow(&fifo, &client);

                if (_new_data) {
                    _total++;
                    _event.notify_one();
                }
            }
            else {
                LOG_FORMAT(error, "Wrong packet format (%s)", __func__);
            }
        }
    }
    _total = 1;
    _event.notify_one();
    t_decoder.join();

    log_server_statistics(&decoder, __func__);
    LOG_FORMAT(trace, "Thread STOPPED (%s)", __func__);
}

int main() {
    GLogger::Initialize("_mudp.log");
    LOG_WRITE(trace, "Process STARTED (main)");

    load_options("_mudp.cfg");

    auto gm_mc_server = GUdpServer(GM_MC_SERVER_ADDR.c_str(), GM_MC_SERVER_PORT, "GM-MC");
    auto gm_mc_client = GUdpClient(GM_MC_CLIENT_ADDR.c_str(), GM_MC_CLIENT_PORT, "GM-MC");
    auto gm_dh_server = GUdpServer(GM_DH_SERVER_ADDR.c_str(), GM_DH_SERVER_PORT, "GM-DH");
    auto gm_dh_client = GUdpClient(GM_DH_CLIENT_ADDR.c_str(), GM_DH_CLIENT_PORT, "GM-DH");
    auto hssl1_server = GUdpServer(HSSL1_SERVER_ADDR.c_str(), HSSL1_SERVER_PORT, "HSSL1");
    auto hssl1_client = GUdpClient(HSSL1_CLIENT_ADDR.c_str(), HSSL1_CLIENT_PORT, "HSSL1");
    auto hssl2_server = GUdpServer(HSSL2_SERVER_ADDR.c_str(), HSSL2_SERVER_PORT, "HSSL2");
    auto hssl2_client = GUdpClient(HSSL2_CLIENT_ADDR.c_str(), HSSL2_CLIENT_PORT, "HSSL2");

    auto quit{false};

    std::thread t_gm_mc_server(f_gm_mc_server, std::ref(quit), std::ref(gm_mc_server), std::ref(gm_mc_client));
    std::thread t_gm_dh_server(f_gm_dh_server, std::ref(quit), std::ref(gm_dh_server), std::ref(gm_dh_client));
    std::thread t_hssl1_server(f_hssl1_server, std::ref(quit), std::ref(hssl1_server), std::ref(hssl1_client));
    std::thread t_hssl2_server(f_hssl2_server, std::ref(quit), std::ref(hssl2_server), std::ref(hssl2_client));

    t_gm_mc_server.join();

    gm_dh_server.Stop();
    hssl1_server.Stop();
    hssl2_server.Stop();

    t_gm_dh_server.join();
    t_hssl1_server.join();
    t_hssl2_server.join();

    LOG_WRITE(trace, "Process STOPPED (main)");
    return 0;
}
