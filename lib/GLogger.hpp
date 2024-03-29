
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
#include <cstdint> // uint16_t
#include <cstdio>  // snprintf

constexpr const char* _filename_(const char* path) {
    const char* _name = path;
    while (*path != 0) {
        char ch = *path++;
        if (ch == '/' || ch == '\\') {
            _name = path;
        }
    }
    return _name;
}

#define __FILENAME__ (_filename_(__FILE__))

#define LOG_MSG_MAXLEN                256
#define LOG_TYPE(type)                GLogger::type, __FILENAME__, __LINE__
#define LOG_WRITE(type, message)      GLogger::Write(LOG_TYPE(type), message)
#define LOG_FORMAT(type, format, ...) GLogger::Format(LOG_TYPE(type), format, __VA_ARGS__)

namespace GLogger {

    enum type_t { debug, error, fatal, info, trace, warning };

    void Initialize(const char* filename, const char* udp_server_addr = nullptr, uint16_t udp_server_port = 0);

    void Write(type_t type, const char* file, size_t line, const char* message);

    template <class... Args> void Format(type_t type, const char* file, size_t line, const char* format, Args... args) {
        char msg[LOG_MSG_MAXLEN];
        snprintf(msg, sizeof(msg), format, args...);
        Write(type, file, line, msg);
    }

    char* AlignToLeft(const char* src, char* dst, size_t span, char filler = ' ');
    char* AlignToCenter(const char* src, char* dst, size_t span, char filler = ' ');
    char* AlignToRight(const char* src, char* dst, size_t span, char filler = ' ');
} // namespace GLogger

#endif // GLOGGER_HPP
