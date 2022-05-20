
////////////////////////////////////////////////////////////////////////////////
/// \file      spi_if.cpp
/// \version   0.1
/// \date      February, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

// clang-format off
#ifndef UNUSED
#define UNUSED(x) if (x) {}
#endif
// clang-format on

#include "spi_if.hpp"

#include "GAXIQuadSPI.hpp"
#include "GLogger.hpp"
#include "definitions.hpp"
#include "sdr_ad9361.hpp"

#include <cstring> // memcpy

GMAPdevice*  ad9361_regs = nullptr;
GAXIQuadSPI* ad9361_qspi = nullptr;

uint32_t __ffs(uint32_t word) {
    uint32_t num = 0;

    if ((word & 0xffff) == 0) {
        num += 16;
        word >>= 16;
    }
    if ((word & 0xff) == 0) {
        num += 8;
        word >>= 8;
    }
    if ((word & 0xf) == 0) {
        num += 4;
        word >>= 4;
    }
    if ((word & 0x3) == 0) {
        num += 2;
        word >>= 2;
    }
    if ((word & 0x1) == 0) { num += 1; }
    return num;
}

bool SPI_SDR_Init(uint8_t id, bool clock_phase, bool clock_polarity) {
    UNUSED(id);

    if (ad9361_regs != nullptr) {
        delete ad9361_regs;
        ad9361_regs = nullptr;
    }

    if (ad9361_qspi != nullptr) {
        delete ad9361_qspi;
        ad9361_qspi = nullptr;
    }

    ad9361_regs = new GMAPdevice(AD9361_REGS_ADDR, AD9361_REGS_SIZE);
    ad9361_qspi = new GAXIQuadSPI(AD9361_QSPI_ADDR, AD9361_QSPI_SIZE);

    if (ad9361_qspi->is_valid()) {
        ad9361_qspi->Initialize(clock_phase, clock_polarity);
        ad9361_qspi->Start();

        LOG_FORMAT(info, "SPI device created (%s)", __func__);
        return true;
    }

    LOG_FORMAT(error, "SPI device failure (%s)", __func__);
    return false;
}

uint8_t SPI_SDR_Read(uint8_t id, uint32_t reg, bool* error) {
    auto    _ret{true};
    uint8_t _buf{0};

    if (!SPI_SDR_ReadM(id, reg, &_buf, 1)) {
        LOG_FORMAT(error, "Read Error [reg: 0x%04X] (%s)", reg, __func__);
        _ret = false;
    }

    if (error != nullptr) { *error = _ret; }
    return _buf;
}

uint8_t SPI_SDR_ReadF(uint8_t id, uint32_t reg, uint8_t mask, bool* error) {
    auto    _ret{true};
    uint8_t _buf{0};

    if (mask == 0) {
        LOG_FORMAT(error, "Wrong Mask [mask: 0x%04X] (%s)", mask, __func__);
        _ret = false;
        goto _exit;
    }

    if (!SPI_SDR_ReadM(id, reg, &_buf, 1)) {
        LOG_FORMAT(error, "Read Error [reg: 0x%04X] (%s)", reg, __func__);
        _ret = false;
        goto _exit;
    }

    _buf &= mask;
    _buf >>= __ffs(mask);

_exit:
    if (error != nullptr) { *error = _ret; }
    return _buf;
}

bool SPI_SDR_ReadM(uint8_t id, uint32_t reg, uint8_t* rx_buf, uint32_t rx_buf_len) {
    UNUSED(id);

    if (rx_buf != nullptr && ad9361_qspi != nullptr) {
        uint16_t cmd = AD_READ | AD_CNT(rx_buf_len) | AD_ADDR(reg);

        uint8_t _buf[2];
        _buf[0] = cmd >> 8;
        _buf[1] = cmd & 0xFF;

        ad9361_qspi->WriteThenRead(_buf, 2, rx_buf, rx_buf_len);
        return true;
    }

    LOG_FORMAT(error, "Read Error [reg: 0x%04X, num: %d] (%s)", reg, rx_buf_len, __func__);
    return false;
}

bool SPI_SDR_Write(uint8_t id, uint32_t reg, uint8_t val) {
    UNUSED(id);

    if (!SPI_SDR_WriteM(id, reg, &val, 1)) {
        LOG_FORMAT(error, "Write Error [reg: 0x%04X, num: %d] (%s)", reg, 1, __func__);
        return false;
    }

    return true;
}

bool SPI_SDR_WriteF(uint8_t id, uint32_t reg, uint8_t mask, uint8_t val) {
    UNUSED(id);

    if (mask == 0) {
        LOG_FORMAT(error, "Wrong Mask [mask: 0x%04X] (%s)", mask, __func__);
        return false;
    }

    uint8_t _buf;

    if (!SPI_SDR_ReadM(id, reg, &_buf, 1)) {
        LOG_FORMAT(error, "Read Error [reg: 0x%04X] (%s)", reg, __func__);
        return false;
    }

    _buf &= ~mask;
    _buf |= ((val << __ffs(mask)) & mask);

    return SPI_SDR_WriteM(id, reg, &_buf, 1);
}

bool SPI_SDR_WriteM(uint8_t id, uint32_t reg, uint8_t* tx_buf, uint32_t tx_buf_len) {
    UNUSED(id);

    if (tx_buf != nullptr && ad9361_qspi != nullptr) {
        if (tx_buf_len > MAX_MBYTE_SPI) {
            LOG_FORMAT(error, "Writing Capacity overcoming [num > max: %d > %d] (%s)", tx_buf_len, MAX_MBYTE_SPI, __func__);
            return false;
        }

        uint16_t cmd = AD_WRITE | AD_CNT(tx_buf_len) | AD_ADDR(reg);

        uint8_t _buf[2 + MAX_MBYTE_SPI];
        _buf[0] = cmd >> 8;
        _buf[1] = cmd & 0xFF;

        memcpy(&_buf[2], tx_buf, tx_buf_len);

        ad9361_qspi->WriteThenRead(_buf, 2 + tx_buf_len, nullptr, 0);
        return true;
    }

    LOG_FORMAT(error, "Write Error [reg: 0x%04X, num: %d] (%s)", reg, tx_buf_len, __func__);
    return false;
}

bool SPI_FPGA_Write(uint32_t reg, uint32_t val) {
    auto _ret{false};

    if (ad9361_regs->Open()) {
        if (ad9361_regs->MapToMemory()) { _ret = ad9361_regs->Write(reg, &val); }
        ad9361_regs->Close();
    }
    return _ret;
}

uint32_t SPI_FPGA_Read(uint32_t reg, bool* error) {
    auto _ret{false};

    uint32_t _buf{0};

    if (ad9361_regs->Open()) {
        if (ad9361_regs->MapToMemory()) { _ret = ad9361_regs->Read(reg, &_buf, 1); }
        ad9361_regs->Close();
    }

    if (error != nullptr) { *error = _ret; }
    return _buf;
}
