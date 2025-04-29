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

        /**
        * @brief push one item.
        * 
        *   be protected by mutex.
        */
        void push(const T& value) 
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_q.push_back(value);
        }

        /**
        * @brief push two items.
        *
        *   be protected by mutex.
        */
        void push(const T& value0, const T& value1)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_q.push_back(value0);
            m_q.push_back(value1);
        }

        /**
        * @brief try pop the front item from queue.
        *
        *   be protected by mutex.
        * 
        * @param [in/out] value - if return value is true, value is set by th poped item.
        * 
        * @return bool - true : success pop. false - none item.
        */
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

        /**
        * @brief try pop all item of queue.
        *
        *   be protected by mutex.
        *
        * @param [in/out] q_of_value - if return is greater then zero, q_of_value is set by th poped items.
        *
        * @return size_t - the number of poped items.
        */
        size_t try_pops(std::deque<T>& q_of_value)
        {
            size_t n_size(0);
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_q.empty()) {
                return n_size;
            }
            do {
                T value = m_q.front();
                m_q.pop_front();
                q_of_value.push_back(value);
                ++n_size;
            } while (!m_q.empty());

            return n_size;
        }

        /**
        * @brief clear all item of queue.
        *
        *   be protected by mutex.
        */
        void clear()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_q.clear();
        }

        /**
        * @brief get size of queue.
        *
        *   be protected by mutex.
        *
        * @return size_t - the number of queue items.
        */
        size_t size()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_q.size();
        }
    private:
        std::deque<T> m_q;
        std::mutex m_mutex;
    };

}