
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
unsigned int   TX_EVENTS_WORDS     = 511;
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

    void load_options(const std::string& filename) {
        GOptions opts;

        // clang-format off
        GOPTIONS_SET(opts, "PL_to_PS", RX_MODE_ENABLED    );
        GOPTIONS_SET(opts, "PL_to_PS", RX_MODE_LOOPS      );
        GOPTIONS_SET(opts, "PL_to_PS", RX_FILE_NAME       );
        GOPTIONS_SET(opts, "PL_to_PS", RX_STREAM_ID       );
        GOPTIONS_SET(opts, "PL_to_PS", RX_STREAM_TYPE     );
        GOPTIONS_SET(opts, "PL_to_PS", RX_CLIENT_ADDR     );
        GOPTIONS_SET(opts, "PL_to_PS", RX_CLIENT_PORT     );
        GOPTIONS_SET(opts, "PL_to_PS", RX_PACKET_WORDS    );
        GOPTIONS_SET(opts, "PL_to_PS", RX_FIFO_TAG_NAME   );
        GOPTIONS_SET(opts, "PL_to_PS", RX_FIFO_DEV_ADDR   );
        GOPTIONS_SET(opts, "PL_to_PS", RX_FIFO_DEV_SIZE   );
        GOPTIONS_SET(opts, "PL_to_PS", RX_FIFO_UIO_NUM    );
        GOPTIONS_SET(opts, "PL_to_PS", RX_FIFO_UIO_MAP    );
        GOPTIONS_SET(opts, "PL_to_PS", RX_ROLLER_NUMBER   );
        GOPTIONS_SET(opts, "PL_to_PS", RX_ROLLER_MAX_LEVEL);
        GOPTIONS_SET(opts, "PL_to_PS", RX_ROLLER_MIM_LEVEL);
        
        GOPTIONS_SET(opts, "PS_to_PL", TX_MODE_ENABLED    );
        GOPTIONS_SET(opts, "PS_to_PL", TX_MODE_LOOPS      );
        GOPTIONS_SET(opts, "PS_to_PL", TX_FILE_NAME       );
        GOPTIONS_SET(opts, "PS_to_PL", TX_STREAM_ID       );
        GOPTIONS_SET(opts, "PS_to_PL", TX_STREAM_TYPE     );
        GOPTIONS_SET(opts, "PS_to_PL", TX_SERVER_ADDR     );
        GOPTIONS_SET(opts, "PS_to_PL", TX_SERVER_PORT     );
        GOPTIONS_SET(opts, "PS_to_PL", TX_PACKET_WORDS    );
        GOPTIONS_SET(opts, "PS_to_PL", TX_EVENTS_WORDS    );
        GOPTIONS_SET(opts, "PS_to_PL", TX_FIFO_TAG_NAME   );
        GOPTIONS_SET(opts, "PS_to_PL", TX_FIFO_DEV_ADDR   );
        GOPTIONS_SET(opts, "PS_to_PL", TX_FIFO_DEV_SIZE   );
        GOPTIONS_SET(opts, "PS_to_PL", TX_FIFO_UIO_NUM    );
        GOPTIONS_SET(opts, "PS_to_PL", TX_FIFO_UIO_MAP    );
        GOPTIONS_SET(opts, "PS_to_PL", TX_ROLLER_NUMBER   );
        GOPTIONS_SET(opts, "PS_to_PL", TX_ROLLER_MAX_LEVEL);
        GOPTIONS_SET(opts, "PS_to_PL", TX_ROLLER_MIM_LEVEL);

        RETURN_IF(!opts.Read(filename));

        GOPTIONS_GET(opts, "PL_to_PS", RX_MODE_ENABLED    );
        GOPTIONS_GET(opts, "PL_to_PS", RX_MODE_LOOPS      );
        GOPTIONS_GET(opts, "PL_to_PS", RX_FILE_NAME       );
        GOPTIONS_GET(opts, "PL_to_PS", RX_STREAM_ID       );
        GOPTIONS_GET(opts, "PL_to_PS", RX_STREAM_TYPE     );
        GOPTIONS_GET(opts, "PL_to_PS", RX_CLIENT_ADDR     );
        GOPTIONS_GET(opts, "PL_to_PS", RX_CLIENT_PORT     );
        GOPTIONS_GET(opts, "PL_to_PS", RX_PACKET_WORDS    );
        GOPTIONS_GET(opts, "PL_to_PS", RX_FIFO_TAG_NAME   );
        GOPTIONS_GET(opts, "PL_to_PS", RX_FIFO_DEV_ADDR   );
        GOPTIONS_GET(opts, "PL_to_PS", RX_FIFO_DEV_SIZE   );
        GOPTIONS_GET(opts, "PL_to_PS", RX_FIFO_UIO_NUM    );
        GOPTIONS_GET(opts, "PL_to_PS", RX_FIFO_UIO_MAP    );
        GOPTIONS_GET(opts, "PL_to_PS", RX_ROLLER_NUMBER   );
        GOPTIONS_GET(opts, "PL_to_PS", RX_ROLLER_MAX_LEVEL);
        GOPTIONS_GET(opts, "PL_to_PS", RX_ROLLER_MIM_LEVEL);
        
        GOPTIONS_GET(opts, "PS_to_PL", TX_MODE_ENABLED    );
        GOPTIONS_GET(opts, "PS_to_PL", TX_MODE_LOOPS      );
        GOPTIONS_GET(opts, "PS_to_PL", TX_FILE_NAME       );
        GOPTIONS_GET(opts, "PS_to_PL", TX_STREAM_ID       );
        GOPTIONS_GET(opts, "PS_to_PL", TX_STREAM_TYPE     );
        GOPTIONS_GET(opts, "PS_to_PL", TX_SERVER_ADDR     );
        GOPTIONS_GET(opts, "PS_to_PL", TX_SERVER_PORT     );
        GOPTIONS_GET(opts, "PS_to_PL", TX_PACKET_WORDS    );
        GOPTIONS_GET(opts, "PS_to_PL", TX_EVENTS_WORDS    );
        GOPTIONS_GET(opts, "PS_to_PL", TX_FIFO_TAG_NAME   );
        GOPTIONS_GET(opts, "PS_to_PL", TX_FIFO_DEV_ADDR   );
        GOPTIONS_GET(opts, "PS_to_PL", TX_FIFO_DEV_SIZE   );
        GOPTIONS_GET(opts, "PS_to_PL", TX_FIFO_UIO_NUM    );
        GOPTIONS_GET(opts, "PS_to_PL", TX_FIFO_UIO_MAP    );
        GOPTIONS_GET(opts, "PS_to_PL", TX_ROLLER_NUMBER   );
        GOPTIONS_GET(opts, "PS_to_PL", TX_ROLLER_MAX_LEVEL);
        GOPTIONS_GET(opts, "PS_to_PL", TX_ROLLER_MIM_LEVEL);
        // clang-format on
    }

    void reset_all() {
        DO_BLOCK_IF(RX_MODE_ENABLED, //
                    args.rx_device->Reset();
                    args.rx_device->ClearEvent();
                    args.rx_roller->Reset());

        DO_BLOCK_IF(TX_MODE_ENABLED, //
                    args.tx_device->Reset();
                    args.tx_device->ClearEvent();
                    args.tx_roller->Reset());
    }

    void quit_deamon() {
        RETURN_IF_OR(*args.quit, *args.quit = true);

        DO_BLOCK_IF(RX_MODE_ENABLED, args.rx_device->ClearEvent());

        DO_BLOCK_IF(TX_MODE_ENABLED, args.tx_server->Stop(); args.tx_device->ClearEvent());
    }
} // namespace Global
