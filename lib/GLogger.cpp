////////////////////////////////////////////////////////////////////////////////
/// \file      GLogger.cpp
/// \version   0.1
/// \date      August, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GLogger.hpp"

#include <algorithm>  // min
#include <cstring>    // strncpy, strnlen, memset, memcpy
#include <ctime>      // localtime, timespec_get
#include <filesystem> // path
#include <fstream>    // ofstream
#include <iostream>   // cout

constexpr const char *file_name(const char *path) {
    const char *_file = path;
    while (*path) {
        if (*path++ == '/') {
            _file = path;
        }
    }
    return _file;
}

constexpr const char *last_dot(const char *path) {
    const char *_last = nullptr;
    while (*path) {
        if (*path == '.') {
            _last = path;
        }
        ++path;
    }
    return _last ? _last : path;
}

namespace GLogger {

    static bool is_open{false};

    static std::ofstream fout{};

    static const char *flags[6] = {"DEBUG", "ERROR", "FATAL", "INFO", "TRACE", "WARNING"};

    // WARNING: unsafe function
    void GetDateTime(char *dst_buffer, size_t dst_buffer_size) {
        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        auto tm{std::localtime(&ts.tv_sec)};
        auto _Y{tm->tm_year + 1900};
        auto _M{tm->tm_mon + 1};
        auto _D{tm->tm_mday};
        auto _h{tm->tm_hour};
        auto _m{tm->tm_min};
        auto _s{tm->tm_sec};
        auto _u{static_cast<int>(ts.tv_nsec / 1000)};

        snprintf(dst_buffer, dst_buffer_size, "%04d-%02d-%02d %02d:%02d:%02d.%06d", _Y, _M, _D, _h, _m, _s, _u);
    }

    void Initialize(const char *file_path) {
        is_open = fout.is_open();
        if (is_open) {
            LOG_WRITE(warning, "File stream already opened");
        }
        else {
            fout    = std::ofstream(file_path);
            is_open = fout.is_open();
        }
    }

    // WARNING: unsafe function
    void Write(Type type, const char *file, size_t line, const char *message) {
        char _text[LOG_MSG_MAXLEN];

        GetDateTime(_text, sizeof(_text));

        auto _flag{flags[type]};
        auto _name{file_name(file)};

        snprintf(_text + 26, sizeof(_text) - 26, " | %8s | %24s (%04ld) | %s", _flag, _name, line, message);

        if (!is_open) {
            auto name_len = strnlen(_name, 256) + 5;
            auto name_log = new char[name_len];
            strncpy(name_log, _name, name_len);
            strncpy((char *)last_dot(name_log), ".log", name_len);

            fout    = std::ofstream(name_log);
            is_open = fout.is_open();

            delete[] name_log;
        }

        using namespace std;
        cout << _text << endl;
        fout << _text << endl;

        cout.flush();
        fout.flush();
    }

    // WARNING: unsafe function
    char *AlignText(Alignment mode, const char *src, char *dst, size_t span, char filler) {
        if (src == nullptr || dst == nullptr) {
            return nullptr;
        }

        memset(dst, filler, span);
        size_t dst_shift = 0;
        size_t src_len   = strnlen(src, span);

        switch (mode) {
            case right: {
                dst_shift = (span - src_len);
            } break;

            case center: {
                dst_shift = (span - src_len) / 2;
            } break;

            case left:
            default:
                break;
        }

        src_len = std::min(src_len, span - dst_shift);
        memcpy(dst + dst_shift, src, src_len);
        dst[span] = 0;

        return dst;
    }

    char *AlignToLeft(const char *src, char *dst, size_t span, char filler) {
        return AlignText(left, src, dst, span, filler);
    }

    char *AlignToCenter(const char *src, char *dst, size_t span, char filler) {
        return AlignText(center, src, dst, span, filler);
    }

    char *AlignToRight(const char *src, char *dst, size_t span, char filler) {
        return AlignText(right, src, dst, span, filler);
    }
} // namespace GLogger
