
////////////////////////////////////////////////////////////////////////////////
/// \file      GAXIQuadSPI.hpp
/// \version   0.1
/// \date      December, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GAXIQUADSPI_HPP
#define GAXIQUADSPI_HPP

#include "GMAPdevice.hpp"

class GAXIQuadSPI : public GMAPdevice {
  public:
    GAXIQuadSPI(size_t addr, size_t size);
    ~GAXIQuadSPI();

    [[nodiscard]] auto is_valid() const {
        return m_is_valid;
    }

    void     Reset();
    void     Initialize(bool clock_phase, bool clock_polarity, bool loopback_mode = false);
    void     Start();
    void     Stop();
    uint32_t WriteThenRead(const uint8_t* tx_buf, uint32_t tx_buf_len, uint8_t* rx_buf, uint32_t rx_buf_len);

  private:
    void update_ctrl_reg(const char* func);

    void*             m_base_addr;
    bool              m_is_valid;
    volatile uint32_t m_ctrl_reg;
    volatile uint32_t m_status_reg;
};

#endif // GAXIQUADSPI_HPP
