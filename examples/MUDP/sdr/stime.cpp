////////////////////////////////////////////////////////////////////////////////
/// \file      stime.cpp
/// \version   0.1
/// \date      February, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "stime.hpp"

#include <unistd.h>

void STIME_uSleep(unsigned long delay) {
    usleep(delay);
}

void STIME_mSleep(unsigned long delay) {
    usleep(delay * 1000);
}
