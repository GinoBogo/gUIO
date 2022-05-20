
////////////////////////////////////////////////////////////////////////////////
/// \file      GDefine.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#define BREAK_IF(_condition) \
    if (_condition) break

#define CONTINUE_IF(_condition) \
    if (_condition) continue

#define EXIT_IF(_condition, _status) \
    if (_condition) exit(_status)

#define GOTO_IF(_condition, _label) \
    if (_condition) goto _label

#define RETURN_IF(_condition) \
    if (_condition) return

#define UNUSED(_object) \
    if (_object) {      \
    }
