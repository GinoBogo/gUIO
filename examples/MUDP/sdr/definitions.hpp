////////////////////////////////////////////////////////////////////////////////
/// \file      definitions.hpp
/// \version   0.1
/// \date      February, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

// clang-format off
#ifndef UNUSED
#define UNUSED(x) if (x) {}
#endif
// clang-format on

#define AD9361_REGS_ADDR    0xA0050000
#define AD9361_REGS_SIZE    4096
#define AD9361_QSPI_ADDR    0xA0020000
#define AD9361_QSPI_SIZE    4096
#define AD9361_RESET_ADDR   0
#define AD9361_RESET_ASSERT 0x00000000
#define AD9361_RESET_FORBID 0x00000001

#define SPI_SDR_NUM         1
#define SPI_SDR1_CS         0

#endif // DEFINITIONS_HPP
