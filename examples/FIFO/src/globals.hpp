
////////////////////////////////////////////////////////////////////////////////
/// \file      globals.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "GArrayRoller.hpp"
#include "GFIFOdevice.hpp"
#include "GProfile.hpp"
#include "GUdpClient.hpp"
#include "GUdpServer.hpp"
#include "GWorksCoupler.hpp"

#define FIFO_WORD_SIZE sizeof(uint16_t)

// SECTION: PL_to_PS global variables
extern bool           RX_MODE_ENABLED;
extern unsigned int   RX_MODE_LOOPS;
extern std::string    RX_FILE_NAME;
extern unsigned int   RX_STREAM_ID;
extern unsigned char  RX_STREAM_TYPE;
extern std::string    RX_CLIENT_ADDR;
extern unsigned short RX_CLIENT_PORT;
extern unsigned int   RX_PACKET_WORDS;
extern std::string    RX_FIFO_TAG_NAME;
extern unsigned int   RX_FIFO_DEV_ADDR;
extern unsigned int   RX_FIFO_DEV_SIZE;
extern int            RX_FIFO_UIO_NUM;
extern int            RX_FIFO_UIO_MAP;
extern unsigned int   RX_ROLLER_NUMBER;
extern int            RX_ROLLER_MAX_LEVEL;
extern int            RX_ROLLER_MIM_LEVEL;

// SECTION: PS_to_PL global variables
extern bool           TX_MODE_ENABLED;
extern unsigned int   TX_MODE_LOOPS;
extern std::string    TX_FILE_NAME;
extern unsigned int   TX_STREAM_ID;
extern unsigned char  TX_STREAM_TYPE;
extern std::string    TX_SERVER_ADDR;
extern unsigned short TX_SERVER_PORT;
extern unsigned int   TX_PACKET_WORDS;
extern unsigned int   TX_EVENTS_WORDS;
extern std::string    TX_FIFO_TAG_NAME;
extern unsigned int   TX_FIFO_DEV_ADDR;
extern unsigned int   TX_FIFO_DEV_SIZE;
extern int            TX_FIFO_UIO_NUM;
extern int            TX_FIFO_UIO_MAP;
extern unsigned int   TX_ROLLER_NUMBER;
extern int            TX_ROLLER_MAX_LEVEL;
extern int            TX_ROLLER_MIM_LEVEL;

// =============================================================================

using g_array_roller_t  = GArrayRoller<uint16_t>;
using g_array_t         = GArray<uint16_t>;
using g_fifo_device_t   = GFIFOdevice;
using g_profile_t       = GProfile;
using g_udp_client_t    = GUdpClient;
using g_udp_server_t    = GUdpServer;
using g_works_coupler_t = GWorksCoupler;

typedef struct worker_args_t {
    unsigned int      loops_counter = 0;
    unsigned int      total_loops   = 0;
    unsigned long     total_bytes   = 0;
    g_udp_client_t*   client        = nullptr;
    g_udp_server_t*   server        = nullptr;
    g_fifo_device_t*  device        = nullptr;
    g_array_roller_t* roller        = nullptr;
    g_profile_t*      profile       = nullptr;

} worker_args_t;

typedef struct global_args_t {
    bool*             quit      = nullptr;
    g_udp_client_t*   rx_client = nullptr;
    g_udp_server_t*   tx_server = nullptr;
    g_fifo_device_t*  rx_device = nullptr;
    g_fifo_device_t*  tx_device = nullptr;
    g_array_roller_t* rx_roller = nullptr;
    g_array_roller_t* tx_roller = nullptr;

} global_args_t;

// =============================================================================

namespace Global {
    extern global_args_t args;

    void load_options(const std::string& filename);

    void reset_all();

    void quit_deamon();
} // namespace Global

#endif // GLOBALS_HPP
