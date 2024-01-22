
////////////////////////////////////////////////////////////////////////////////
/// \file      globals.cpp
/// \version   0.1
/// \date      July, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "globals.hpp"

#include "GOptions.hpp" // GOPTIONS_GET, GOPTIONS_SET, GOptions
#include "GString.hpp"  // GString

#include <fstream> // ifstream

// SECTION: PL_to_PS global variables
std::string  PL2PS_REGS_TAG_NAME = "PL2PS";
std::string  PL2PS_REGS_CFG_FILE = "PL2PS_reg_values.cfg";
unsigned int PL2PS_REGS_DEV_ADDR = 0x00010000;
unsigned int PL2PS_REGS_DEV_SIZE = 512;
unsigned int PL2PS_REGS_DEV_NUMB = 64;

// SECTION: PS_to_PL global variables
std::string  PS2PL_REGS_TAG_NAME = "PS2PL";
std::string  PS2PL_REGS_CFG_FILE = "PS2PL_reg_values.cfg";
unsigned int PS2PL_REGS_DEV_ADDR = 0x00020000;
unsigned int PS2PL_REGS_DEV_SIZE = 512;
unsigned int PS2PL_REGS_DEV_NUMB = 64;

// =============================================================================

namespace Global {

    void load_options(const std::string& filename) {
        GOptions opts;

        // clang-format off
        GOPTIONS_SET(opts, "PL_to_PS", PL2PS_REGS_TAG_NAME);
        GOPTIONS_SET(opts, "PL_to_PS", PL2PS_REGS_CFG_FILE);
        GOPTIONS_SET(opts, "PL_to_PS", PL2PS_REGS_DEV_ADDR);
        GOPTIONS_SET(opts, "PL_to_PS", PL2PS_REGS_DEV_SIZE);
        GOPTIONS_SET(opts, "PL_to_PS", PL2PS_REGS_DEV_NUMB);

        GOPTIONS_SET(opts, "PS_to_PL", PS2PL_REGS_TAG_NAME);
        GOPTIONS_SET(opts, "PS_to_PL", PS2PL_REGS_CFG_FILE);
        GOPTIONS_SET(opts, "PS_to_PL", PS2PL_REGS_DEV_ADDR);
        GOPTIONS_SET(opts, "PS_to_PL", PS2PL_REGS_DEV_SIZE);
        GOPTIONS_SET(opts, "PS_to_PL", PS2PL_REGS_DEV_NUMB);

        RETURN_IF(!opts.Read(filename));

        GOPTIONS_GET(opts, "PL_to_PS", PL2PS_REGS_TAG_NAME);
        GOPTIONS_GET(opts, "PL_to_PS", PL2PS_REGS_CFG_FILE);
        GOPTIONS_GET(opts, "PL_to_PS", PL2PS_REGS_DEV_ADDR);
        GOPTIONS_GET(opts, "PL_to_PS", PL2PS_REGS_DEV_SIZE);
        GOPTIONS_GET(opts, "PL_to_PS", PL2PS_REGS_DEV_NUMB);

        GOPTIONS_GET(opts, "PS_to_PL", PS2PL_REGS_TAG_NAME);
        GOPTIONS_GET(opts, "PS_to_PL", PS2PL_REGS_CFG_FILE);
        GOPTIONS_GET(opts, "PS_to_PL", PS2PL_REGS_DEV_ADDR);
        GOPTIONS_GET(opts, "PS_to_PL", PS2PL_REGS_DEV_SIZE);
        GOPTIONS_GET(opts, "PS_to_PL", PS2PL_REGS_DEV_NUMB);
        // clang-format on
    }

    static GMAPdevice::reg_access_t decode_access(const std::string& item) {
        if (item == "R") {
            return GMAPdevice::READ_ONLY;
        }
        if (item == "W") {
            return GMAPdevice::WRITE_ONLY;
        }
        return GMAPdevice::READ_WRITE;
    }

    GMAPdevice::reg_list_t load_registers(const std::string& filename) {
        GMAPdevice::reg_list_t reg_list;

        std::ifstream _fs(filename);

        if (_fs.is_open()) {
            std::string _line;

            while (!_fs.eof()) {
                std::getline(_fs, _line);

                auto items{GString::split(_line, "[ \\t]")};

                GString::sanitize(items);

                switch (items.size()) {
                    case 2: {
                        GMAPdevice::reg_pair_t reg_pair;
                        reg_pair.offset = GString::strtoui(items[0]);
                        reg_pair.value  = GString::strtoui(items[1]);

                        reg_list.push_back(reg_pair);
                        break;
                    }

                    case 3: {
                        GMAPdevice::reg_pair_t reg_pair;
                        reg_pair.access = decode_access(items[2]);
                        reg_pair.offset = GString::strtoui(items[0]);
                        reg_pair.value  = GString::strtoui(items[1]);

                        reg_list.push_back(reg_pair);
                        break;
                    }

                    default:
                        break;
                }
            }
        }

        return reg_list;
    }

} // namespace Global
