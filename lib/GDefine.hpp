
////////////////////////////////////////////////////////////////////////////////
/// \file      GDefine.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GDEFINE_HPP
#define GDEFINE_HPP

#include "GLogger.hpp"

// SECTION: control flow statements

#define LOG_IF(_condition, _type, _format, ...)  \
    if (_condition) {                            \
        LOG_FORMAT(_type, _format, __VA_ARGS__); \
    }

#define BREAK_IF(_condition, ...) \
    __VA_ARGS__;                  \
    if (_condition) break

#define BREAK_IF_THEN_LOG(_condition, _type, _format, ...) \
    if (_condition) {                                      \
        LOG_FORMAT(_type, _format, __VA_ARGS__);           \
        break;                                             \
    }

#define BREAK_IF_ELSE_LOG(_condition, _type, _format, ...) \
    if (_condition) {                                      \
        break;                                             \
    }                                                      \
    LOG_FORMAT(_type, _format, __VA_ARGS__);

#define CONTINUE_IF(_condition, ...) \
    __VA_ARGS__;                     \
    if (_condition) continue

#define CONTINUE_IF_THEN_LOG(_condition, _type, _format, ...) \
    if (_condition) {                                         \
        LOG_FORMAT(_type, _format, __VA_ARGS__);              \
        continue;                                             \
    }

#define CONTINUE_IF_ELSE_LOG(_condition, _type, _format, ...) \
    if (_condition) {                                         \
        continue;                                             \
    }                                                         \
    LOG_FORMAT(_type, _format, __VA_ARGS__);

#define EXIT_IF(_condition, _status, ...) \
    __VA_ARGS__;                          \
    if (_condition) exit(_status)

#define EXIT_IF_THEN_LOG(_condition, _status, _type, _format, ...) \
    if (_condition) {                                              \
        LOG_FORMAT(_type, _format, __VA_ARGS__);                   \
        exit(_status);                                             \
    }

#define EXIT_IF_ELSE_LOG(_condition, _status, _type, _format, ...) \
    if (_condition) {                                              \
        exit(_status);                                             \
    }                                                              \
    LOG_FORMAT(_type, _format, __VA_ARGS__);

#define GOTO_IF(_condition, _label, ...) \
    __VA_ARGS__;                         \
    if (_condition) goto _label

#define GOTO_IF_THEN_LOG(_condition, _label, _type, _format, ...) \
    if (_condition) {                                             \
        LOG_FORMAT(_type, _format, __VA_ARGS__);                  \
        goto _label;                                              \
    }

#define GOTO_IF_ELSE_LOG(_condition, _label, _type, _format, ...) \
    if (_condition) {                                             \
        goto _label;                                              \
    }                                                             \
    LOG_FORMAT(_type, _format, __VA_ARGS__);

#define RETURN_IF(_condition, ...) \
    __VA_ARGS__;                   \
    if (_condition) return

#define RETURN_IF_THEN_LOG(_condition, _type, _format, ...) \
    if (_condition) {                                       \
        LOG_FORMAT(_type, _format, __VA_ARGS__);            \
        return;                                             \
    }

#define RETURN_IF_ELSE_LOG(_condition, _type, _format, ...) \
    if (_condition) {                                       \
        return;                                             \
    }                                                       \
    LOG_FORMAT(_type, _format, __VA_ARGS__);

// SECTION: general purpose

// clang-format off
#ifndef UNUSED
#define UNUSED(x) if (x) {}
#endif
// clang-format on

#endif // GDEFINE_HPP