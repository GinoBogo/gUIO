////////////////////////////////////////////////////////////////////////////////
/// \file      GRegisters.hpp
/// \version   0.1
/// \date      September, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GREGISTERS_HPP
#define GREGISTERS_HPP

#include <bitset>  // std::bitset
#include <cstdint> // uint8_t, uint32_t
#include <vector>  // std::vector

//==============================================================================
// SECTION: bits manipulation
//==============================================================================

#define __REG(BASE_ADDR, REG_OFFSET) *((volatile uint32_t*)((uint8_t*)BASE_ADDR + REG_OFFSET))

constexpr uint32_t SET_BIT(uint8_t _bit_pos) {
    return (1 << _bit_pos);
}

constexpr uint32_t NOT_BIT(uint8_t _bit_pos) {
    return ~SET_BIT(_bit_pos);
}

constexpr uint32_t SET_MASK(uint32_t _mask_bits, uint8_t _mask_pos) {
    return (_mask_bits << _mask_pos);
}

constexpr uint32_t NOT_MASK(uint32_t _mask_bits, uint8_t _mask_pos) {
    return ~SET_MASK(_mask_bits, _mask_pos);
}

template <uint32_t N = 32> auto to_bits(uint32_t value) {
    return std::bitset<N>(value).to_string();
}

template <typename T> auto set_bit(uint8_t pos) {
    return static_cast<T>(1 << pos);
}

template <typename T> auto not_bit(uint8_t pos) {
    return ~set_bit<T>(pos);
}

template <typename T> auto set_mask(std::vector<uint8_t> pos_list) {
    T _res{0};
    for (auto pos : pos_list) _res |= set_bit<T>(pos);
    return _res;
}

template <typename T> auto not_mask(std::vector<uint8_t> pos_list) {
    return ~set_mask<T>(pos_list);
}

const auto register_info_label_maxlen = 64;

struct register_info {
    uint32_t offset;
    uint32_t value;
    char     label[register_info_label_maxlen];
};

//==============================================================================
// LogiCORE IP: AXI GPIO v2.0
//==============================================================================

const auto gpio_registers_number = 7;

// AXI GPIO Address Space Offset
#define REG_GPIO_DATA_1 0x0000 // Channel 1 AXI GPIO Data Register (R/W)
#define REG_GPIO_TRI_1  0x0004 // Channel 1 AXI GPIO 3-state Control Register (R/W)
#define REG_GPIO_DATA_2 0x0008 // Channel 2 AXI GPIO Data Register (R/W)
#define REG_GPIO_TRI_2  0x000C // Channel 2 AXI GPIO 3-state Control Register (R/W)
#define REG_GPIO_GIER   0x011C // Global Interrupt Enable Register (R/W)
#define REG_GPIO_IP_IER 0x0128 // IP Interrupt Enable Register (R/W)
#define REG_GPIO_IP_ISR 0x0120 // IP Interrupt Status Register (R/TOW)

// IP Interrupt Registers Bits
#define BIT_GPIO_GIER     SET_BIT(31) // Global Interrupt Enable Bit
#define BIT_GPIO_IP_IER_1 SET_BIT(0)  // Channel 1 IP Interrupt Enable Bit
#define BIT_GPIO_IP_IER_2 SET_BIT(1)  // Channel 2 IP Interrupt Enable Bit
#define BIT_GPIO_IP_ISR_1 SET_BIT(0)  // Channel 1 IP Interrupt Status Bit
#define BIT_GPIO_IP_ISR_2 SET_BIT(1)  // Channel 2 IP Interrupt Status Bit

#define GPIO_getDataCh1(base_addr)                      __REG(base_addr, REG_GPIO_DATA_1)
#define GPIO_setDataCh1(base_addr, value)               __REG(base_addr, REG_GPIO_DATA_1) = value
#define GPIO_getTriStateCh1(base_addr)                  __REG(base_addr, REG_GPIO_TRI_1)
#define GPIO_setTriStateCh1(base_addr, value)           __REG(base_addr, REG_GPIO_TRI_1) = value
#define GPIO_getDataCh2(base_addr)                      __REG(base_addr, REG_GPIO_DATA_2)
#define GPIO_setDataCh2(base_addr, value)               __REG(base_addr, REG_GPIO_DATA_2) = value
#define GPIO_getTriStateCh2(base_addr)                  __REG(base_addr, REG_GPIO_TRI_2)
#define GPIO_setTriStateCh2(base_addr, value)           __REG(base_addr, REG_GPIO_TRI_2) = value
#define GPIO_getGlobalInterruptEnable(base_addr)        __REG(base_addr, REG_GPIO_GIER)
#define GPIO_setGlobalInterruptEnable(base_addr, value) __REG(base_addr, REG_GPIO_GIER) = value
#define GPIO_getIpInterruptEnable(base_addr)            __REG(base_addr, REG_GPIO_IP_IER)
#define GPIO_setIpInterruptEnable(base_addr, value)     __REG(base_addr, REG_GPIO_IP_IER) = value
#define GPIO_getIpInterruptStatus(base_addr)            __REG(base_addr, REG_GPIO_IP_ISR)
#define GPIO_setIpInterruptStatus(base_addr, value)     __REG(base_addr, REG_GPIO_IP_ISR) = value

void GPIO_getRegistersInfo(void* base_addr, register_info* info_list);

//==============================================================================
// LogiCORE IP: AXI Quad SPI v3.2
//==============================================================================
/*
 * [http://kernel.org/doc/Documentation/spi/spidev]
 *
 * SPI devices have a limited userspace API, supporting basic half-duplex read()
 * and write() access to SPI slave devices. Using ioctl() requests, full duplex
 * transfers and device I/O configuration are also available.
 *
 * For a SPI device with chipselect C on bus B, you should see:
 * 		/dev/spidevB.C
 * 		/sys/devices/platform/.../spi_master/spiB/spiB.C
 * 		/sys/class/spidev/spidevB.C
 */

const auto spi_registers_num = 11;

// SPI Address Space Offset (Legacy and Enhanced Mode)
#define REG_SPI_SRR   0x0040 // Software Reset Register (W)
#define REG_SPI_CR    0x0060 // SPI Control Register (R/W)
#define REG_SPI_SR    0x0064 // SPI Status Register (R)
#define REG_SPI_DTR   0x0068 // SPI Data Transmit Register (W)
#define REG_SPI_DRR   0x006C // SPI Data Receive Register (R)
#define REG_SPI_SSR   0x0070 // SPI Slave Select Register (R/W)
#define REG_SPI_TFOR  0x0074 // SPI Transmit FIFO Occupancy Register (R)
#define REG_SPI_RFOR  0x0078 // SPI receive FIFO Occupancy Register (R)
#define REG_SPI_DGIER 0x001C // Device Global Interrupt Enable Register (R/W)
#define REG_SPI_IPISR 0x0020 // IP Interrupt Status Register (R/TOW)
#define REG_SPI_IPIER 0x0028 // IP Interrupt Enable Register (R/W)

// SPI Control Register Bits
#define BIT_SPI_CR_LSBF  SET_BIT(9) // LSB First transfer format (R/W)
#define BIT_SPI_CR_MTI   SET_BIT(8) // Master Transaction Inhibit (R/W)
#define BIT_SPI_CR_MSSAE SET_BIT(7) // Manual Slave Select Assertion Enable (R/W)
#define BIT_SPI_CR_RFR   SET_BIT(6) // Receive FIFO Reset (R/W)
#define BIT_SPI_CR_TFR   SET_BIT(5) // Transmit FIFO Reset (R/W)
#define BIT_SPI_CR_CPHA  SET_BIT(4) // Clock Phase (R/W)
#define BIT_SPI_CR_CPOL  SET_BIT(3) // Clock Polarity (R/W)
#define BIT_SPI_CR_SMM   SET_BIT(2) // SPI Master Mode (R/W)
#define BIT_SPI_CR_SPE   SET_BIT(1) // SPI System Enable (R/W)
#define BIT_SPI_CR_LLM   SET_BIT(0) // Local Loopback Mode (R/W)

// SPI Status Register Bits
#define BIT_SPI_SR_CE   SET_BIT(10) // Command Error (R)
#define BIT_SPI_SR_LE   SET_BIT(9)  // Loopback Error (R)
#define BIT_SPI_SR_ME   SET_BIT(8)  // MSB Error (R)
#define BIT_SPI_SR_SMSE SET_BIT(7)  // Slave Mode Select Error (R)
#define BIT_SPI_SR_CPE  SET_BIT(6)  // CPHA/CPOL Error (R)
#define BIT_SPI_SR_SMS  SET_BIT(5)  // Slave Mode Select (R)
#define BIT_SPI_SR_MFE  SET_BIT(4)  // Mode-Fault Error (R)
#define BIT_SPI_SR_TXF  SET_BIT(3)  // Transmit Full (R)
#define BIT_SPI_SR_TXE  SET_BIT(2)  // Transmit Empty (R)
#define BIT_SPI_SR_RXF  SET_BIT(1)  // Receive Full (R)
#define BIT_SPI_SR_RXE  SET_BIT(0)  // Receive Empty (R)

// IP Interrupt Status Register Bits
#define BIT_SPI_ISR_CE    SET_BIT(13) // Command Error (R/TOW)
#define BIT_SPI_ISR_LE    SET_BIT(12) // Loopback Error (R/TOW)
#define BIT_SPI_ISR_ME    SET_BIT(11) // MSB Error (R/TOW)
#define BIT_SPI_ISR_SME   SET_BIT(10) // Slave Mode Error (R/TOW)
#define BIT_SPI_ISR_CPE   SET_BIT(9)  // CPHA/CPOL Error (R/TOW)
#define BIT_SPI_ISR_DRRNE SET_BIT(8)  // DDR Not Empty (R/TOW)
#define BIT_SPI_ISR_SMSE  SET_BIT(7)  // Slave Mode Select Error (R/TOW)
#define BIT_SPI_ISR_TFHE  SET_BIT(6)  // Transmit FIFO Half Empty (R/TOW)
#define BIT_SPI_ISR_DRRO  SET_BIT(5)  // DRR Overrun (R/TOW)
#define BIT_SPI_ISR_DRRF  SET_BIT(4)  // DRR Full (R/TOW)
#define BIT_SPI_ISR_DTRU  SET_BIT(3)  // DTR Underrun (R/TOW)
#define BIT_SPI_ISR_DTRE  SET_BIT(2)  // DTR Empty (R/TOW)
#define BIT_SPI_ISR_SMFE  SET_BIT(1)  // Slave Mode-Fault Error (R/TOW)
#define BIT_SPI_ISR_MFE   SET_BIT(0)  // Mode-Fault Error (R/TOW)

// IP Interrupt Enable Register Bits
#define BIT_SPI_IER_CE    SET_BIT(13) // Command Error (R/W)
#define BIT_SPI_IER_LE    SET_BIT(12) // Loopback Error (R/W)
#define BIT_SPI_IER_ME    SET_BIT(11) // MSB Error (R/W)
#define BIT_SPI_IER_SME   SET_BIT(10) // Slave Mode Error (R/W)
#define BIT_SPI_IER_CPE   SET_BIT(9)  // CPHA/CPOL Error (R/W)
#define BIT_SPI_IER_DRRNE SET_BIT(8)  // DDR Not Empty (R/W)
#define BIT_SPI_IER_SMS   SET_BIT(7)  // Slave Mode Select (R/W)
#define BIT_SPI_IER_TFHE  SET_BIT(6)  // Transmit FIFO Half Empty (R/W)
#define BIT_SPI_IER_DRRO  SET_BIT(5)  // DRR Overrun (R/W)
#define BIT_SPI_IER_DRRF  SET_BIT(4)  // DRR Full (R/W)
#define BIT_SPI_IER_DTRU  SET_BIT(3)  // DTR Underrun (R/W)
#define BIT_SPI_IER_DTRE  SET_BIT(2)  // DTR Empty (R/W)
#define BIT_SPI_IER_SMFE  SET_BIT(1)  // Slave Mode-Fault Error (R/W)
#define BIT_SPI_IER_MFE   SET_BIT(0)  // Mode-Fault Error (R/W)

#define QSPI_softwareResetRegister(base_addr)                   __REG(base_addr, REG_SPI_SRR) = 0x0000000A
#define QSPI_getControlRegister(base_addr)                      __REG(base_addr, REG_SPI_CR)
#define QSPI_setControlRegister(base_addr, value)               __REG(base_addr, REG_SPI_CR) = value
#define QSPI_getStatusRegister(base_addr)                       __REG(base_addr, REG_SPI_SR)
#define QSPI_setDataTransmitRegister(base_addr, value)          __REG(base_addr, REG_SPI_DTR) = value
#define QSPI_getDataReceiveRegister(base_addr)                  __REG(base_addr, REG_SPI_DRR)
#define QSPI_getSlaveSelectRegister(base_addr)                  __REG(base_addr, REG_SPI_SSR)
#define QSPI_setSlaveSelectRegister(base_addr, value)           __REG(base_addr, REG_SPI_SSR) = value
#define QSPI_getTransmitFifoOccupancyRegister(base_addr)        __REG(base_addr, REG_SPI_TFOR)
#define QSPI_getReceiveFifoOccupancyRegister(base_addr)         __REG(base_addr, REG_SPI_RFOR)
#define QSPI_getDeviceGlobalInterruptRegister(base_addr)        __REG(base_addr, REG_SPI_DGIER)
#define QSPI_setDeviceGlobalInterruptRegister(base_addr, value) __REG(base_addr, REG_SPI_DGIER) = value
#define QSPI_getIpInterruptStatusRegister(base_addr)            __REG(base_addr, REG_SPI_IPISR)
#define QSPI_setIpInterruptStatusRegister(base_addr, value)     __REG(base_addr, REG_SPI_IPISR) = value
#define QSPI_getIpInterruptEnableRegister(base_addr)            __REG(base_addr, REG_SPI_IPIER)
#define QSPI_setIpInterruptEnableRegister(base_addr, value)     __REG(base_addr, REG_SPI_IPIER) = value

#endif // GREGISTERS_HPP
