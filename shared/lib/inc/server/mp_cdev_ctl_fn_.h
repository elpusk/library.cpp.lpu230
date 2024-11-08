#pragma once

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <deque>
#include <string>

#include <mp_type.h>
#include <mp_cqueue.h>
#include <mp_clog.h>
#include <server/mp_cio_packet.h>
#include <hid/mp_clibhid_dev.h>

namespace _mp {
	class cdev_ctl_fn
	{
	public:
		typedef	std::weak_ptr< cdev_ctl_fn >		type_wptr;
		typedef	std::shared_ptr< cdev_ctl_fn >	type_ptr;

	public:
		virtual ~cdev_ctl_fn();
	protected:
		cdev_ctl_fn();

		type_pair_bool_result_bool_complete _execute_open_sync(clog *p_log, const cio_packet& request, cio_packet& response);
		type_pair_bool_result_bool_complete _execute_close_sync(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring & s_dev_path
		);
		type_pair_bool_result_bool_complete _execute_close(clog* p_log, const cio_packet& request, cio_packet& response);

		type_pair_bool_result_bool_complete _execute_read(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
			);
		type_pair_bool_result_bool_complete _execute_write(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		);
		type_pair_bool_result_bool_complete _execute_transmit(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		);
		type_pair_bool_result_bool_complete _execute_cancel(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		);

		type_pair_bool_result_bool_complete _execute_aync(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		);
		
		bool _execute_general_error_response(clog* p_log, cio_packet& request, cio_packet& response, cio_packet::type_error_reason n_reason = cio_packet::error_reason_none);
		/**
		* for pushing request, async response
		*/
		void _push_back_request_for_response(const cio_packet& req)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_q_ptr_cur_req.push_back(std::make_shared<cio_packet>(req));
		}

		bool _pop_front_cur_reqeust_for_response_and_remve(cio_packet& out)
		{
			bool b_result(false);

			do {
				std::lock_guard<std::mutex> lock(m_mutex);
				if (m_q_ptr_cur_req.empty()) {
					continue;
				}

				out = *m_q_ptr_cur_req.front();
				m_q_ptr_cur_req.pop_front();

				b_result = true;
			} while (false);
			return b_result;
		}
	protected://callback of dev control

		/**
		* callback of clibhid_dev.start_read, start_write, start_write_read or start_cancel
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		static bool _cb_dev_io(cqitem_dev& qi, void*p_user);

	protected:
		int m_n_cnt_open;//open counter

		std::mutex m_mutex;
		std::deque<cio_packet::type_ptr> m_q_ptr_cur_req;

	private://don't call these methods
		cdev_ctl_fn(const cdev_ctl_fn&);
		cdev_ctl_fn& operator=(const cdev_ctl_fn&);

	};

}//the end of _mp namespace

