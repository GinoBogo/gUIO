
////////////////////////////////////////////////////////////////////////////////
/// \file      globals.hpp
/// \version   0.1
/// \date      July, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "GOptions.hpp"

// SECTION: PL_to_PS global variables
extern std::string  PL2PS_REGS_TAG_NAME;
extern unsigned int PL2PS_REGS_DEV_ADDR;
extern unsigned int PL2PS_REGS_DEV_SIZE;
extern unsigned int PL2PS_REGS_DEV_NUMB;

// SECTION: PS_to_PL global variables
extern std::string  PS2PL_REGS_TAG_NAME;
extern unsigned int PS2PL_REGS_DEV_ADDR;
extern unsigned int PS2PL_REGS_DEV_SIZE;
extern unsigned int PS2PL_REGS_DEV_NUMB;

// ============================================================================

namespace Global {
    void load_options(const std::string& filename);

} // namespace Global

#endif // GLOBALS_HPP
