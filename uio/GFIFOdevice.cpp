////////////////////////////////////////////////////////////////////////////////
/// \file      GFIFOdevice.cpp
/// \version   0.1
/// \date      March 15, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GFIFOdevice.hpp"

#include "GLogger.hpp"

GFIFOdevice::GFIFOdevice(size_t dev_addr, size_t dev_size, int uio_num, int uio_map) {
    m_dev_addr = dev_addr;
    m_dev_size = dev_size;
    m_uio_num  = uio_num;
    m_uio_map  = uio_map;

    m_dev      = new GMAPdevice(m_dev_addr, m_dev_size);
    m_uio      = new GUIOdevice(m_uio_num, m_uio_map);
    m_uio_regs = nullptr;
    m_is_ready = false;

    LOG_WRITE(trace, "FIFO device created");
}

GFIFOdevice::~GFIFOdevice() {
    if (m_is_ready) {
        Close();
    }
    delete m_dev;
    delete m_uio;

    LOG_WRITE(trace, "FIFO device destroyed");
}

bool GFIFOdevice::Open() {
    m_is_ready = false;

    if (m_dev->Open()) {
        if (m_dev->MapToMemory()) {
            if (m_uio->Open()) {
                if (m_uio->MapToMemory()) {
                    m_uio_regs = m_uio->virt_addr();
                    GPIO_setIpInterruptEnable(m_uio_regs, __ON(BIT_GPIO_IP_IER_1));
                    GPIO_setGlobalInterruptEnable(m_uio_regs, __ON(BIT_GPIO_GIER));

                    m_is_ready = true;
                    LOG_WRITE(trace, "FIFO device open success");
                    goto jmp_exit;
                }
            }
        }
    }

    LOG_WRITE(error, "FIFO device open failure");
jmp_exit:
    return m_is_ready;
}

void GFIFOdevice::Close() {
    if (m_is_ready) {
        m_is_ready = false;
        GPIO_setIpInterruptEnable(m_uio_regs, __OFF(BIT_GPIO_IP_IER_1));
        GPIO_setGlobalInterruptEnable(m_uio_regs, __OFF(BIT_GPIO_GIER));
    }

    m_dev->Close();
    m_uio->Close();

    LOG_WRITE(trace, "FIFO device close success");
}

bool GFIFOdevice::Reset() {
    auto _res{false};

    if (m_is_ready) {
        uint32_t _enable{1};
        uint32_t _forbid{0};

        _res = _res || m_dev->Write(IP_CONTROL, &_enable);
        _res = _res && m_dev->Write(IP_CONTROL, &_forbid);
    }

    return _res;
}

uint32_t GFIFOdevice::PEEK(uint32_t offset, bool* error) {
    uint32_t _val{0};
    auto     _res{false};

    if (m_is_ready) {
        _res = m_dev->Read(offset, &_val);
    }

    if (error != nullptr) {
        *error = !_res;
    }

    return _val;
}

bool GFIFOdevice::POKE(uint32_t offset, uint32_t value) {
    auto _res{false};

    if (m_is_ready) {
        _res = m_dev->Write(offset, &value);
    }

    return _res;
}

bool GFIFOdevice::SetTxPacketWords(uint32_t words) {
    auto _res{false};

    if (m_is_ready) {
        uint32_t _enable{1};
        uint32_t _forbid{0};

        _res = _res || m_dev->Write(IP_CONTROL, &_enable);
        _res = _res && m_dev->Write(TX_PACKET_WORDS, &words);
        _res = _res && m_dev->Write(IP_CONTROL, &_forbid);
    }

    return _res;
}

uint32_t GFIFOdevice::GetTxPacketWords(bool* error) {
    uint32_t _val{0};
    auto     _res{false};

    if (m_is_ready) {
        _res = m_dev->Read(TX_PACKET_WORDS, &_val);
    }

    if (error != nullptr) {
        *error = !_res;
    }

    return _val;
}

uint32_t GFIFOdevice::GetTxUnusedWords(bool* error) {
    uint32_t _val{0};
    auto     _res{false};

    if (m_is_ready) {
        _res = m_dev->Read(TX_UNUSED_WORDS, &_val);
        _val = 0x00001FFF & _val;
    }

    if (error != nullptr) {
        *error = !_res;
    }

    return _val;
}

uint32_t GFIFOdevice::GetRxLengthLevel(bool* error) {
    uint32_t _val{0};
    auto     _res{false};

    if (m_is_ready) {
        _res = m_dev->Read(RX_LENGTH_LEVEL, &_val);
        _val = 0x0000FFFF & _val;
    }

    if (error != nullptr) {
        *error = !_res;
    }

    return _val;
}

uint32_t GFIFOdevice::GetRxPacketWords(bool* error) {
    uint32_t _val{0};
    auto     _res{false};

    if (m_is_ready) {
        _res = m_dev->Read(RX_PACKET_BYTES, &_val);
        _val = 0x0000FFFF & ((_val + 1) / sizeof(uint16_t));
    }

    if (error != nullptr) {
        *error = !_res;
    }

    return _val;
}
