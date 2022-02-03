////////////////////////////////////////////////////////////////////////////////
/// \file      GRegisters.cpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GRegisters.hpp"

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
