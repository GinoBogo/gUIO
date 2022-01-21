////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GLogger.hpp"
#include "GPacket.hpp"
#include "GUdpClient.hpp"
#include "GUdpServer.hpp"

#include <thread>

#define CTRLX_SERVER_ADDR "127.0.0.1"
#define CTRLX_SERVER_PORT 40001
#define CTRLX_CLIENT_ADDR "127.0.0.1"
#define CTRLX_CLIENT_PORT 40101

#define HSSL0_SERVER_ADDR "127.0.0.1"
#define HSSL0_SERVER_PORT 50001
#define HSSL0_CLIENT_ADDR "127.0.0.1"
#define HSSL0_CLIENT_PORT 50101

#define HSSL1_SERVER_ADDR "127.0.0.1"
#define HSSL1_SERVER_PORT 60001
#define HSSL1_CLIENT_ADDR "127.0.0.1"
#define HSSL1_CLIENT_PORT 60101

void f_ctrlx_server(GUdpServer &p_server, GUdpClient &p_client) {
    LOG_WRITE(trace, "Thread STARTED (ctrlx_server)");

    uint8_t buffer[GUdpServer::MAX_DATAGRAM_SIZE];
    size_t  bytes;

    while (p_server.Receive(buffer, &bytes)) {

        if (GPacket::IsValid(buffer, bytes)) {

            auto packet = (TPacket *)buffer;

            if (GPacket::IsSingle(packet)) {
                // GMessage decode
                continue;
            }

            if (GPacket::IsFirst(packet)) {
                // GMessage reset
                // GMessage append
                continue;
            }

            if (GPacket::IsLast(packet)) {
                // GMessage append
                // GMessage decode
                continue;
            }

            if (GPacket::IsMiddle(packet)) {
                // GMessage append
            }
        }
        else {
            LOG_WRITE(error, "Wrong packet");
        }
    }

    LOG_WRITE(trace, "Thread STOPPED (ctrlx_server)");
}

void f_hssl0_server(GUdpServer &p_server, GUdpClient &p_client) {
    LOG_WRITE(trace, "Thread STARTED (hssl0_server)");

    LOG_WRITE(trace, "Thread STOPPED (hssl0_server)");
}

void f_hssl1_server(GUdpServer &p_server, GUdpClient &p_client) {
    LOG_WRITE(trace, "Thread STARTED (hssl1_server)");

    LOG_WRITE(trace, "Thread STOPPED (hssl1_server)");
}

int main() {
    GLogger::Initialize("example_MUDP.log");
    LOG_WRITE(trace, "Process STARTED (main)");

    auto ctrlx_server = GUdpServer(CTRLX_SERVER_ADDR, CTRLX_SERVER_PORT, "CTRL x");
    auto ctrlx_client = GUdpClient(CTRLX_CLIENT_ADDR, CTRLX_CLIENT_PORT, "CTRL x");
    auto hssl0_server = GUdpServer(HSSL0_SERVER_ADDR, HSSL0_SERVER_PORT, "HSSL 0");
    auto hssl0_client = GUdpClient(HSSL0_CLIENT_ADDR, HSSL0_CLIENT_PORT, "HSSL 0");
    auto hssl1_server = GUdpServer(HSSL1_SERVER_ADDR, HSSL1_SERVER_PORT, "HSSL 1");
    auto hssl1_client = GUdpClient(HSSL1_CLIENT_ADDR, HSSL1_CLIENT_PORT, "HSSL 1");

    std::thread t_ctrlx_server(f_ctrlx_server, std::ref(ctrlx_server), std::ref(ctrlx_client));
    std::thread t_hssl0_server(f_hssl0_server, std::ref(hssl0_server), std::ref(hssl0_client));
    std::thread t_hssl1_server(f_hssl1_server, std::ref(hssl1_server), std::ref(hssl1_client));

    t_ctrlx_server.join();
    t_hssl0_server.join();
    t_hssl1_server.join();

    LOG_WRITE(trace, "Process STOPPED (main)");
    return 0;
}
