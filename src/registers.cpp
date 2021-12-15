////////////////////////////////////////////////////////////////////////////////
/// \file      registers.cpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "registers.hpp"

#include <cstdio> // snprintf

//==============================================================================
// LogiCORE IP: AXI GPIO v2.0
//==============================================================================

void GPIO_getRegistersInfo(void *base_addr, register_info *info_list) {
    info_list[0].offset = REG_GPIO_DATA_1;
    info_list[1].offset = REG_GPIO_TRI_1;
    info_list[2].offset = REG_GPIO_DATA_2;
    info_list[3].offset = REG_GPIO_TRI_2;
    info_list[4].offset = REG_GPIO_GIER;
    info_list[5].offset = REG_GPIO_IP_IER;
    info_list[6].offset = REG_GPIO_IP_ISR;

    info_list[0].value = __REG(base_addr, REG_GPIO_DATA_1);
    info_list[1].value = __REG(base_addr, REG_GPIO_TRI_1);
    info_list[2].value = __REG(base_addr, REG_GPIO_DATA_2);
    info_list[3].value = __REG(base_addr, REG_GPIO_TRI_2);
    info_list[4].value = __REG(base_addr, REG_GPIO_GIER);
    info_list[5].value = __REG(base_addr, REG_GPIO_IP_IER);
    info_list[6].value = __REG(base_addr, REG_GPIO_IP_ISR);

    snprintf(info_list[0].label, register_info_label_maxlen, "Channel 1 Data Register (R/W)");
    snprintf(info_list[1].label, register_info_label_maxlen, "Channel 1 3-state Control Register (R/W)");
    snprintf(info_list[2].label, register_info_label_maxlen, "Channel 2 Data Register (R/W)");
    snprintf(info_list[3].label, register_info_label_maxlen, "Channel 2 3-state Control Register (R/W)");
    snprintf(info_list[4].label, register_info_label_maxlen, "Global Interrupt Enable Register (R/W)");
    snprintf(info_list[5].label, register_info_label_maxlen, "IP Interrupt Enable Register (R/W)");
    snprintf(info_list[6].label, register_info_label_maxlen, "IP Interrupt Status Register (R/TOW)");
}

uint32_t GPIO_getDataCh1(void *base_addr) {
    return __REG(base_addr, REG_GPIO_DATA_1);
}

void GPIO_setDataCh1(void *base_addr, uint32_t value) {
    __REG(base_addr, REG_GPIO_DATA_1) = value;
}

uint32_t GPIO_getTriStateCh1(void *base_addr) {
    return __REG(base_addr, REG_GPIO_TRI_1);
}

void GPIO_setTriStateCh1(void *base_addr, uint32_t pins) {
    __REG(base_addr, REG_GPIO_TRI_1) = pins;
}

uint32_t GPIO_getDataCh2(void *base_addr) {
    return __REG(base_addr, REG_GPIO_DATA_2);
}

void GPIO_setDataCh2(void *base_addr, uint32_t value) {
    __REG(base_addr, REG_GPIO_DATA_2) = value;
}

uint32_t GPIO_getTriStateCh2(void *base_addr) {
    return __REG(base_addr, REG_GPIO_TRI_2);
}

void GPIO_setTriStateCh2(void *base_addr, uint32_t pins) {
    __REG(base_addr, REG_GPIO_TRI_2) = pins;
}

bool GPIO_getGlobalInterruptEnable(void *base_addr) {
    constexpr auto _val{SET_BIT(31)};
    return (__REG(base_addr, REG_GPIO_GIER) & _val) > 0;
}

void GPIO_setGlobalInterruptEnable(void *base_addr, bool enable) {
    constexpr auto _val{SET_BIT(31)};
    __REG(base_addr, REG_GPIO_GIER) = enable ? _val : 0;
}

bool GPIO_getInterruptEnableCh1(void *base_addr) {
    constexpr auto _val{SET_BIT(0)};
    return (__REG(base_addr, REG_GPIO_IP_IER) & _val) > 0;
}

void GPIO_setInterruptEnableCh1(void *base_addr, bool enable) {
    constexpr auto _set{SET_BIT(0)};
    constexpr auto _not{NOT_BIT(0)};
    const auto     _val{_not & __REG(base_addr, REG_GPIO_IP_IER)};
    __REG(base_addr, REG_GPIO_IP_IER) = enable ? _set | _val : _val;
}

bool GPIO_getInterruptEnableCh2(void *base_addr) {
    constexpr auto _val{SET_BIT(1)};
    return (__REG(base_addr, REG_GPIO_IP_IER) & _val) > 0;
}

void GPIO_setInterruptEnableCh2(void *base_addr, bool enable) {
    constexpr auto _set{SET_BIT(1)};
    constexpr auto _not{NOT_BIT(1)};
    const auto     _val{_not & __REG(base_addr, REG_GPIO_IP_IER)};
    __REG(base_addr, REG_GPIO_IP_IER) = enable ? _set | _val : _val;
}

bool GPIO_getInterruptStatusCh1(void *base_addr) {
    constexpr auto _val{SET_BIT(0)};
    return (__REG(base_addr, REG_GPIO_IP_ISR) & _val) > 0;
}

void GPIO_clrInterruptStatusCh1(void *base_addr) {
    constexpr auto _set{SET_BIT(0)};
    const auto     _val{__REG(base_addr, REG_GPIO_IP_IER) & _set};
    if (_val) {
        __REG(base_addr, REG_GPIO_IP_ISR) = _set;
    }
}

bool GPIO_getInterruptStatusCh2(void *base_addr) {
    constexpr auto _val{SET_BIT(1)};
    return (__REG(base_addr, REG_GPIO_IP_ISR) & _val) > 0;
}

void GPIO_clrInterruptStatusCh2(void *base_addr) {
    constexpr auto _set{SET_BIT(1)};
    const auto     _val{__REG(base_addr, REG_GPIO_IP_IER) & _set};
    if (_val) {
        __REG(base_addr, REG_GPIO_IP_ISR) = _set;
    }
}

//==============================================================================
// LogiCORE IP: AXI Quad SPI v3.2
//==============================================================================

/*
 * [AXI Quad SPI v3.2 LogicCORE IP Product Guide - PG153 August 6, 2021]
 *
 * The AXI Quad SPI core, when configured in STANDARD SPI mode, is a full-duplex
 * synchronous channel that supports a four-wire interface (receive, transmit, clock
 * and slave-select) between a master and a selected slave. When configured in
 * DUAL/QUAD SPI mode, this core supports additional pins for interfacing with external
 * memory.
 *
 * The core supports the Manual Slave Select mode as the default mode of operations for
 * slave select mode. This mode allows manual control of the slave select line with the
 * data written to the slave select register, thereby allowing transfers of an arbitrary
 * number of elements without toggling the slave line between elements. However, before
 * starting a new transfer, the slave select line must be toggled.
 */
