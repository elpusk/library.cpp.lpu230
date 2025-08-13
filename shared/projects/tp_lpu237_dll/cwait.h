#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include <list>
#include <condition_variable>
#include <chrono>
#include <algorithm>
#include <iterator>
namespace _tp_lpu237
{
    class cwait
    {
    public:
        typedef std::shared_ptr<cwait> type_ptr;

        enum : int {
            const_event_timeout = -1,
            const_event_min_index = 0,
            const_event_max_index = 2000
        };
    public:
        cwait() : m_n_last_generated_event(cwait::const_event_timeout)
        {
        }

        /**
        * generate new event index number.
        * return event index number const_event_min_index ~ const_event_max_index
        */
        int generate_new_event()
        {
            int n_event(0);
            do {
                std::lock_guard<std::mutex> lock(m_mutex);
                n_event = m_n_last_generated_event;
                ++n_event;
                //
                if (n_event >= cwait::const_event_max_index)
                    continue;
                m_n_last_generated_event = n_event;
                m_v_generated_event.push_back(n_event);
            } while (false);

            return n_event;
        }

        /**
        * trigger event
        */
        bool set(int n_event)
        {
            bool b_result(false);
            do {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (n_event < const_event_min_index)
                    continue;
                if (n_event > m_n_last_generated_event)
                    continue;
                //
                m_list_int_event_index.push_back(n_event);
                m_cv.notify_one();
                b_result = true;
            } while (false);
            return b_result;
        }

        /**
        * cancel trigger
        */
        void reset(int n_event = -1)
        {
            do {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (n_event == -1) {
                    m_list_int_event_index.clear();
                    continue;
                }

                std::list<int>::iterator it = std::find(std::begin(m_list_int_event_index), std::end(m_list_int_event_index), n_event);
                if (it == std::end(m_list_int_event_index))
                    continue;
                m_list_int_event_index.erase(it);
            } while (false);
        }

        /**
        * parameter n_time_out -1: waits inifinite
        * return vector size is zero - time out, this vector contains all trigged events.
        */
        std::vector<int> wait_for_at_once(int n_time_out = -1)
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            std::vector<int> v_signaled_index;
            while (m_list_int_event_index.empty()) {
                if (n_time_out == -1) {
                    m_cv.wait(lock);
                }
                else {
                    if (m_cv.wait_for(lock, std::chrono::milliseconds(n_time_out)) == std::cv_status::timeout) {
                        return v_signaled_index;
                    }
                }
            };

            std::copy(std::begin(m_list_int_event_index), std::end(m_list_int_event_index), std::back_inserter(v_signaled_index));
            m_list_int_event_index.clear();
            return v_signaled_index;
        }
        /**
        * parameter n_time_out -1 : waits inifinite
        * return -1(cwait::const_event_timeout)  - time out.
        */
        int wait_for_one_at_time(int n_time_out = -1)
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            int n_evt(cwait::const_event_timeout);
            while (m_list_int_event_index.empty()) {
                if (n_time_out == -1) {
                    m_cv.wait(lock);
                }
                else {
                    if (m_cv.wait_for(lock, std::chrono::milliseconds(n_time_out)) == std::cv_status::timeout) {
                        return n_evt;
                    }
                }
            };

            n_evt = m_list_int_event_index.front();
            m_list_int_event_index.pop_front();
            return n_evt;
        }

        /**
        * parameter v_event : vector of waiting events.
        * parameter n_time_out -1 : waits inifinite
        * return true : all triggerd, false : time out
        */
        int wait_for_all(const std::vector<int>& v_event = std::vector<int>(0), int n_time_out = -1)
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            bool b_result(false);
            int n_evt(cwait::const_event_timeout);
            std::list<int> list_e;

            if (v_event.empty()) {
                list_e.assign(m_v_generated_event.cbegin(), m_v_generated_event.cend());
            }
            else {
                list_e.assign(v_event.cbegin(), v_event.cend());
            }


            do {
                while (m_list_int_event_index.empty()) {
                    if (n_time_out == -1) {
                        m_cv.wait(lock);
                    }
                    else {
                        if (m_cv.wait_for(lock, std::chrono::milliseconds(n_time_out)) == std::cv_status::timeout) {
                            return b_result;
                        }
                    }
                };

                do {
                    n_evt = m_list_int_event_index.front();
                    std::list<int>::iterator it = std::find(std::begin(list_e), std::end(list_e), n_evt);
                    if (it != std::end(list_e)) {
                        list_e.erase(it);
                    }
                    m_list_int_event_index.pop_front();
                } while (!m_list_int_event_index.empty());

            } while (!list_e.empty());

            b_result = true;
            return b_result;
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::list<int> m_list_int_event_index;
        std::vector< int > m_v_generated_event;
        int m_n_last_generated_event;

    private:
        cwait(const cwait&);
        cwait& operator=(const cwait&);
    };
}