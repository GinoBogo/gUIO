
////////////////////////////////////////////////////////////////////////////////
/// \file      GBarrier.hpp
/// \version   0.1
/// \date      September, 2022
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#ifndef GBARRIER_HPP
#define GBARRIER_HPP

#include <condition_variable> // condition_variable
#include <mutex>              // defer_lock, mutex, unique_lock

class GBarrier {
  public:
    void Close() {
        std::unique_lock _gate(m_mutex, std::defer_lock);
        _gate.lock();
        m_is_open = false;
        _gate.unlock();
    }

    void Open() {
        std::unique_lock _gate(m_mutex, std::defer_lock);
        _gate.lock();
        m_is_open = true;
        _gate.unlock();
        m_event.notify_one();
    }

    void Wait() {
        std::unique_lock _gate(m_mutex, std::defer_lock);
        _gate.lock();
        m_event.wait(_gate, [&] { return m_is_open; });
        _gate.unlock();
    }

  private:
    std::condition_variable m_event;
    std::mutex              m_mutex;
    bool                    m_is_open{false};
};

#endif // GBARRIER_HPP
