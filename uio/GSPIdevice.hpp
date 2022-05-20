
////////////////////////////////////////////////////////////////////////////////
/// \file      GSPIdevice.hpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GSPIDEVICE_HPP
#define GSPIDEVICE_HPP

#include <cstdint>            // uint8_t, uint32_t
#include <linux/spi/spidev.h> // SPI_MODE_*

const auto spi_device_t_file_maxlen = 64;

struct spi_device_t {
    // SECTION: node properties
    char file[spi_device_t_file_maxlen];
    int  fd;
    // SECTION: device settings
    uint32_t mode;
    uint8_t  lsb_first;
    uint8_t  bits_per_word;
    uint32_t max_speed_hz;
};

class GSPIdevice {
    public:
    GSPIdevice(const char* file,                       //
               uint32_t    mode          = SPI_MODE_1, //
               uint8_t     lsb_first     = 0,          //
               uint8_t     bits_per_word = 8,          //
               uint32_t    max_speed_hz  = 1000000     //
    );
    ~GSPIdevice();

    bool Open();
    void Close();
    void PrintSettings();

    bool Transfer(const void* tx_buf, void* rx_buf, uint32_t buf_len) const;
    bool Read(void* rx_buf, uint32_t rx_buf_len) const;
    bool Write(void* tx_buf, uint32_t tx_buf_len) const;
    bool WriteThenRead(const void* tx_buf, uint32_t tx_buf_len, void* rx_buf, uint32_t rx_buf_len) const;

    private:
    spi_device_t m_dev;
};

#endif // GSPIDEVICE_HPP
