
////////////////////////////////////////////////////////////////////////////////
/// \file      GFIFOdevice.cpp
/// \version   0.1
/// \date      March 15, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GFIFOdevice.hpp"

#include "GLogger.hpp"

GFIFOdevice::GFIFOdevice(size_t dev_addr, size_t dev_size, int uio_num, int uio_map, const std::string& tag_name) {
    m_dev_addr = dev_addr;
    m_dev_size = dev_size;
    m_uio_num  = uio_num;
    m_uio_map  = uio_map;
    m_tag_name = tag_name.empty() ? "FIFO Device" : "\"" + tag_name + "\" FIFO Device";

    m_dev      = new GMAPdevice(m_dev_addr, m_dev_size);
    m_uio      = new GUIOdevice(m_uio_num, m_uio_map);
    m_uio_regs = nullptr;
    m_is_ready = false;

    LOG_FORMAT(debug, "%s constructor [0x%08X, 0x%05X, %d, %d]", m_tag_name.c_str(), m_dev_addr, m_dev_size, m_uio_num, m_uio_map);
}

GFIFOdevice::GFIFOdevice(const GFIFOdevice& fifo_device) {
    *this = fifo_device;
}

GFIFOdevice::~GFIFOdevice() {
    if (m_is_ready) {
        Close();
    }
    delete m_dev;
    delete m_uio;

    LOG_FORMAT(debug, "%s destructor", m_tag_name.c_str());
}

GFIFOdevice& GFIFOdevice::operator=(const GFIFOdevice& fifo_device) {
    if (this != &fifo_device) {
        this->~GFIFOdevice();

        m_dev_addr = fifo_device.m_dev_addr;
        m_dev_size = fifo_device.m_dev_size;
        m_uio_num  = fifo_device.m_uio_num;
        m_uio_map  = fifo_device.m_uio_map;
        m_tag_name = fifo_device.m_tag_name;

        m_dev      = new GMAPdevice(m_dev_addr, m_dev_size);
        m_uio      = new GUIOdevice(m_uio_num, m_uio_map);
        m_uio_regs = nullptr;
        m_is_ready = false;

        LOG_FORMAT(trace, "%s constructor [0x%08X, 0x%05X, %d, %d]", m_tag_name.c_str(), m_dev_addr, m_dev_size, m_uio_num, m_uio_map);
    }
    return *this;
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
                    LOG_FORMAT(trace, "%s opened", m_tag_name.c_str());
                    goto jmp_exit;
                }
            }
        }
    }

    LOG_FORMAT(error, "%s open failure", m_tag_name.c_str());
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

    LOG_FORMAT(trace, "%s closed", m_tag_name.c_str());
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

uint32_t GFIFOdevice::PEEK(uint32_t offset, bool& error) {
    uint32_t _val{0};

    error = false;
    if (m_is_ready) {
        error = !m_dev->Read(offset, &_val);
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

uint32_t GFIFOdevice::GetTxPacketWords(bool& error) {
    uint32_t _val{0};

    error = false;
    if (m_is_ready) {
        error = !m_dev->Read(TX_PACKET_WORDS, &_val);
    }

    return _val;
}

uint32_t GFIFOdevice::GetTxUnusedWords(bool& error) {
    uint32_t _val{0};

    error = false;
    if (m_is_ready) {
        error = !m_dev->Read(TX_UNUSED_WORDS, &_val);
        _val  = 0x00001FFF & _val;
    }

    return _val;
}

uint32_t GFIFOdevice::GetRxLengthLevel(bool& error) {
    uint32_t _val{0};

    error = false;
    if (m_is_ready) {
        error = !m_dev->Read(RX_LENGTH_LEVEL, &_val);
        _val  = 0x0000FFFF & _val;
    }

    return _val;
}

uint32_t GFIFOdevice::GetRxPacketWords(bool& error) {
    uint32_t _val{0};

    error = false;
    if (m_is_ready) {
        error = !m_dev->Read(RX_PACKET_BYTES, &_val);
        _val  = 0x0000FFFF & ((_val + 1) / sizeof(uint16_t));
    }

    return _val;
}
