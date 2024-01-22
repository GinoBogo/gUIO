
////////////////////////////////////////////////////////////////////////////////
/// \file      GDefine.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GDEFINE_HPP
#define GDEFINE_HPP

// SECTION: control flow statements

// clang-format off
#define BREAK_IF_BUT_LOG(_condition, _type, _format, ...)         /**/ if (_condition) { LOG_FORMAT(_type, _format, __VA_ARGS__); break; }
#define BREAK_IF_BUT(_condition, ...)                             /**/ if (_condition) { __VA_ARGS__; break; }
#define BREAK_IF_OR_LOG(_condition, _type, _format, ...)          /**/ if (_condition) break; LOG_FORMAT(_type, _format, __VA_ARGS__)
#define BREAK_IF_OR(_condition, ...)                              /**/ if (_condition) break; __VA_ARGS__
#define BREAK_IF(_condition, ...)                                 /**/ __VA_ARGS__; if (_condition) break
#define CALL(_func_ptr, ...)                                      /**/ if (_func_ptr != nullptr) _func_ptr(__VA_ARGS__)
#define CONTINUE_IF_BUT_LOG(_condition, _type, _format, ...)      /**/ if (_condition) { LOG_FORMAT(_type, _format, __VA_ARGS__); continue; }
#define CONTINUE_IF_BUT(_condition, ...)                          /**/ if (_condition) { __VA_ARGS__; continue; }
#define CONTINUE_IF_OR_LOG(_condition, _type, _format, ...)       /**/ if (_condition) continue; LOG_FORMAT(_type, _format, __VA_ARGS__)
#define CONTINUE_IF_OR(_condition, ...)                           /**/ if (_condition) continue; __VA_ARGS__
#define CONTINUE_IF(_condition, ...)                              /**/ __VA_ARGS__; if (_condition) continue
#define DEC_IF(_condition, _variable, _result, ...)               /**/ bool _result = _condition; _variable -= (decltype(_variable))_result
#define DO_BLOCK_IF(_condition, ...)                              /**/ if (_condition) { __VA_ARGS__; }
#define DO_BLOCK(...)                                             /**/ { __VA_ARGS__; }
#define DO_GUARD(_unique_lock, ...)                               /**/ _unique_lock.lock(); __VA_ARGS__; _unique_lock.unlock()
#define DO_IF(_condition, ...)                                    /**/ if (_condition) __VA_ARGS__
#define DO_LOCK(_unique_lock, ...)                                /**/ _unique_lock.lock(); __VA_ARGS__
#define DO_UNLOCK(_unique_lock, ...)                              /**/ __VA_ARGS__; _unique_lock.unlock()
#define DO(...)                                                   /**/ __VA_ARGS__
#define EXIT_IF_BUT_LOG(_condition, _status, _type, _format, ...) /**/ if (_condition) { LOG_FORMAT(_type, _format, __VA_ARGS__); exit(_status); }
#define EXIT_IF_BUT(_condition, _status, ...)                     /**/ if (_condition) { __VA_ARGS__; exit(_status); }
#define EXIT_IF_OR_LOG(_condition, _status, _type, _format, ...)  /**/ if (_condition) exit(_status); LOG_FORMAT(_type, _format, __VA_ARGS__)
#define EXIT_IF_OR(_condition, _status, ...)                      /**/ if (_condition) exit(_status); __VA_ARGS__
#define EXIT_IF(_condition, _status, ...)                         /**/ __VA_ARGS__; if (_condition) exit(_status)
#define GOTO_IF_BUT_LOG(_condition, _label, _type, _format, ...)  /**/ if (_condition) { LOG_FORMAT(_type, _format, __VA_ARGS__); goto _label; }
#define GOTO_IF_BUT(_condition, _label, ...)                      /**/ if (_condition) { __VA_ARGS__; goto _label; }
#define GOTO_IF_OR_LOG(_condition, _label, _type, _format, ...)   /**/ if (_condition) goto _label; LOG_FORMAT(_type, _format, __VA_ARGS__)
#define GOTO_IF_OR(_condition, _label, ...)                       /**/ if (_condition) goto _label; __VA_ARGS__
#define GOTO_IF(_condition, _label, ...)                          /**/ __VA_ARGS__; if (_condition) goto _label
#define IF(_condition, ...)                                       /**/ _condition; if (_condition) __VA_ARGS__
#define INC_IF(_condition, _variable, _result, ...)               /**/ bool _result = _condition; _variable += (decltype(_variable))_result
#define LOG_IF(_condition, _type, _format, ...)                   /**/ if (_condition) LOG_FORMAT(_type, _format, __VA_ARGS__)
#define RETURN_IF_BUT_LOG(_condition, _type, _format, ...)        /**/ if (_condition) { LOG_FORMAT(_type, _format, __VA_ARGS__); return; }
#define RETURN_IF_BUT(_condition, ...)                            /**/ if (_condition) { __VA_ARGS__; return; }
#define RETURN_IF_OR_LOG(_condition, _type, _format, ...)         /**/ if (_condition) return; LOG_FORMAT(_type, _format, __VA_ARGS__)
#define RETURN_IF_OR(_condition, ...)                             /**/ if (_condition) return; __VA_ARGS__
#define RETURN_IF(_condition, ...)                                /**/ __VA_ARGS__; if (_condition) return
// clang-format on

// SECTION: general purpose

// clang-format off
#ifndef UNUSED
#define UNUSED(x) if (x) {}
#endif
// clang-format on

#endif // GDEFINE_HPP
