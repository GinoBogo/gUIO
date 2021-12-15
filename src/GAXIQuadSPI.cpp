////////////////////////////////////////////////////////////////////////////////
/// \file      GAXIQuadSPI.cpp
/// \version   0.1
/// \date      December, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GAXIQuadSPI.hpp"

#include "GLogger.hpp"
#include "registers.hpp"

#define enable_system       BIT_SPI_CR_SPE
#define inhibit_master      BIT_SPI_CR_MTI
#define reset_all_fifo      BIT_SPI_CR_RFR | BIT_SPI_CR_TFR
#define transmit_full       BIT_SPI_SR_TXF
#define transmit_empty      BIT_SPI_SR_TXE
#define receive_full        BIT_SPI_SR_RXF
#define receive_empty       BIT_SPI_SR_RXE
#define enable_chip_select  0x00000000 // chip-select is active low
#define disable_chip_select 0x00000001
#define enable_global_irq   0x80000000
#define disable_global_irq  0x00000000

GAXIQuadSPI::GAXIQuadSPI(size_t addr, size_t size) : GMEMdevice(addr, size) {
    m_base_addr  = nullptr;
    m_is_valid   = false;
    m_ctrl_reg   = 0;
    m_status_reg = 0;

    if (Open()) {
        if (MapToMemory()) {
            m_base_addr = virt_addr();
            m_is_valid  = true;
            LOG_WRITE(trace, "AXI Quad SPI class created");
            return;
        }
    }
    LOG_WRITE(error, "AXI Quad SPI class failure");
}

GAXIQuadSPI::~GAXIQuadSPI() {
    LOG_WRITE(trace, "AXI Quad SPI class destroyed");
}

void GAXIQuadSPI::update_ctrl_reg(const char *func) {
    m_ctrl_reg = QSPI_getControlRegister(m_base_addr);

    LOG_FORMAT(debug, "SPI Control Register: %s (%s)", to_bits<10>(m_ctrl_reg).c_str(), func);
}

void GAXIQuadSPI::Reset() {
    QSPI_setSlaveSelectRegister(m_base_addr, disable_chip_select); // abort any transfer

    auto ctrl_reg = QSPI_getControlRegister(m_base_addr);

    QSPI_setControlRegister(m_base_addr, ctrl_reg | inhibit_master | reset_all_fifo);

    QSPI_softwareResetRegister(m_base_addr);

    update_ctrl_reg(__func__);
}

void GAXIQuadSPI::Initialize(bool clock_phase, bool clock_polarity, bool loopback_mode) {
    Reset();

    auto ctrl_reg = 0                      // INFO: set device parameters
                    | 0 * BIT_SPI_CR_LSBF  //  9 | LSB First transfer format (R/W)
                    | 1 * BIT_SPI_CR_MTI   //  8 | Master Transaction Inhibit (R/W)
                    | 1 * BIT_SPI_CR_MSSAE //  7 | Manual Slave Select Assertion Enable (R/W)
                    | 0 * BIT_SPI_CR_RFR   //  6 | Receive FIFO Reset (R/W)
                    | 0 * BIT_SPI_CR_TFR   //  5 | Transmit FIFO Reset (R/W)
                    | 0 * BIT_SPI_CR_CPHA  //  4 | Clock Phase (R/W)
                    | 0 * BIT_SPI_CR_CPOL  //  3 | Clock Polarity (R/W)
                    | 1 * BIT_SPI_CR_SMM   //  2 | SPI Master Mode (R/W)
                    | 0 * BIT_SPI_CR_SPE   //  1 | SPI System Enable (R/W)
                    | 0 * BIT_SPI_CR_LLM;  //  1 | Local Loopback Mode (R/W)

    if (clock_phase) ctrl_reg |= BIT_SPI_CR_CPHA;

    if (clock_polarity) ctrl_reg |= BIT_SPI_CR_CPOL;

    if (loopback_mode) ctrl_reg |= BIT_SPI_CR_LLM;

    QSPI_setControlRegister(m_base_addr, ctrl_reg);

    update_ctrl_reg(__func__);
}

void GAXIQuadSPI::Start() {
    const auto enable_irq = 0                       // INFO: set interrupts mask
                            | 1 * BIT_SPI_IER_CE    // 13 | Command Error (R/W)
                            | 1 * BIT_SPI_IER_LE    // 12 | Loopback Error (R/W)
                            | 1 * BIT_SPI_IER_ME    // 11 | MSB Error (R/W)
                            | 1 * BIT_SPI_IER_SME   // 10 | Slave Mode Error (R/W)
                            | 1 * BIT_SPI_IER_CPE   //  9 | CPHA/CPOL Error (R/W)
                            | 1 * BIT_SPI_IER_DRRNE //  8 | DDR Not Empty (R/W)
                            | 1 * BIT_SPI_IER_SMS   //  7 | Slave Mode Select (R/W)
                            | 1 * BIT_SPI_IER_TFHE  //  6 | Transmit FIFO Half Empty (R/W)
                            | 1 * BIT_SPI_IER_DRRO  //  5 | DRR Overrun (R/W)
                            | 1 * BIT_SPI_IER_DRRF  //  4 | DRR Full (R/W)
                            | 1 * BIT_SPI_IER_DTRU  //  3 | DTR Underrun (R/W)
                            | 1 * BIT_SPI_IER_DTRE  //  2 | DTR Empty (R/W)
                            | 1 * BIT_SPI_IER_SMFE  //  1 | Slave Mode-Fault Error (R/W)
                            | 1 * BIT_SPI_IER_MFE;  //  0 | Mode-Fault Error (R/W)

    QSPI_setIpInterruptEnableRegister(m_base_addr, enable_irq);

    auto ctrl_reg = QSPI_getControlRegister(m_base_addr);

    QSPI_setControlRegister(m_base_addr, ctrl_reg | reset_all_fifo | enable_system);

    QSPI_setDeviceGlobalInterruptRegister(m_base_addr, enable_global_irq);

    update_ctrl_reg(__func__);
}

void GAXIQuadSPI::Stop() {
    QSPI_setDeviceGlobalInterruptRegister(m_base_addr, disable_global_irq);

    auto ctrl_reg = QSPI_getControlRegister(m_base_addr);

    QSPI_setControlRegister(m_base_addr, ctrl_reg & ~enable_system);

    update_ctrl_reg(__func__);
}

uint32_t GAXIQuadSPI::WriteThenRead(uint8_t *tx_buf, uint32_t tx_buf_len, uint8_t *rx_buf, uint32_t rx_buf_len) {
    QSPI_setDeviceGlobalInterruptRegister(m_base_addr, disable_global_irq);
    {
        m_ctrl_reg = QSPI_getControlRegister(m_base_addr);

        QSPI_setControlRegister(m_base_addr, m_ctrl_reg | inhibit_master);

        for (decltype(tx_buf_len) i{0}; i < tx_buf_len; ++i) {
            volatile auto status_reg = QSPI_getStatusRegister(m_base_addr);

            if (status_reg & transmit_full) break;

            QSPI_setDataTransmitRegister(m_base_addr, tx_buf[i]);
        }

        for (decltype(rx_buf_len) i{0}; i < rx_buf_len; ++i) {
            volatile auto status_reg = QSPI_getStatusRegister(m_base_addr);

            if (status_reg & transmit_full) break;

            QSPI_setDataTransmitRegister(m_base_addr, 0);
        }

        QSPI_setSlaveSelectRegister(m_base_addr, enable_chip_select);

        QSPI_setControlRegister(m_base_addr, m_ctrl_reg & ~inhibit_master);

        while (true) {
            volatile auto status_reg = QSPI_getStatusRegister(m_base_addr);

            if (status_reg & transmit_empty) break;
        }

        for (decltype(tx_buf_len) i{0}; i < tx_buf_len; ++i) {
            volatile auto status_reg = QSPI_getStatusRegister(m_base_addr);

            if (status_reg & receive_empty) break;

            auto data = QSPI_getDataReceiveRegister(m_base_addr);

            if (rx_buf != nullptr) rx_buf[i] = data;
        }

        for (decltype(rx_buf_len) i{0}; i < rx_buf_len; ++i) {
            volatile auto status_reg = QSPI_getStatusRegister(m_base_addr);

            if (status_reg & receive_empty) break;

            auto data = QSPI_getDataReceiveRegister(m_base_addr);

            if (rx_buf != nullptr) rx_buf[i] = data;
        }

        QSPI_setControlRegister(m_base_addr, m_ctrl_reg | inhibit_master);

        QSPI_setSlaveSelectRegister(m_base_addr, disable_chip_select);
    }
    QSPI_setDeviceGlobalInterruptRegister(m_base_addr, enable_global_irq);

    m_status_reg = QSPI_getStatusRegister(m_base_addr);

    return m_status_reg;
}
