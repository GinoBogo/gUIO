
////////////////////////////////////////////////////////////////////////////////
/// \file      GLogger.cpp
/// \version   0.1
/// \date      August, 2021
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GLogger.hpp"

#include "GString.hpp"
#include "GUdpStreamWriter.hpp"

#include <algorithm>  // min
#include <ctime>      // localtime_r, timespec_get
#include <filesystem> // path
#include <fstream>    // ifstream, ofstream
#include <iostream>   // cout

constexpr const char* file_name(const char* path) {
    const char* _file = path;
    while (*path != 0) {
        if (*path++ == '/') {
            _file = path;
        }
    }
    return _file;
}

constexpr const char* last_dot(const char* path) {
    const char* _last = nullptr;
    while (*path != 0) {
        if (*path == '.') {
            _last = path;
        }
        ++path;
    }
    return _last != nullptr ? _last : path;
}

namespace GLogger {
    using namespace std;

    GUdpStreamWriter sout{};

    std::ofstream fout{};

    bool is_open{false};

    const char* flags[6] = {"DEBUG", "*ERROR", "*FATAL", "INFO", "TRACE", "*WARNING"};

    enum alignment_t { LEFT, CENTER, RIGHT };

    // WARNING: unsafe function
    void GetDateTime(char* dst_buffer, size_t dst_buffer_size) {
        struct timespec _ts;
        clock_gettime(CLOCK_REALTIME, &_ts);

        struct tm _tm;
        localtime_r(&_ts.tv_sec, &_tm);

        auto _Y{_tm.tm_year + 1900};
        auto _M{_tm.tm_mon + 1};
        auto _D{_tm.tm_mday};
        auto _h{_tm.tm_hour};
        auto _m{_tm.tm_min};
        auto _s{_tm.tm_sec};
        auto _u{static_cast<int>(_ts.tv_nsec / 1000)};

        snprintf(dst_buffer, dst_buffer_size, "%04d-%02d-%02d %02d:%02d:%02d.%06d", _Y, _M, _D, _h, _m, _s, _u);
    }

    void initialize_stream(const char* filename, const char* udp_server_addr = nullptr, uint16_t udp_server_port = 0) {
        std::cout.sync_with_stdio(false); // INFO: on some platforms, stdout flushes on '\n'

        std::string _addr;
        uint16_t    _port;

        if (udp_server_addr != nullptr) {
            _addr = udp_server_addr;
            _port = udp_server_port;
        }
        else {
            auto _fs = ifstream(std::string(filename) + "_cfg");
            if (_fs.is_open()) {
                std::string _line;

                while (std::getline(_fs, _line)) {
                    GString::sanitize(_line);

                    if (_line.find("udp_server_addr") != std::string::npos) {
                        auto _pair = GString::split(_line, "[=]");
                        if (_pair.size() > 1) {
                            _addr = _pair[1];
                            continue;
                        }
                    }

                    if (_line.find("udp_server_port") != std::string::npos) {
                        auto _pair = GString::split(_line, "[=]");
                        if (_pair.size() > 1) {
                            _port = GString::strtous(_pair[1]);
                            continue;
                        }
                    }
                }
                _fs.close();
            }
        }

        sout.open(_addr, _port);
        fout.open(filename);
        is_open = fout.is_open();

        sout << '\n';
        fout << '\n'; // NOTE: std::endl puts '\n' in the stream, then flushes
        cout << '\n';

        sout.flush();
        fout.flush();
        cout.flush();
    }

    void Initialize(const char* filename, const char* udp_server_addr, uint16_t udp_server_port) {
        is_open = fout.is_open();
        if (is_open) {
            LOG_WRITE(warning, "File stream already opened");
        }
        else {
            initialize_stream(filename, udp_server_addr, udp_server_port);
        }
    }

    // WARNING: unsafe function
    void Write(type_t type, const char* file, size_t line, const char* message) {
        char _text[LOG_MSG_MAXLEN];

        GetDateTime(_text, sizeof(_text));

        const auto* _flag{flags[type]};
        const auto* _name{file_name(file)};

        snprintf(_text + 26, sizeof(_text) - 26, " | %9s | %24s (%04lu) | %s", _flag, _name, line, message);

        if (!is_open) {
            auto  name_len = strnlen(_name, 256) + 5;
            auto* name_log = new char[name_len];

            strncpy(name_log, _name, name_len);
            strncpy((char*)last_dot(name_log), ".log", name_len);

            initialize_stream(name_log);

            delete[] name_log;
        }

        sout << _text << '\n';
        fout << _text << '\n';
        cout << _text << '\n';

        sout.flush();
        fout.flush();
        cout.flush();
    }

    // WARNING: unsafe function
    char* align_text(alignment_t mode, const char* src, char* dst, size_t span, char filler) {
        if (src == nullptr || dst == nullptr) {
            return nullptr;
        }

        memset(dst, filler, span);
        size_t dst_shift = 0;
        size_t src_len   = strnlen(src, span);

        switch (mode) {
            case RIGHT: {
                dst_shift = (span - src_len);
            } break;

            case CENTER: {
                dst_shift = (span - src_len) / 2;
            } break;

            case LEFT:
            default:
                break;
        }

        src_len = std::min(src_len, span - dst_shift);
        memcpy(dst + dst_shift, src, src_len);
        dst[span] = 0;

        return dst;
    }

    char* AlignToLeft(const char* src, char* dst, size_t span, char filler) {
        return align_text(LEFT, src, dst, span, filler);
    }

    char* AlignToCenter(const char* src, char* dst, size_t span, char filler) {
        return align_text(CENTER, src, dst, span, filler);
    }

    char* AlignToRight(const char* src, char* dst, size_t span, char filler) {
        return align_text(RIGHT, src, dst, span, filler);
    }
} // namespace GLogger
