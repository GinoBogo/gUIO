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

#include <cstring> // memset

int main() {
    GLogger::Initialize("_mmap.log");
    LOG_WRITE(trace, "Process STARTED");

    auto ps2pl_regs{GMAPdevice(0xa0001000, 4096)};
    auto pl2ps_regs{GMAPdevice(0xa0010000, 4096)};

    uint32_t data[32];

    data[0] = set_bit<uint32_t>(3);
    data[1] = not_bit<uint32_t>(3);
    data[2] = set_mask<uint32_t>({0, 2, 7, 8});
    data[3] = not_mask<uint32_t>({2, 0, 8, 7});

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

    LOG_WRITE(trace, "Process STOPPED");
    return 0;
}
