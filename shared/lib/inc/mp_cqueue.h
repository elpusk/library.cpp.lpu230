#pragma once

#include <deque>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <memory>

namespace _mp{

    template<typename T>
    class cqueue{
    public:
        void push(const T& value) 
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_q.push_back(value);
        }

        bool try_pop(T& value) 
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_q.empty()) {
                return false;
            }
            value = m_q.front();
            m_q.pop_front();
            return true;
        }

        void clear()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::deque<T> q;
            m_q.swap(q);
        }
    private:
        std::deque<T> m_q;
        std::mutex m_mutex;
    };

}