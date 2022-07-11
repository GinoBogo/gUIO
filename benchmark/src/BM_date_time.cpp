
#include <benchmark/benchmark.h>
#include <cstring> // strncpy
#include <ctime>   // clock_gettime, localtime_r

void GetDateTime_1(char* dst_buffer, size_t dst_buffer_size) {
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

void GetDateTime_2(char* dst_buffer, size_t dst_buffer_size) {
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

    //                0         1         2
    //                01234567890123456789012345
    char timestamp[]{"0000-00-00 00:00:00.000000"};

    auto intrcpy = [&timestamp](int __n, int __r) {
        while (true) {
            auto _div = __n / 10;
            auto _rem = __n % 10;

            if (!_div && !_rem) {
                break;
            }
            timestamp[__r--] = '0' + _rem;

            __n = _div;
        }
    };

    intrcpy(_Y, 3);
    intrcpy(_M, 6);
    intrcpy(_D, 9);
    intrcpy(_h, 12);
    intrcpy(_m, 15);
    intrcpy(_s, 18);
    intrcpy(_u, 25);

    strncpy(dst_buffer, timestamp, dst_buffer_size);
}

static void BM_date_time_snprintf(benchmark::State& state) {
    char _text[512];

    for (auto _ : state) {
        GetDateTime_1(_text, sizeof(_text));
    }
}

static void BM_date_time_intrcpy(benchmark::State& state) {
    char _text[512];

    for (auto _ : state) {
        GetDateTime_2(_text, sizeof(_text));
    }
}

BENCHMARK(BM_date_time_snprintf);
BENCHMARK(BM_date_time_intrcpy);

BENCHMARK_MAIN();

// #include <iostream> // cout
// int main() {
//     char _text[512];
//
//     GetDateTime_2(_text, sizeof(_text));
//     std::cout << _text << std::endl;
//
//     return 0;
// }