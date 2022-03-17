////////////////////////////////////////////////////////////////////////////////
/// \file      GSPIdevice.cpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GSPIdevice.hpp"

#include "GLogger.hpp"

#include <errno.h>     // errno
#include <fcntl.h>     // open
#include <string.h>    // strncpy
#include <sys/ioctl.h> // ioctl
#include <unistd.h>    // close

auto spi_device_reset = [](spi_device_t* dev, bool clear_all) {
    if (clear_all) {
        bzero(dev, sizeof(spi_device_t));
    }
    dev->fd = -1;
};

auto is_mode_legal = [](uint8_t mode) {
    switch (mode) {
        case SPI_MODE_0:
        case SPI_MODE_1:
        case SPI_MODE_2:
        case SPI_MODE_3:
            return true;
        default:
            return false;
    }
};

GSPIdevice::GSPIdevice(const char* file,          //
                       uint32_t    mode,          //
                       uint8_t     lsb_first,     //
                       uint8_t     bits_per_word, //
                       uint32_t    max_speed_hz) {

    spi_device_reset(&m_dev, true);

    if (!is_mode_legal(mode)) {
        LOG_FORMAT(warning, "Wrong SPI mode (%d)", mode);
    }

    strncpy(m_dev.file, file, spi_device_t_file_maxlen - 1);
    m_dev.mode          = mode;
    m_dev.lsb_first     = lsb_first;
    m_dev.bits_per_word = bits_per_word;
    m_dev.max_speed_hz  = max_speed_hz;
}

GSPIdevice::~GSPIdevice() {
    Close();
}

bool GSPIdevice::Open() {
    m_dev.fd = open(m_dev.file, O_RDWR);

    if (m_dev.fd == -1) {
        LOG_FORMAT(error, "Unable to open the \"%s\" device [E%d]", m_dev.file, errno);
        goto fail_open;
    }

    if (ioctl(m_dev.fd, SPI_IOC_WR_MODE32, &(m_dev.mode)) == -1) {
        LOG_FORMAT(error, "SPI MODE32 writing failure [E%d]", errno);
        goto fail_ioctl;
    }

    if (ioctl(m_dev.fd, SPI_IOC_WR_LSB_FIRST, &(m_dev.lsb_first)) == -1) {
        LOG_FORMAT(error, "SPI LSB_FIRST writing failure [E%d]", errno);
        goto fail_ioctl;
    }

    if (ioctl(m_dev.fd, SPI_IOC_WR_BITS_PER_WORD, &(m_dev.bits_per_word)) == -1) {
        LOG_FORMAT(error, "SPI BITS_PER_WORD writing failure [E%d]", errno);
        goto fail_ioctl;
    }

    if (ioctl(m_dev.fd, SPI_IOC_WR_MAX_SPEED_HZ, &(m_dev.max_speed_hz)) == -1) {
        LOG_FORMAT(error, "SPI MAX_SPEED_HZ writing failure [E%d]", errno);
        goto fail_ioctl;
    }

    return true;

fail_ioctl:
    Close();

fail_open:
    return false;
}

void GSPIdevice::Close() {
    if (m_dev.fd != -1) {
        close(m_dev.fd);
    }

    spi_device_reset(&m_dev, false);
}

void GSPIdevice::PrintSettings() {
    __u8  lsb_first = 0, bits_per_word = 0;
    __u32 mode = 0, max_speed_hz = 0;

    if (ioctl(m_dev.fd, SPI_IOC_RD_MODE32, &mode) == -1) {
        LOG_FORMAT(error, "SPI MODE32 reading failure [E%d]", errno);
        return;
    }

    if (ioctl(m_dev.fd, SPI_IOC_RD_LSB_FIRST, &lsb_first) == -1) {
        LOG_FORMAT(error, "SPI LSB_FIRST reading failure [E%d]", errno);
        return;
    }

    if (ioctl(m_dev.fd, SPI_IOC_RD_BITS_PER_WORD, &bits_per_word) == -1) {
        LOG_FORMAT(error, "SPI BITS_PER_WORD reading failure [E%d]", errno);
        return;
    }

    if (ioctl(m_dev.fd, SPI_IOC_RD_MAX_SPEED_HZ, &max_speed_hz) == -1) {
        LOG_FORMAT(error, "SPI MAX_SPEED_HZ reading failure [E%d]", errno);
        return;
    }

    auto spi_cpha{(bool)(mode & SPI_CPHA)};
    auto spi_cpol{(bool)(mode & SPI_CPOL)};

    LOG_FORMAT(info, "SPI Settings <PARAM> <VALUE> of \"%s\" device:", m_dev.file);
    LOG_FORMAT(info, "  MODE          | %d", mode);
    LOG_FORMAT(info, "  CLK_PHASE     | %d", spi_cpha);
    LOG_FORMAT(info, "  CLK_POLARITY  | %d", spi_cpol);
    LOG_FORMAT(info, "  LSB_FIRST     | %d", lsb_first);
    LOG_FORMAT(info, "  BITS_PER_WORD | %d", bits_per_word);
    LOG_FORMAT(info, "  MAX_SPEED_HZ  | %d", max_speed_hz);
}

bool GSPIdevice::Transfer(const void* tx_buf, void* rx_buf, uint32_t buf_len) {
    struct spi_ioc_transfer _msg;
    bzero(&_msg, sizeof(_msg));

    _msg.tx_buf    = (__u64)tx_buf;
    _msg.rx_buf    = (__u64)rx_buf;
    _msg.len       = (__u32)buf_len;
    _msg.cs_change = 1;

    if (ioctl(m_dev.fd, SPI_IOC_MESSAGE(1), &_msg) == -1) {
        LOG_FORMAT(error, "SPI MESSAGE(1) transfer failure [E%d]", errno);
        return false;
    }

    return true;
}

bool GSPIdevice::Read(void* rx_buf, uint32_t rx_buf_len) {
    struct spi_ioc_transfer _msg;
    bzero(&_msg, sizeof(_msg));

    _msg.rx_buf = (__u64)rx_buf;
    _msg.len    = (__u32)rx_buf_len;

    if (ioctl(m_dev.fd, SPI_IOC_MESSAGE(1), &_msg) == -1) {
        LOG_FORMAT(error, "SPI MESSAGE(1) reading failure [E%d]", errno);
        return false;
    }

    return true;
}

bool GSPIdevice::Write(void* tx_buf, uint32_t tx_buf_len) {
    struct spi_ioc_transfer _msg;
    bzero(&_msg, sizeof(_msg));

    _msg.rx_buf = (__u64)tx_buf;
    _msg.len    = (__u32)tx_buf_len;

    if (ioctl(m_dev.fd, SPI_IOC_MESSAGE(1), &_msg) == -1) {
        LOG_FORMAT(error, "SPI MESSAGE(1) writing failure [E%d]", errno);
        return false;
    }

    return true;
}

bool GSPIdevice::WriteThenRead(const void* tx_buf, uint32_t tx_buf_len, void* rx_buf, uint32_t rx_buf_len) {
    struct spi_ioc_transfer _msg[2];
    bzero(_msg, sizeof(_msg));

    _msg[0].tx_buf = (__u64)tx_buf;
    _msg[0].len    = (__u32)tx_buf_len;
    _msg[1].rx_buf = (__u64)rx_buf;
    _msg[1].len    = (__u32)rx_buf_len;

    if (ioctl(m_dev.fd, SPI_IOC_MESSAGE(2), _msg) == -1) {
        LOG_FORMAT(error, "SPI MESSAGE(2) transfer failure [E%d]", errno);
        return false;
    }

    return true;
}
