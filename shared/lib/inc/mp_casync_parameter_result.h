#pragma once
#include <memory>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>

#ifdef _WIN32
#include <Windows.h>
#endif 
#include <mp_type.h>
#include <mp_cwait.h>

namespace _mp
{
	class casync_parameter_result
	{
	public:
		typedef	std::shared_ptr< casync_parameter_result>	type_ptr_ct_async_parameter_result;
		typedef	void(__stdcall* type_callback)(void*);

	public:
		casync_parameter_result() : 
			m_p_fun(nullptr)
			, m_p_para(nullptr)
			, m_h_wnd(NULL)
			, m_n_msg(0)
			, m_n_result_code(0)
			, m_b_success(false)
		{
			m_n_evt_kill = m_event_kill_and_wait.generate_new_event();
			m_n_evt_wait = m_event_kill_and_wait.generate_new_event();
		}
		casync_parameter_result(
			casync_parameter_result::type_callback p_fun
			, void* p_para
			, HWND h_wnd
			, UINT n_msg
		) : 
			m_p_fun(p_fun)
			, m_p_para(p_para)
			, m_h_wnd(h_wnd)
			, m_n_msg(n_msg)
			, m_n_result_code(0)
			, m_b_success(false)
		{
			m_n_evt_kill = m_event_kill_and_wait.generate_new_event();
			m_n_evt_wait = m_event_kill_and_wait.generate_new_event();
		}

		virtual ~casync_parameter_result()
		{
			m_event_kill_and_wait.set(m_n_evt_kill);

			if (m_ptr_thread_for_cb) {
				m_ptr_thread_for_cb->detach();
				if (m_ptr_thread_for_cb->joinable())
					m_ptr_thread_for_cb->join();
				//
				m_ptr_thread_for_cb.reset();
			}
		}

		bool waits(unsigned long dw_wait_time = _MP_TIMEOUT)
		{
			bool b_result(false);

			do {
				std::vector<int> v_evt_index = m_event_kill_and_wait.wait_for_at_once((int)dw_wait_time);
				if (v_evt_index.empty()) {
					continue;
				}
				b_result = true;
			} while (false);
			return b_result;
		}

		void set_parameter(
			casync_parameter_result::type_callback p_fun
			, void* p_para
			, HWND h_wnd
			, UINT n_msg
		)
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result);
			m_p_fun = p_fun;
			m_p_para = p_para;
			m_h_wnd = h_wnd;
			m_n_msg = n_msg;
		}

		void set_result(
			bool b_success
			, unsigned long n_result_code
			, const type_v_buffer& v_binary_result
		)
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result);
			m_b_success = b_success;
			m_v_binary_result = v_binary_result;
			m_n_result_code = n_result_code;
		}

		void set_result(bool b_success, const std::wstring& s_result)
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result);
			m_b_success = b_success;
			m_s_result = s_result;
		}
		void set_result(bool b_success, const type_set_wstring& set_wstring_result)
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result);
			m_b_success = b_success;
			m_set_wstring_result = set_wstring_result;
		}
		void set_result(bool b_success, const type_list_wstring& list_wstring_result)
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result);
			m_b_success = b_success;
			m_list_wstring_result = list_wstring_result;
		}
		void set_result(bool b_success, unsigned long n_result_code)
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result);
			m_b_success = b_success;
			m_n_result_code = n_result_code;
		}

		bool get_result()
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result);
			return m_b_success;
		}
		bool get_result(type_v_buffer& v_out_result)
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result);
			v_out_result = m_v_binary_result;
			return m_b_success;
		}
		bool get_result(std::wstring& s_result)
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result);
			s_result = m_s_result;
			return m_b_success;
		}
		bool get_result(type_set_wstring& set_wstring_result)
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result);
			set_wstring_result = m_set_wstring_result;
			return m_b_success;
		}
		bool get_result(type_list_wstring& list_wstring_result)
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result);
			list_wstring_result = m_list_wstring_result;
			return m_b_success;
		}
		bool get_result(unsigned long& n_result_code)
		{
			std::lock_guard<std::mutex> lock(m_mutex_for_result);
			n_result_code = m_n_result_code;
			return m_b_success;
		}

		void notify()
		{
			do {
				if (_run_callback()) {
					m_event_kill_and_wait.set(m_n_evt_wait);
					continue;
				}
				if (_post_message()) {
					m_event_kill_and_wait.set(m_n_evt_wait);
					continue;
				}
				m_event_kill_and_wait.set(m_n_evt_wait);
			} while (false);
		}

	private:
		bool _run_callback()
		{
			if (m_p_fun) {
				if (m_ptr_thread_for_cb) {
					if (m_ptr_thread_for_cb->joinable())
						m_ptr_thread_for_cb->join();
					m_ptr_thread_for_cb.reset();
				}

				m_ptr_thread_for_cb = std::make_shared<std::thread>(m_p_fun, m_p_para);
				if (!m_ptr_thread_for_cb)
					return false;
				//m_p_fun(m_p_para);
				return true;
			}
			else
				return false;
		}

		bool _post_message()
		{
#ifdef _WIN32
			if (m_h_wnd) {
				PostMessage(m_h_wnd, m_n_msg, 0, 0);
				return true;
			}
			else {
				return false;
			}
#else
			return false;//not support the other OS.
#endif
		}


	private:
		std::mutex m_mutex_for_result;

		//parameters
		cwait m_event_kill_and_wait;
		int m_n_evt_kill;
		int m_n_evt_wait;

		casync_parameter_result::type_callback m_p_fun;
		void* m_p_para;
		HWND m_h_wnd;
		UINT m_n_msg;
		std::shared_ptr<std::thread> m_ptr_thread_for_cb;

		//results
		unsigned long m_n_result_code;
		type_v_buffer m_v_binary_result;
		bool m_b_success;
		std::wstring m_s_result;
		type_set_wstring m_set_wstring_result;
		type_list_wstring m_list_wstring_result;

	private://don't call these methods
		casync_parameter_result(const casync_parameter_result&);
		casync_parameter_result& operator=(const casync_parameter_result&);

	};

}//the end of _mp