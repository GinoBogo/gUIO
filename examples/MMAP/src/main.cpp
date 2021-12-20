////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GLogger.hpp"
#include "GMEMdevice.hpp"
#include "registers.hpp"

#include <strings.h> // bzero

int main() {
    GLogger::Initialize("example_MMAP.log");
    LOG_WRITE(trace, "Process STARTED");

    auto ps2pl_regs{GMEMdevice(0xa0001000, 4096)};
    auto pl2ps_regs{GMEMdevice(0xa0010000, 4096)};

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

    bzero(data, sizeof(data));

    if (pl2ps_regs.Open()) {
        if (pl2ps_regs.MapToMemory()) {
            pl2ps_regs.Read(0, data, 32);
        }
        pl2ps_regs.Close();
    }

    LOG_WRITE(trace, "Process STOPPED");
    return 0;
}
