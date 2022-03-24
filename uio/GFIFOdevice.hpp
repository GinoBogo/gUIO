////////////////////////////////////////////////////////////////////////////////
/// \file      GFIFOdevice.hpp
/// \version   0.1
/// \date      March 15, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GFIFODEVICE_HPP
#define GFIFODEVICE_HPP

#include "GMAPdevice.hpp"
#include "GRegisters.hpp"
#include "GUIOdevice.hpp"

class GFIFOdevice {
    public:
    GFIFOdevice(size_t dev_addr, size_t dev_size, int uio_num, int uio_map);
    ~GFIFOdevice();

    bool     Open();
    void     Close();
    bool     Reset();
    bool     SetPacketWords(uint32_t words);
    uint32_t GetPacketWords(bool* error = nullptr);
    uint32_t GetFreeWords(bool* error = nullptr);

    bool WritePacket(uint16_t* src_buf, size_t words) {
        if (m_is_ready) {
            return m_dev->OverWrite(4, src_buf, words);
        }
        return false;
    }

    bool ReadPacket(uint16_t* dst_buf, size_t words) {
        if (m_is_ready) {
            return m_dev->OverRead(4, dst_buf, words);
        }
        return false;
    }

    bool EnableReader(bool mode = true) {
        if (m_is_ready) {
            auto _val{mode ? SET_BIT(31) : 0};
            return m_dev->Write(0, &_val);
        }
        return false;
    }

    bool ClearEvent() {
        if (m_is_ready) {
            auto _val{GPIO_getIpInterruptStatus(m_uio_regs)};
            if (_val & BIT_GPIO_IP_ISR_1) {
                GPIO_setIpInterruptStatus(m_uio_regs, BIT_GPIO_IP_ISR_1);
            }
            return m_uio->IRQ_Clear();
        }
        return false;
    }

    bool WaitEvent(int timeout = -1) {
        if (m_is_ready) {
            return m_uio->IRQ_Wait(timeout);
        }
        return false;
    }

    private:
    size_t   m_dev_addr;
    size_t   m_dev_size;
    int      m_uio_num;
    int      m_uio_map;
    uint32_t m_words;

    GMAPdevice* m_dev;
    GUIOdevice* m_uio;
    uint8_t*    m_uio_regs;
    bool        m_is_ready;
};

#endif // GFIFODEVICE_HPP
