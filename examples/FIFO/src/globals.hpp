
////////////////////////////////////////////////////////////////////////////////
/// \file      globals.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "GOptions.hpp"

#include <string> // string

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

// ============================================================================
// Common Options & Parameters
// ============================================================================

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

#endif // GLOBALS_HPP
