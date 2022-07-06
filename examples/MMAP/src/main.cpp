
////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GLogger.hpp"
#include "GMAPdevice.hpp"
#include "GRegisters.hpp"
#include "globals.hpp"

#include <cstring>    // memset
#include <filesystem> // path

int main(int argc, char* argv[]) {
    auto exec     = std::filesystem::path(argv[0]);
    auto exec_log = exec.stem().concat(".log");
    auto exec_cfg = exec.stem().concat(".cfg");

    GLogger::Initialize(exec_log.c_str());
    LOG_FORMAT(trace, "Process STARTED (%s)", exec.stem().c_str());

    Global::load_options(exec_cfg.c_str());

    auto ps2pl_regs{GMAPdevice(PS2PL_REGS_DEV_ADDR, PS2PL_REGS_DEV_SIZE)};
    auto pl2ps_regs{GMAPdevice(PL2PS_REGS_DEV_ADDR, PL2PS_REGS_DEV_SIZE)};

    uint32_t data[32];
    auto     _set = std::vector<uint8_t>({0, 2, 7, 8});
    auto     _not = std::vector<uint8_t>({2, 0, 8, 7});

    data[0] = set_bit<uint32_t>(3);
    data[1] = not_bit<uint32_t>(3);
    data[2] = set_mask<uint32_t>(_set);
    data[3] = not_mask<uint32_t>(_not);

    if (ps2pl_regs.Open()) {
        if (ps2pl_regs.MapToMemory()) {
            ps2pl_regs.Write(1, data, 4);
        }
        ps2pl_regs.Close();
    }

    memset(data, 0, sizeof(data));

    if (pl2ps_regs.Open()) {
        if (pl2ps_regs.MapToMemory()) {
            pl2ps_regs.Read(0, data, 32);
        }
        pl2ps_regs.Close();
    }

    LOG_FORMAT(trace, "Process STOPPED (%s)", exec.stem().c_str());
    return 0;
}
