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
    enum t_fifo_regs {
        IP_CONTROL      = 0,
        TX_PACKET_WORDS = 1,
        TX_UNUSED_WORDS = 2,
        RX_LENGTH_LEVEL = 2,
        TX_EVENTS_COUNT = 3,
        RX_PACKET_BYTES = 7,
        TX_BUFFER_BEGIN = 8,
        RX_BUFFER_BEGIN = 8,
    };

    GFIFOdevice(size_t dev_addr, size_t dev_size, int uio_num, int uio_map);
    ~GFIFOdevice();

    bool Open();
    void Close();
    bool Reset();

    uint32_t PEEK(uint32_t offset, bool& error);
    bool     POKE(uint32_t offset, uint32_t value);

    bool     SetTxPacketWords(uint32_t words);
    uint32_t GetTxPacketWords(bool& error);
    uint32_t GetTxUnusedWords(bool& error);
    uint32_t GetRxLengthLevel(bool& error);
    uint32_t GetRxPacketWords(bool& error);

    bool ReadPacket(uint16_t* dst_buf, size_t words) {
        if (m_is_ready) {
            return m_dev->OverRead(RX_BUFFER_BEGIN, dst_buf, words);
        }
        return false;
    }

    bool WritePacket(uint16_t* src_buf, size_t words) {
        if (m_is_ready) {
            return m_dev->OverWrite(TX_BUFFER_BEGIN, src_buf, words);
        }
        return false;
    }

    bool EnableReader(bool enable = true) {
        if (m_is_ready) {
            uint32_t _val{enable ? SET_BIT(31) : 0};
            return m_dev->Write(IP_CONTROL, &_val);
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

    bool WaitThenClearEvent(int timeout = -1) {
        if (m_is_ready) {
            if (m_uio->IRQ_Wait(timeout)) {
                auto _val{GPIO_getIpInterruptStatus(m_uio_regs)};
                if (_val & BIT_GPIO_IP_ISR_1) {
                    GPIO_setIpInterruptStatus(m_uio_regs, BIT_GPIO_IP_ISR_1);
                }
                return m_uio->IRQ_Clear();
            }
        }
        return false;
    }

    private:
    size_t m_dev_addr;
    size_t m_dev_size;
    int    m_uio_num;
    int    m_uio_map;

    GMAPdevice* m_dev;
    GUIOdevice* m_uio;
    uint8_t*    m_uio_regs;
    bool        m_is_ready;
};

#endif // GFIFODEVICE_HPP
