
#include <benchmark/benchmark.h>
#include <ctime> // clock_gettime, localtime_r

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

void GetDateTime_2(char* dst_buffer) {
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

    auto intrcpy = [](char* _dst, int __n, int __r) {
        while (true) {
            auto _div = __n / 10;
            auto _rem = __n % 10;

            if (!_div && !_rem) {
                break;
            }
            _dst[__r--] = '0' + _rem;

            __n = _div;
        }
    };

    intrcpy(dst_buffer, _Y, 3);
    intrcpy(dst_buffer, _M, 6);
    intrcpy(dst_buffer, _D, 9);
    intrcpy(dst_buffer, _h, 12);
    intrcpy(dst_buffer, _m, 15);
    intrcpy(dst_buffer, _s, 18);
    intrcpy(dst_buffer, _u, 25);
}

static void BM_date_time_snprintf(benchmark::State& state) {
    char _text[256];

    for (auto _ : state) {
        GetDateTime_1(_text, sizeof(_text));
    }
}

static void BM_date_time_intrcpy(benchmark::State& state) {
    //               0         1         2         3         4         5         6
    //               0123456789012345678901234567890123456789012345678901234567890123456789012345
    char _text[256]{"0000-00-00 00:00:00.000000 |           |                          (0000) | "};

    for (auto _ : state) {
        GetDateTime_2(_text);
    }
}

BENCHMARK(BM_date_time_snprintf);
BENCHMARK(BM_date_time_intrcpy);

BENCHMARK_MAIN();

// #include <iostream> // cout
// int main() {
//     char _text[256];
//
//     GetDateTime_2(_text, sizeof(_text));
//     std::cout << _text << std::endl;
//
//     return 0;
// }