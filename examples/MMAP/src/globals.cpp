
////////////////////////////////////////////////////////////////////////////////
/// \file      globals.cpp
/// \version   0.1
/// \date      July, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "globals.hpp"

// SECTION: PL_to_PS global variables
std::string  PL2PS_REGS_TAG_NAME = "PL2PS";
unsigned int PL2PS_REGS_DEV_ADDR = 0x00010000;
unsigned int PL2PS_REGS_DEV_SIZE = 512;
unsigned int PL2PS_REGS_DEV_NUMB = 64;

// SECTION: PS_to_PL global variables
std::string  PS2PL_REGS_TAG_NAME = "PS2PL";
unsigned int PS2PL_REGS_DEV_ADDR = 0x00020000;
unsigned int PS2PL_REGS_DEV_SIZE = 512;
unsigned int PS2PL_REGS_DEV_NUMB = 64;

// ============================================================================

namespace Global {

    void load_options(const std::string& filename) {
        GOptions opts;

        // clang-format off
        GOPTIONS_SET(opts, "PL_to_PS", PL2PS_REGS_TAG_NAME);
        GOPTIONS_SET(opts, "PL_to_PS", PL2PS_REGS_DEV_ADDR);
        GOPTIONS_SET(opts, "PL_to_PS", PL2PS_REGS_DEV_SIZE);
        GOPTIONS_SET(opts, "PL_to_PS", PL2PS_REGS_DEV_NUMB);

        GOPTIONS_SET(opts, "PS_to_PL", PS2PL_REGS_TAG_NAME);
        GOPTIONS_SET(opts, "PS_to_PL", PS2PL_REGS_DEV_ADDR);
        GOPTIONS_SET(opts, "PS_to_PL", PS2PL_REGS_DEV_SIZE);
        GOPTIONS_SET(opts, "PS_to_PL", PS2PL_REGS_DEV_NUMB);

        RETURN_IF(!opts.Read(filename));

        GOPTIONS_GET(opts, "PL_to_PS", PL2PS_REGS_TAG_NAME);
        GOPTIONS_GET(opts, "PL_to_PS", PL2PS_REGS_DEV_ADDR);
        GOPTIONS_GET(opts, "PL_to_PS", PL2PS_REGS_DEV_SIZE);
        GOPTIONS_GET(opts, "PL_to_PS", PL2PS_REGS_DEV_NUMB);

        GOPTIONS_GET(opts, "PS_to_PL", PS2PL_REGS_TAG_NAME);
        GOPTIONS_GET(opts, "PS_to_PL", PS2PL_REGS_DEV_ADDR);
        GOPTIONS_GET(opts, "PS_to_PL", PS2PL_REGS_DEV_SIZE);
        GOPTIONS_GET(opts, "PS_to_PL", PS2PL_REGS_DEV_NUMB);
        // clang-format on
    }

} // namespace Global
