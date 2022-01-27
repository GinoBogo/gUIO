////////////////////////////////////////////////////////////////////////////////
/// \file      GLogger.hpp
/// \version   0.1
/// \date      August, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GLOGGER_HPP
#define GLOGGER_HPP

#include <cstddef> // size_t
#include <cstdio>  // snprintf

#define LOG_MSG_MAXLEN                256
#define LOG_TYPE(type)                GLogger::type, __FILE__, __LINE__
#define LOG_WRITE(type, message)      GLogger::Write(LOG_TYPE(type), message)
#define LOG_FORMAT(type, format, ...) GLogger::Format(LOG_TYPE(type), format, __VA_ARGS__)

namespace GLogger {

    enum Type { debug, error, fatal, info, trace, warning };

    enum Alignment { left, center, right };

    void Initialize(const char *file_path);

    void Write(Type type, const char *file, size_t line, const char *message);

    template <class... Args> void Format(Type type, const char *file, size_t line, const char *format, Args... args) {
        char msg[LOG_MSG_MAXLEN];
        snprintf(msg, sizeof(msg), format, args...);
        Write(type, file, line, msg);
    }

    char *AlignText(Alignment mode, const char *src, char *dst, size_t span, char filler = ' ');
    char *AlignToLeft(const char *src, char *dst, size_t span, char filler = ' ');
    char *AlignToCenter(const char *src, char *dst, size_t span, char filler = ' ');
    char *AlignToRight(const char *src, char *dst, size_t span, char filler = ' ');
} // namespace GLogger

#endif // GLOGGER_HPP
