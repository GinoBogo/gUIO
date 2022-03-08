////////////////////////////////////////////////////////////////////////////////
/// \file      f_ad9361.hpp
/// \version   0.1
/// \date      February, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef F_AD9361_HPP
#define F_AD9361_HPP

#include "definitions.hpp"
#include "sdr_ad9361_api.hpp"
#include "sdr_if.hpp"
#include "spi_if.hpp"

namespace f_ad9361 {

    namespace __local {
        bool is_ready;
    }

    bool initialize() {
        __local::is_ready = false;

        if (SPI_SDR_Init(SPI_SDR1_CS, true, false)) {

            SDR_Reset(SPI_SDR1_CS);

            SDR_SoftReset(SPI_SDR1_CS);

            __local::is_ready = SDR_Configure(SPI_SDR1_CS);
        }

        return __local::is_ready;
    }

    void sdr_dump_regs() {
        if (__local::is_ready) {
            SDR_DumpRegs(SPI_SDR1_CS);
        }
        else {
            LOG_FORMAT(error, "[%s] Unable to dump the SDR registers", __func__);
        }
    }

    void sdr_self_test(bool pre_reset) {
        if (__local::is_ready) {
            SDR_SelfTest(SPI_SDR1_CS, pre_reset);
        }
        else {
            LOG_FORMAT(error, "[%s] Unable to perform the SDR self-test", __func__);
        }
    }

} // namespace f_ad9361

#endif // F_AD9361_HPP
