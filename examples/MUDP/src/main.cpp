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
#include "f_hssl0.hpp"
#include "f_hssl1.hpp"

#include <thread> // thread, mutex

void send_packet_start_flow(GFiFo& fifo, GUdpClient& client) {
    GFiFo::fsm_state_t new_state, old_state;

    if (fifo.IsStateChanged(&new_state, &old_state)) {
        if (new_state == GFiFo::MIN_LEVEL_PASSED) {
            TPacketHead packet;
            packet.packet_type     = TPacketType::packet_start_flow;
            packet.file_id         = 0;
            packet.data_length     = 0;
            packet.current_segment = 1;
            packet.total_segments  = 1;

            client.Send(&packet, GPacket::PACKET_HEAD_SIZE);
        }
    }
}

void send_packet_stop_flow(GFiFo& fifo, GUdpClient& client) {
    GFiFo::fsm_state_t new_state, old_state;

    if (fifo.IsStateChanged(&new_state, &old_state)) {
        if (new_state == GFiFo::MAX_LEVEL_PASSED) {
            TPacketHead packet;
            packet.packet_type     = TPacketType::packet_stop_flow;
            packet.file_id         = 0;
            packet.data_length     = 0;
            packet.current_segment = 1;
            packet.total_segments  = 1;

            client.Send(&packet, GPacket::PACKET_HEAD_SIZE);
        }
    }
}

void f_gm_mc_server(bool& quit, GUdpServer& server, GUdpClient& client) {
    LOG_WRITE(trace, "Thread STARTED (gm_mc_server)");

    auto       fifo = GFiFo(GPacket::PACKET_FULL_SIZE, 20, 15, 5);
    std::mutex gate;

    // SECTION: decoder thread

    f_gm_mc::WorkerArgs args;
    args.quit   = &quit;
    args.client = &client;

    auto decoder = GDecoder(f_gm_mc::decode_packet, f_gm_mc::decode_message, args);

    std::thread t_decoder([&]() {
        while (!quit) {
            gate.lock();

            while (!fifo.IsEmpty()) {
                if (fifo.Pop(decoder.packet_ptr(), decoder.packet_len()) > 0) {
                    send_packet_start_flow(fifo, client);
                    decoder.Process();
                }
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
                if (fifo.Push(buffer, bytes)) {
                    send_packet_stop_flow(fifo, client);
                    gate.unlock();
                }
            }
            else {
                LOG_FORMAT(error, "Wrong packet format (%s)", __func__);
            }
        }
    }
    gate.unlock();
    t_decoder.join();

    LOG_WRITE(trace, "Thread STOPPED (gm_mc_server)");
}

void f_gm_dh_server(bool& quit, GUdpServer& server, GUdpClient& client) {
    LOG_WRITE(trace, "Thread STARTED (gm_dh_server)");

    auto       fifo = GFiFo(GPacket::PACKET_FULL_SIZE, 20, 15, 5);
    std::mutex gate;

    // SECTION: decoder thread

    f_gm_dh::WorkerArgs args;
    args.client = &client;

    auto decoder = GDecoder(f_gm_dh::decode_packet, f_gm_dh::decode_message, args);

    std::thread t_decoder([&]() {
        while (!quit) {
            gate.lock();

            while (!fifo.IsEmpty()) {
                if (fifo.Pop(decoder.packet_ptr(), decoder.packet_len()) > 0) {
                    send_packet_start_flow(fifo, client);
                    decoder.Process();
                }
            }
        }
    });

    // SECTION: socket thread

    uint8_t buffer[GUdpServer::MAX_DATAGRAM_SIZE];
    size_t  bytes;

    while (!quit) {
        if (server.Receive(buffer, &bytes)) {
            if (GPacket::IsValid(buffer, bytes)) {
                if (fifo.Push(buffer, bytes)) {
                    send_packet_stop_flow(fifo, client);
                    gate.unlock();
                }
            }
            else {
                LOG_FORMAT(error, "Wrong packet format (%s)", __func__);
            }
        }
    }
    gate.unlock();
    t_decoder.join();

    LOG_WRITE(trace, "Thread STOPPED (gm_dh_server)");
}

void f_hssl0_server(bool& quit, GUdpServer& server, GUdpClient& client) {
    LOG_WRITE(trace, "Thread STARTED (hssl0_server)");

    auto       fifo = GFiFo(GPacket::PACKET_FULL_SIZE, 20, 15, 5);
    std::mutex gate;

    // SECTION: decoder thread

    f_hssl0::WorkerArgs args;
    args.client = &client;

    auto decoder = GDecoder(f_hssl0::decode_packet, f_hssl0::decode_message, args);

    std::thread t_decoder([&]() {
        while (!quit) {
            gate.lock();

            while (!fifo.IsEmpty()) {
                if (fifo.Pop(decoder.packet_ptr(), decoder.packet_len()) > 0) {
                    send_packet_start_flow(fifo, client);
                    decoder.Process();
                }
            }
        }
    });

    // SECTION: socket thread

    uint8_t buffer[GUdpServer::MAX_DATAGRAM_SIZE];
    size_t  bytes;

    while (!quit) {
        if (server.Receive(buffer, &bytes)) {
            if (GPacket::IsValid(buffer, bytes)) {
                if (fifo.Push(buffer, bytes)) {
                    send_packet_stop_flow(fifo, client);
                    gate.unlock();
                }
            }
            else {
                LOG_FORMAT(error, "Wrong packet format (%s)", __func__);
            }
        }
    }
    gate.unlock();
    t_decoder.join();

    LOG_WRITE(trace, "Thread STOPPED (hssl0_server)");
}

void f_hssl1_server(bool& quit, GUdpServer& server, GUdpClient& client) {
    LOG_WRITE(trace, "Thread STARTED (hssl1_server)");

    auto       fifo = GFiFo(GPacket::PACKET_FULL_SIZE, 20, 15, 5);
    std::mutex gate;

    // SECTION: decoder thread

    f_hssl1::WorkerArgs args;
    args.client = &client;

    auto decoder = GDecoder(f_hssl1::decode_packet, f_hssl1::decode_message, args);

    std::thread t_decoder([&]() {
        while (!quit) {
            gate.lock();

            while (!fifo.IsEmpty()) {
                if (fifo.Pop(decoder.packet_ptr(), decoder.packet_len()) > 0) {
                    send_packet_start_flow(fifo, client);
                    decoder.Process();
                }
            }
        }
    });

    // SECTION: socket thread

    uint8_t buffer[GUdpServer::MAX_DATAGRAM_SIZE];
    size_t  bytes;

    while (!quit) {
        if (server.Receive(buffer, &bytes)) {
            if (GPacket::IsValid(buffer, bytes)) {
                if (fifo.Push(buffer, bytes)) {
                    send_packet_stop_flow(fifo, client);
                    gate.unlock();
                }
            }
            else {
                LOG_FORMAT(error, "Wrong packet format (%s)", __func__);
            }
        }
    }
    gate.unlock();
    t_decoder.join();

    LOG_WRITE(trace, "Thread STOPPED (hssl1_server)");
}

std::string GM_MC_SERVER_ADDR = "127.0.0.1";
int         GM_MC_SERVER_PORT = 30001;
std::string GM_MC_CLIENT_ADDR = "127.0.0.1";
int         GM_MC_CLIENT_PORT = 30101;
std::string GM_DH_SERVER_ADDR = "127.0.0.1";
int         GM_DH_SERVER_PORT = 40001;
std::string GM_DH_CLIENT_ADDR = "127.0.0.1";
int         GM_DH_CLIENT_PORT = 40101;
std::string HSSL0_SERVER_ADDR = "127.0.0.1";
int         HSSL0_SERVER_PORT = 50001;
std::string HSSL0_CLIENT_ADDR = "127.0.0.1";
int         HSSL0_CLIENT_PORT = 50101;
std::string HSSL1_SERVER_ADDR = "127.0.0.1";
int         HSSL1_SERVER_PORT = 60001;
std::string HSSL1_CLIENT_ADDR = "127.0.0.1";
int         HSSL1_CLIENT_PORT = 60101;

void load_options(const char* filename) {
    auto opts = GOptions();

    // clang-format off
    opts.Insert<std::string>("socket.GM_MC_SERVER_ADDR", GM_MC_SERVER_ADDR);
    opts.Insert<int        >("socket.GM_MC_SERVER_PORT", GM_MC_SERVER_PORT);
    opts.Insert<std::string>("socket.GM_MC_CLIENT_ADDR", GM_MC_CLIENT_ADDR);
    opts.Insert<int        >("socket.GM_MC_CLIENT_PORT", GM_MC_CLIENT_PORT);
    opts.Insert<std::string>("socket.GM_DH_SERVER_ADDR", GM_DH_SERVER_ADDR);
    opts.Insert<int        >("socket.GM_DH_SERVER_PORT", GM_DH_SERVER_PORT);
    opts.Insert<std::string>("socket.GM_DH_CLIENT_ADDR", GM_DH_CLIENT_ADDR);
    opts.Insert<int        >("socket.GM_DH_CLIENT_PORT", GM_DH_CLIENT_PORT);
    opts.Insert<std::string>("socket.HSSL0_SERVER_ADDR", HSSL0_SERVER_ADDR);
    opts.Insert<int        >("socket.HSSL0_SERVER_PORT", HSSL0_SERVER_PORT);
    opts.Insert<std::string>("socket.HSSL0_CLIENT_ADDR", HSSL0_CLIENT_ADDR);
    opts.Insert<int        >("socket.HSSL0_CLIENT_PORT", HSSL0_CLIENT_PORT);
    opts.Insert<std::string>("socket.HSSL1_SERVER_ADDR", HSSL1_SERVER_ADDR);
    opts.Insert<int        >("socket.HSSL1_SERVER_PORT", HSSL1_SERVER_PORT);
    opts.Insert<std::string>("socket.HSSL1_CLIENT_ADDR", HSSL1_CLIENT_ADDR);
    opts.Insert<int        >("socket.HSSL1_CLIENT_PORT", HSSL1_CLIENT_PORT);
    // clang-format on

    if (opts.Read(filename)) {
        // clang-format off
        GM_MC_SERVER_ADDR = opts.Get<std::string>("socket.GM_MC_SERVER_ADDR");
        GM_MC_SERVER_PORT = opts.Get<int        >("socket.GM_MC_SERVER_PORT");
        GM_MC_CLIENT_ADDR = opts.Get<std::string>("socket.GM_MC_CLIENT_ADDR");
        GM_MC_CLIENT_PORT = opts.Get<int        >("socket.GM_MC_CLIENT_PORT");
        GM_DH_SERVER_ADDR = opts.Get<std::string>("socket.GM_DH_SERVER_ADDR");
        GM_DH_SERVER_PORT = opts.Get<int        >("socket.GM_DH_SERVER_PORT");
        GM_DH_CLIENT_ADDR = opts.Get<std::string>("socket.GM_DH_CLIENT_ADDR");
        GM_DH_CLIENT_PORT = opts.Get<int        >("socket.GM_DH_CLIENT_PORT");
        HSSL0_SERVER_ADDR = opts.Get<std::string>("socket.HSSL0_SERVER_ADDR");
        HSSL0_SERVER_PORT = opts.Get<int        >("socket.HSSL0_SERVER_PORT");
        HSSL0_CLIENT_ADDR = opts.Get<std::string>("socket.HSSL0_CLIENT_ADDR");
        HSSL0_CLIENT_PORT = opts.Get<int        >("socket.HSSL0_CLIENT_PORT");
        HSSL1_SERVER_ADDR = opts.Get<std::string>("socket.HSSL1_SERVER_ADDR");
        HSSL1_SERVER_PORT = opts.Get<int        >("socket.HSSL1_SERVER_PORT");
        HSSL1_CLIENT_ADDR = opts.Get<std::string>("socket.HSSL1_CLIENT_ADDR");
        HSSL1_CLIENT_PORT = opts.Get<int        >("socket.HSSL1_CLIENT_PORT");
        // clang-format on
    }
}

int main() {
    GLogger::Initialize("_mudp.log");
    LOG_WRITE(trace, "Process STARTED (main)");

    load_options("example_MUDP.cfg");

    auto gm_mc_server = GUdpServer(GM_MC_SERVER_ADDR.c_str(), GM_MC_SERVER_PORT, "GM-MC");
    auto gm_mc_client = GUdpClient(GM_MC_CLIENT_ADDR.c_str(), GM_MC_CLIENT_PORT, "GM-MC");
    auto gm_dh_server = GUdpServer(GM_DH_SERVER_ADDR.c_str(), GM_DH_SERVER_PORT, "GM-DH");
    auto gm_dh_client = GUdpClient(GM_DH_CLIENT_ADDR.c_str(), GM_DH_CLIENT_PORT, "GM-DH");
    auto hssl0_server = GUdpServer(HSSL0_SERVER_ADDR.c_str(), HSSL0_SERVER_PORT, "HSSL0");
    auto hssl0_client = GUdpClient(HSSL0_CLIENT_ADDR.c_str(), HSSL0_CLIENT_PORT, "HSSL0");
    auto hssl1_server = GUdpServer(HSSL1_SERVER_ADDR.c_str(), HSSL1_SERVER_PORT, "HSSL1");
    auto hssl1_client = GUdpClient(HSSL1_CLIENT_ADDR.c_str(), HSSL1_CLIENT_PORT, "HSSL1");

    auto quit{false};

    std::thread t_gm_mc_server(f_gm_mc_server, std::ref(quit), std::ref(gm_mc_server), std::ref(gm_mc_client));
    std::thread t_gm_dh_server(f_gm_dh_server, std::ref(quit), std::ref(gm_dh_server), std::ref(gm_dh_client));
    std::thread t_hssl0_server(f_hssl0_server, std::ref(quit), std::ref(hssl0_server), std::ref(hssl0_client));
    std::thread t_hssl1_server(f_hssl1_server, std::ref(quit), std::ref(hssl1_server), std::ref(hssl1_client));

    t_gm_mc_server.join();

    gm_dh_server.Stop();
    hssl0_server.Stop();
    hssl1_server.Stop();

    t_gm_dh_server.join();
    t_hssl0_server.join();
    t_hssl1_server.join();

    LOG_WRITE(trace, "Process STOPPED (main)");
    return 0;
}
