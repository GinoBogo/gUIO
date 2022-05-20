
////////////////////////////////////////////////////////////////////////////////
/// \file      GDefine.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GLogger.hpp"

// SECTION: control flow statements

#define BREAK_IF(_condition) \
    if (_condition) break

#define BREAK_AND_LOG_IF(_condition, _type, _format, ...) \
    if (_condition) {                                     \
        LOG_FORMAT(_type, _format, __VA_ARGS__);          \
        break;                                            \
    }

#define CONTINUE_IF(_condition) \
    if (_condition) continue

#define CONTINUE_AND_LOG_IF(_condition, _type, _format, ...) \
    if (_condition) {                                        \
        LOG_FORMAT(_type, _format, __VA_ARGS__);             \
        continue;                                            \
    }

#define EXIT_IF(_condition, _status) \
    if (_condition) exit(_status)

#define EXIT_AND_LOG_IF(_condition, _status, _type, _format, ...) \
    if (_condition) {                                             \
        LOG_FORMAT(_type, _format, __VA_ARGS__);                  \
        exit(_status);                                            \
    }

#define GOTO_IF(_condition, _label) \
    if (_condition) goto _label

#define GOTO_AND_LOG_IF(_condition, _label, _type, _format, ...) \
    if (_condition) {                                            \
        LOG_FORMAT(_type, _format, __VA_ARGS__);                 \
        goto _label;                                             \
    }

#define RETURN_IF(_condition) \
    if (_condition) return

#define RETURN_AND_LOG_IF(_condition, _type, _format, ...) \
    if (_condition) {                                      \
        LOG_FORMAT(_type, _format, __VA_ARGS__);           \
        return;                                            \
    }

// SECTION: general purpose

#define UNUSED(_object) \
    if (_object) {      \
    }
