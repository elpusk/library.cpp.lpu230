#pragma once

#include <queue>
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
            m_q.push(value);
        }

        bool try_pop(T& value) 
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_q.empty()) {
                return false;
            }
            value = m_q.front();
            m_q.pop();
            return true;
        }

    private:
        std::queue<T> m_q;
        std::mutex m_mutex;
    };

}