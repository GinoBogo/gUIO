
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
extern std::string    TX_FIFO_TAG_NAME;
extern unsigned int   TX_FIFO_DEV_ADDR;
extern unsigned int   TX_FIFO_DEV_SIZE;
extern int            TX_FIFO_UIO_NUM;
extern int            TX_FIFO_UIO_MAP;
extern unsigned int   TX_ROLLER_NUMBER;
extern int            TX_ROLLER_MAX_LEVEL;
extern int            TX_ROLLER_MIM_LEVEL;

// ============================================================================

void load_options(const char* filename);

#endif // GLOBALS_HPP
