
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
        RETURN_IF(!is_enabled, );

        t_waiter_group = std::thread([&] {
            if (work_func.waiter_preamble != nullptr) {
                work_func.waiter_preamble(quit, args);
            }

            while (!quit && !m_close) {
                std::unique_lock _gate(m_mutex);
                m_event.wait(_gate, [&] { return m_total > 0; });
                m_total--;
                _gate.unlock();

_work_label:
                work_func.waiter_calculus(quit, args);

                DO_LOCK(_gate, auto _loop = IF(m_total > 0, m_total--));

                GOTO_IF(_loop, _work_label, );
            }

            if (work_func.waiter_epilogue != nullptr) {
                work_func.waiter_epilogue(quit, args);
            }
        });

        t_master_group = std::thread([&] {
            if (work_func.master_preamble != nullptr) {
                work_func.master_preamble(quit, args);
            }

            while (!quit && !m_close) {
                work_func.master_calculus(quit, args);

                std::lock_guard _gate(m_mutex);
                m_total++;
                m_event.notify_one();
            }

            if (work_func.master_epilogue != nullptr) {
                work_func.master_epilogue(quit, args);
            }

            if (!m_close) {
                Close();
            }
        });
    }

    ~GWorksCoupler() {
        Close();
    }

    void Close() {
        m_close = true;
        m_total = 1;
        m_event.notify_one();
    }

    void Wait() {
        if (t_waiter_group.joinable()) {
            t_waiter_group.join();
        }
        if (t_master_group.joinable()) {
            t_master_group.join();
        }
    }

    private:
    std::thread t_waiter_group;
    std::thread t_master_group;

    bool                    m_close{false};
    volatile unsigned       m_total{0};
    std::mutex              m_mutex;
    std::condition_variable m_event;
};

#endif // GWORKSCOUPLER_HPP