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
                m_is_ready = m_uio->MapToMemory();
                if (m_is_ready) {
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
    m_dev->Close();
    m_uio->Close();
    m_is_ready = false;
    LOG_WRITE(trace, "FIFO device close success");
}

bool GFIFOdevice::Reset() {
    auto _res{false};

    if (m_is_ready) {
        uint32_t _enable{0};
        uint32_t _forbid{1};

        _res = _res || m_dev->Write(0, &_enable);
        _res = _res && m_dev->Write(0, &_forbid);
    }

    return _res;
}

bool GFIFOdevice::SetPacketSize(uint32_t words) {
    auto _res{false};
    m_words = words;

    if (m_is_ready) {
        uint32_t _enable{0};
        uint32_t _forbid{1};

        _res = _res || m_dev->Write(0, &_enable);
        _res = _res && m_dev->Write(1, &m_words);
        _res = _res && m_dev->Write(0, &_forbid);
    }

    return _res;
}

uint32_t GFIFOdevice::GetPacketSize(bool* error) {
    uint32_t _val{0};
    auto     _res{false};

    if (m_is_ready) {
        _res = m_dev->Read(1, &_val);
    }

    if (error != nullptr) {
        *error = !_res || (_val != m_words);
    }

    return _val;
}

uint32_t GFIFOdevice::GetFreeWords(bool* error) {
    uint32_t _val{0};
    auto     _res{false};

    if (m_is_ready) {
        _res = m_dev->Read(2, &_val);
        _val = 0x00001FFF & _val;
    }

    if (error != nullptr) {
        *error = !_res;
    }

    return _val;
}
