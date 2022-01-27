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
#include "GUdpClient.hpp"
#include "GUdpServer.hpp"
#include "f_gm_mc.hpp"

#include <thread>

#define GM_MC_SERVER_ADDR "127.0.0.1"
#define GM_MC_SERVER_PORT 30001
#define GM_MC_CLIENT_ADDR "127.0.0.1"
#define GM_MC_CLIENT_PORT 30101

#define GM_DH_SERVER_ADDR "127.0.0.1"
#define GM_DH_SERVER_PORT 40001
#define GM_DH_CLIENT_ADDR "127.0.0.1"
#define GM_DH_CLIENT_PORT 40101

#define HSSL0_SERVER_ADDR "127.0.0.1"
#define HSSL0_SERVER_PORT 50001
#define HSSL0_CLIENT_ADDR "127.0.0.1"
#define HSSL0_CLIENT_PORT 50101

#define HSSL1_SERVER_ADDR "127.0.0.1"
#define HSSL1_SERVER_PORT 60001
#define HSSL1_CLIENT_ADDR "127.0.0.1"
#define HSSL1_CLIENT_PORT 60101

void f_gm_mc_server(bool &quit, GUdpServer &server, GUdpClient &client) {
    LOG_WRITE(trace, "Thread STARTED (gm_mc_server)");

    auto       fifo = GFiFo(GPacket::PACKET_FULL_SIZE, 20);
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
                    gate.unlock();
                }
            }
            else {
                LOG_FORMAT(error, "Wrong packet (%s)", __func__);
            }
        }
        else {
            quit = true;
        }
    }
    gate.unlock();
    t_decoder.join();

    LOG_WRITE(trace, "Thread STOPPED (gm_mc_server)");
}

void f_gm_dh_server(bool &quit, GUdpServer &server, GUdpClient &client) {
    LOG_WRITE(trace, "Thread STARTED (gm_dh_server)");

    uint8_t buffer[GUdpServer::MAX_DATAGRAM_SIZE];
    size_t  bytes;

    while (!quit) {
        if (server.Receive(buffer, &bytes)) {
        }
    }

    LOG_WRITE(trace, "Thread STOPPED (gm_dh_server)");
}

void f_hssl0_server(bool &quit, GUdpServer &server, GUdpClient &client) {
    LOG_WRITE(trace, "Thread STARTED (hssl0_server)");

    uint8_t buffer[GUdpServer::MAX_DATAGRAM_SIZE];
    size_t  bytes;

    while (!quit) {
        if (server.Receive(buffer, &bytes)) {
        }
    }

    LOG_WRITE(trace, "Thread STOPPED (hssl0_server)");
}

void f_hssl1_server(bool &quit, GUdpServer &server, GUdpClient &client) {
    LOG_WRITE(trace, "Thread STARTED (hssl1_server)");

    uint8_t buffer[GUdpServer::MAX_DATAGRAM_SIZE];
    size_t  bytes;

    while (!quit) {
        if (server.Receive(buffer, &bytes)) {
        }
    }

    LOG_WRITE(trace, "Thread STOPPED (hssl1_server)");
}

int main() {
    GLogger::Initialize("example_MUDP.log");
    LOG_WRITE(trace, "Process STARTED (main)");

    auto gm_mc_server = GUdpServer(GM_MC_SERVER_ADDR, GM_MC_SERVER_PORT, "GM-MC");
    auto gm_mc_client = GUdpClient(GM_MC_CLIENT_ADDR, GM_MC_CLIENT_PORT, "GM-MC");
    auto gm_dh_server = GUdpServer(GM_DH_SERVER_ADDR, GM_DH_SERVER_PORT, "GM-DH");
    auto gm_dh_client = GUdpClient(GM_DH_CLIENT_ADDR, GM_DH_CLIENT_PORT, "GM-DH");
    auto hssl0_server = GUdpServer(HSSL0_SERVER_ADDR, HSSL0_SERVER_PORT, "HSSL0");
    auto hssl0_client = GUdpClient(HSSL0_CLIENT_ADDR, HSSL0_CLIENT_PORT, "HSSL0");
    auto hssl1_server = GUdpServer(HSSL1_SERVER_ADDR, HSSL1_SERVER_PORT, "HSSL1");
    auto hssl1_client = GUdpClient(HSSL1_CLIENT_ADDR, HSSL1_CLIENT_PORT, "HSSL1");

    auto quit = false;

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
