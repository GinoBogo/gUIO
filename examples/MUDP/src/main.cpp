////////////////////////////////////////////////////////////////////////////////
/// \file      main.cpp
/// \version   0.1
/// \date      November, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "FiFo.hpp"
#include "GLogger.hpp"

int main() {
    GLogger::Initialize("example_MUDP.log");
    LOG_WRITE(trace, "Process STARTED");

    auto fifo = FiFo(200, 100);

    LOG_WRITE(trace, "Process STOPPED");
    return 0;
}
