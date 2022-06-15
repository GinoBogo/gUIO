
////////////////////////////////////////////////////////////////////////////////
/// \file      globals.cpp
/// \version   0.1
/// \date      July, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "globals.hpp"

// SECTION: PL_to_PS global variables
bool           RX_MODE_ENABLED     = true;
unsigned int   RX_MODE_LOOPS       = 20;
std::string    RX_FILE_NAME        = "rx_words.bin";
unsigned int   RX_STREAM_ID        = 1;
unsigned char  RX_STREAM_TYPE      = 1;
std::string    RX_CLIENT_ADDR      = "127.0.0.1";
unsigned short RX_CLIENT_PORT      = 30001;
unsigned int   RX_PACKET_WORDS     = 1024;
std::string    RX_FIFO_TAG_NAME    = "RX";
unsigned int   RX_FIFO_DEV_ADDR    = 0xA0010000;
unsigned int   RX_FIFO_DEV_SIZE    = 4096;
int            RX_FIFO_UIO_NUM     = 1;
int            RX_FIFO_UIO_MAP     = 1;
unsigned int   RX_ROLLER_NUMBER    = 20;
int            RX_ROLLER_MAX_LEVEL = -1;
int            RX_ROLLER_MIM_LEVEL = -1;

// SECTION: PS_to_PL global variables
bool           TX_MODE_ENABLED     = true;
unsigned int   TX_MODE_LOOPS       = 20;
std::string    TX_FILE_NAME        = "tx_words.bin";
unsigned int   TX_STREAM_ID        = 2;
unsigned char  TX_STREAM_TYPE      = 2;
std::string    TX_SERVER_ADDR      = "127.0.0.1";
unsigned short TX_SERVER_PORT      = 30101;
unsigned int   TX_PACKET_WORDS     = 1024;
std::string    TX_FIFO_TAG_NAME    = "TX";
unsigned int   TX_FIFO_DEV_ADDR    = 0xA0020000;
unsigned int   TX_FIFO_DEV_SIZE    = 4096;
int            TX_FIFO_UIO_NUM     = 2;
int            TX_FIFO_UIO_MAP     = 2;
unsigned int   TX_ROLLER_NUMBER    = 20;
int            TX_ROLLER_MAX_LEVEL = -1;
int            TX_ROLLER_MIM_LEVEL = -1;

// ============================================================================

namespace Global {
    global_args_t args;

    void load_options(const char* filename) {
        auto opts = GOptions();

        // clang-format off
        opts.Insert<bool          >("PL_to_PS.RX_MODE_ENABLED"    , RX_MODE_ENABLED    );
        opts.Insert<unsigned int  >("PL_to_PS.RX_MODE_LOOPS"      , RX_MODE_LOOPS      );
        opts.Insert<std::string   >("PL_to_PS.RX_FILE_NAME"       , RX_FILE_NAME       );
        opts.Insert<unsigned int  >("PL_to_PS.RX_STREAM_ID"       , RX_STREAM_ID       );
        opts.Insert<unsigned char >("PL_to_PS.RX_STREAM_TYPE"     , RX_STREAM_TYPE     );
        opts.Insert<std::string   >("PL_to_PS.RX_CLIENT_ADDR"     , RX_CLIENT_ADDR     );
        opts.Insert<unsigned short>("PL_to_PS.RX_CLIENT_PORT"     , RX_CLIENT_PORT     );
        opts.Insert<unsigned int  >("PL_to_PS.RX_PACKET_WORDS"    , RX_PACKET_WORDS    );
        opts.Insert<std::string   >("PL_to_PS.RX_FIFO_TAG_NAME"   , RX_FIFO_TAG_NAME   );
        opts.Insert<unsigned int  >("PL_to_PS.RX_FIFO_DEV_ADDR"   , RX_FIFO_DEV_ADDR   );
        opts.Insert<unsigned int  >("PL_to_PS.RX_FIFO_DEV_SIZE"   , RX_FIFO_DEV_SIZE   );
        opts.Insert<int           >("PL_to_PS.RX_FIFO_UIO_NUM"    , RX_FIFO_UIO_NUM    );
        opts.Insert<int           >("PL_to_PS.RX_FIFO_UIO_MAP"    , RX_FIFO_UIO_MAP    );
        opts.Insert<unsigned int  >("PL_to_PS.RX_ROLLER_NUMBER"   , RX_ROLLER_NUMBER   );
        opts.Insert<int           >("PL_to_PS.RX_ROLLER_MAX_LEVEL", RX_ROLLER_MAX_LEVEL);
        opts.Insert<int           >("PL_to_PS.RX_ROLLER_MIM_LEVEL", RX_ROLLER_MIM_LEVEL);

        opts.Insert<bool          >("PS_to_PL.TX_MODE_ENABLED"    , TX_MODE_ENABLED    );
        opts.Insert<unsigned int  >("PS_to_PL.TX_MODE_LOOPS"      , TX_MODE_LOOPS      );
        opts.Insert<std::string   >("PS_to_PL.TX_FILE_NAME"       , TX_FILE_NAME       );
        opts.Insert<unsigned int  >("PS_to_PL.TX_STREAM_ID"       , TX_STREAM_ID       );
        opts.Insert<unsigned char >("PS_to_PL.TX_STREAM_TYPE"     , TX_STREAM_TYPE     );
        opts.Insert<std::string   >("PS_to_PL.TX_SERVER_ADDR"     , TX_SERVER_ADDR     );
        opts.Insert<unsigned short>("PS_to_PL.TX_SERVER_PORT"     , TX_SERVER_PORT     );
        opts.Insert<unsigned int  >("PS_to_PL.TX_PACKET_WORDS"    , TX_PACKET_WORDS    );
        opts.Insert<std::string   >("PS_to_PL.TX_FIFO_TAG_NAME"   , TX_FIFO_TAG_NAME   );
        opts.Insert<unsigned int  >("PS_to_PL.TX_FIFO_DEV_ADDR"   , TX_FIFO_DEV_ADDR   );
        opts.Insert<unsigned int  >("PS_to_PL.TX_FIFO_DEV_SIZE"   , TX_FIFO_DEV_SIZE   );
        opts.Insert<int           >("PS_to_PL.TX_FIFO_UIO_NUM"    , TX_FIFO_UIO_NUM    );
        opts.Insert<int           >("PS_to_PL.TX_FIFO_UIO_MAP"    , TX_FIFO_UIO_MAP    );
        opts.Insert<unsigned int  >("PS_to_PL.TX_ROLLER_NUMBER"   , TX_ROLLER_NUMBER   );
        opts.Insert<int           >("PS_to_PL.TX_ROLLER_MAX_LEVEL", TX_ROLLER_MAX_LEVEL);
        opts.Insert<int           >("PS_to_PL.TX_ROLLER_MIM_LEVEL", TX_ROLLER_MIM_LEVEL);
        // clang-format on

        if (opts.Read(filename)) {
            // clang-format off
            RX_MODE_ENABLED     = opts.Get<bool          >("PL_to_PS.RX_MODE_ENABLED"    );
            RX_MODE_LOOPS       = opts.Get<unsigned int  >("PL_to_PS.RX_MODE_LOOPS"      );
            RX_FILE_NAME        = opts.Get<std::string   >("PL_to_PS.RX_FILE_NAME"       );
            RX_STREAM_ID        = opts.Get<unsigned int  >("PL_to_PS.RX_STREAM_ID"       );
            RX_STREAM_TYPE      = opts.Get<unsigned char >("PL_to_PS.RX_STREAM_TYPE"     );
            RX_CLIENT_ADDR      = opts.Get<std::string   >("PL_to_PS.RX_CLIENT_ADDR"     );
            RX_CLIENT_PORT      = opts.Get<unsigned short>("PL_to_PS.RX_CLIENT_PORT"     );
            RX_PACKET_WORDS     = opts.Get<unsigned int  >("PL_to_PS.RX_PACKET_WORDS"    );
            RX_FIFO_TAG_NAME    = opts.Get<std::string   >("PL_to_PS.RX_FIFO_TAG_NAME"   );
            RX_FIFO_DEV_ADDR    = opts.Get<unsigned int  >("PL_to_PS.RX_FIFO_DEV_ADDR"   );
            RX_FIFO_DEV_SIZE    = opts.Get<unsigned int  >("PL_to_PS.RX_FIFO_DEV_SIZE"   );
            RX_FIFO_UIO_NUM     = opts.Get<int           >("PL_to_PS.RX_FIFO_UIO_NUM"    );
            RX_FIFO_UIO_MAP     = opts.Get<int           >("PL_to_PS.RX_FIFO_UIO_MAP"    );
            RX_ROLLER_NUMBER    = opts.Get<unsigned int  >("PL_to_PS.RX_ROLLER_NUMBER"   );
            RX_ROLLER_MAX_LEVEL = opts.Get<int           >("PL_to_PS.RX_ROLLER_MAX_LEVEL");
            RX_ROLLER_MIM_LEVEL = opts.Get<int           >("PL_to_PS.RX_ROLLER_MIM_LEVEL");

            TX_MODE_ENABLED     = opts.Get<bool          >("PS_to_PL.TX_MODE_ENABLED"    );
            TX_MODE_LOOPS       = opts.Get<unsigned int  >("PS_to_PL.TX_MODE_LOOPS"      );
            TX_FILE_NAME        = opts.Get<std::string   >("PS_to_PL.TX_FILE_NAME"       );
            TX_STREAM_ID        = opts.Get<unsigned int  >("PS_to_PL.TX_STREAM_ID"       );
            TX_STREAM_TYPE      = opts.Get<unsigned char >("PS_to_PL.TX_STREAM_TYPE"     );
            TX_SERVER_ADDR      = opts.Get<std::string   >("PS_to_PL.TX_SERVER_ADDR"     );
            TX_SERVER_PORT      = opts.Get<unsigned short>("PS_to_PL.TX_SERVER_PORT"     );
            TX_PACKET_WORDS     = opts.Get<unsigned int  >("PS_to_PL.TX_PACKET_WORDS"    );
            TX_FIFO_TAG_NAME    = opts.Get<std::string   >("PS_to_PL.TX_FIFO_TAG_NAME"   );
            TX_FIFO_DEV_ADDR    = opts.Get<unsigned int  >("PS_to_PL.TX_FIFO_DEV_ADDR"   );
            TX_FIFO_DEV_SIZE    = opts.Get<unsigned int  >("PS_to_PL.TX_FIFO_DEV_SIZE"   );
            TX_FIFO_UIO_NUM     = opts.Get<int           >("PS_to_PL.TX_FIFO_UIO_NUM"    );
            TX_FIFO_UIO_MAP     = opts.Get<int           >("PS_to_PL.TX_FIFO_UIO_MAP"    );
            TX_ROLLER_NUMBER    = opts.Get<unsigned int  >("PS_to_PL.TX_ROLLER_NUMBER"   );
            TX_ROLLER_MAX_LEVEL = opts.Get<int           >("PS_to_PL.TX_ROLLER_MAX_LEVEL");
            TX_ROLLER_MIM_LEVEL = opts.Get<int           >("PS_to_PL.TX_ROLLER_MIM_LEVEL");
            // clang-format on
        }
    }

    void reset_all() {
        args.rx_device->Reset();
        args.rx_device->ClearEvent();
        args.rx_roller->Reset();

        args.tx_device->Reset();
        args.tx_device->ClearEvent();
        args.tx_roller->Reset();
    }

    void quit_process() {
    }
} // namespace Global
