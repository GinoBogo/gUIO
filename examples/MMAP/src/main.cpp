
////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GLogger.hpp" // Initialize, LOG_FORMAT, trace
#include "globals.hpp"

#include <filesystem> // path

int main(int argc, char* argv[]) {
    auto exec     = std::filesystem::path(argv[0]);
    auto exec_log = exec.stem().concat(".log");
    auto exec_cfg = exec.stem().concat(".cfg");

    GLogger::Initialize(exec_log.c_str());
    LOG_FORMAT(trace, "Process STARTED (%s)", exec.stem().c_str());

    Global::load_options(exec_cfg);

    auto ps2pl_list = Global::load_registers(PS2PL_REGS_CFG_FILE);
    auto pl2ps_list = Global::load_registers(PL2PS_REGS_CFG_FILE);

    auto ps2pl_regs{GMAPdevice(PS2PL_REGS_DEV_ADDR, PS2PL_REGS_DEV_SIZE)};
    auto pl2ps_regs{GMAPdevice(PL2PS_REGS_DEV_ADDR, PL2PS_REGS_DEV_SIZE)};

    if (ps2pl_regs.Open()) {
        if (ps2pl_regs.MapToMemory()) {
            ps2pl_regs.Write(ps2pl_list);
        }
        ps2pl_regs.Close();
    }

    if (pl2ps_regs.Open()) {
        if (pl2ps_regs.MapToMemory()) {
            pl2ps_regs.Read(pl2ps_list);
        }
        pl2ps_regs.Close();
    }

    LOG_FORMAT(trace, "Process STOPPED (%s)", exec.stem().c_str());
    return 0;
}
