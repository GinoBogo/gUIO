
////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      September, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GLogger.hpp"
#include "GRegisters.hpp"
#include "GUIOdevice.hpp"

#include <filesystem> // path

void GPIO_printRegistersInfo(GUIOdevice* uio_dev) {
    register_info info_list[gpio_registers_number];

    auto* base_addr{uio_dev->virt_addr()};

    GPIO_getRegistersInfo(base_addr, info_list);

    LOG_FORMAT(info, "GPIO_%d Registers Info <OFFSET> <VALUE> <LABEL>:", uio_dev->uio_num());
    for (const auto& item : info_list) { //
        LOG_FORMAT(info, "  0x%04x | 0x%08x | %s", item.offset, item.value, item.label);
    }
}

// #define QEMU

#ifdef QEMU
#define UIO_NUM 4
#else
#define UIO_NUM 0
#endif

int main(int argc, char* argv[]) {
    auto exec     = std::filesystem::path(argv[0]);
    auto exec_log = exec.stem().concat(".log");

    GLogger::Initialize(exec_log.c_str());
    LOG_FORMAT(trace, "Process STARTED (%s)", exec.stem().c_str());

    auto uio_dev{GUIOdevice(UIO_NUM, 0)};

    if (uio_dev.Open()) {
        if (uio_dev.MapToMemory()) {
            uio_dev.PrintMapAttributes();

            auto* base_addr{uio_dev.virt_addr()};

            // SECTION: interrupts
            GPIO_setIpInterruptEnable(base_addr, 1 * BIT_GPIO_IP_IER_2);
            GPIO_setGlobalInterruptEnable(base_addr, 1 * BIT_GPIO_GIER);

            GPIO_printRegistersInfo(&uio_dev);

            GPIO_setIpInterruptStatus(base_addr, BIT_GPIO_IP_ISR_2);
            uio_dev.IRQ_Clear();
            for (auto i{0}; i < 8; ++i) {
                if (uio_dev.IRQ_Wait()) {
                    GPIO_setDataCh1(base_addr, i % 2 ? 0b11110000 : 0b00001111);

                    LOG_FORMAT(debug, "IRQ counter: %d", uio_dev.irq_count());

                    GPIO_setIpInterruptStatus(base_addr, BIT_GPIO_IP_ISR_2);
                    uio_dev.IRQ_Clear();
                }
            }

            GPIO_setGlobalInterruptEnable(base_addr, 0 * BIT_GPIO_GIER);
        }
        uio_dev.Close();
    }

    LOG_FORMAT(trace, "Process STOPPED (%s)", exec.stem().c_str());
    return 0;
}
