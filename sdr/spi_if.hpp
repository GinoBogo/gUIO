
////////////////////////////////////////////////////////////////////////////////
/// \file      spi_if.hpp
/// \version   0.1
/// \date      February, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef SPI_IF_HPP
#define SPI_IF_HPP

#include <cstdint> // uint8_t, uint32_t

bool SPI_SDR_Init(uint8_t id, bool clock_phase, bool clock_polarity);

uint8_t SPI_SDR_Read(uint8_t id, uint32_t reg, bool* error = nullptr);

uint8_t SPI_SDR_ReadF(uint8_t id, uint32_t reg, uint8_t mask, bool* error = nullptr);

bool SPI_SDR_ReadM(uint8_t id, uint32_t reg, uint8_t* rx_buf, uint32_t rx_buf_len);

bool SPI_SDR_Write(uint8_t id, uint32_t reg, uint8_t val);

bool SPI_SDR_WriteF(uint8_t id, uint32_t reg, uint8_t mask, uint8_t val);

bool SPI_SDR_WriteM(uint8_t id, uint32_t reg, uint8_t* tx_buf, uint32_t tx_buf_len);

bool SPI_FPGA_Write(uint32_t reg, uint32_t val);

uint32_t SPI_FPGA_Read(uint32_t reg, bool* error = nullptr);

#endif // SPI_IF_HPP
