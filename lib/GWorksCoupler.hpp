
////////////////////////////////////////////////////////////////////////////////
/// \file      GWorksCoupler.hpp
/// \version   0.1
/// \date      May, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GWORKSCOUPLER_HPP
#define GWORKSCOUPLER_HPP

#include "GDefine.hpp"

#include <any>
#include <condition_variable>
#include <mutex>
#include <thread>

class GWorksCoupler {
    public:
    typedef void (*WorkFunc)(bool& quit, std::any& args);

    typedef struct work_funct_t {
        WorkFunc waiter_preamble = nullptr;
        WorkFunc waiter_calculus = nullptr;
        WorkFunc waiter_epilogue = nullptr;

        WorkFunc master_preamble = nullptr;
        WorkFunc master_calculus = nullptr;
        WorkFunc master_epilogue = nullptr;

    } work_func_t;

    GWorksCoupler(work_func_t& work_func, bool& quit, std::any& args, bool is_enabled = true) {
        RETURN_IF(!is_enabled,);

        t_waiter_group = std::thread([&] {
            CALL(work_func.waiter_preamble, quit, args);

            std::unique_lock _gate(m_mutex, std::defer_lock);
            while (!quit && !m_close) {
                DO_GUARD(_gate, m_event.wait(_gate, [&] { return m_count > 0; }); --m_count);
_work_label:
                GOTO_IF(quit || m_close, _exit_label,);

                work_func.waiter_calculus(quit, args);

                DO_GUARD(_gate, DEC_IF(m_count > 0, m_count, _loop,));

                GOTO_IF(_loop, _work_label,);
            }
_exit_label:
            CALL(work_func.waiter_epilogue, quit, args);
        });

        std::this_thread::sleep_for(std::chrono::microseconds(200));

        t_master_group = std::thread([&] {
            CALL(work_func.master_preamble, quit, args);

            std::unique_lock _gate(m_mutex, std::defer_lock);
            while (!quit && !m_close) {
                work_func.master_calculus(quit, args);

                DO(DO_GUARD(_gate, ++m_count); m_event.notify_one());
            }

            CALL(work_func.master_epilogue, quit, args);

            DO_IF(!m_close, Close());
        });
    }

    ~GWorksCoupler() {
        Close();
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }

    void Close() {
        DO_IF(!m_close, m_close = true, m_count = 1, m_event.notify_one());
    }

    void Wait() {
        DO_IF(t_waiter_group.joinable(), t_waiter_group.join());
        DO_IF(t_master_group.joinable(), t_master_group.join());
    }

    private:
    std::thread t_waiter_group;
    std::thread t_master_group;

    bool                    m_close{false};
    unsigned                m_count{0};
    std::mutex              m_mutex;
    std::condition_variable m_event;
};

#endif // GWORKSCOUPLER_HPP
