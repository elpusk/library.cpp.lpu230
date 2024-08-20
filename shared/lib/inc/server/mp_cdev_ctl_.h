#pragma once

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <deque>

#include <mp_type.h>
#include <mp_cqueue.h>
#include <server/mp_cio_packet.h>
#include <server/mp_cworker_ctl.h>

#include <hid/mp_clibhid_dev.h>

namespace _mp {
	class cdev_ctl : public cworker_ctl
	{
	public:
		typedef	std::weak_ptr< cdev_ctl >		type_wptr;
		typedef	std::shared_ptr< cdev_ctl >	type_ptr;

	public:
		virtual ~cdev_ctl();
		cdev_ctl(clog* p_log);

	protected:

		/**
		* executed by worker thread.
		* processing request.
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete
		*/
		virtual bool _execute(cio_packet& request);

	private:
		type_pair_bool_result_bool_complete _execute_open_sync(const cio_packet& request, cio_packet& response);
		type_pair_bool_result_bool_complete _execute_close_sync(const cio_packet& request, cio_packet& response);
		type_pair_bool_result_bool_complete _execute_close(const cio_packet& request, cio_packet& response);

		type_pair_bool_result_bool_complete _execute_read(const cio_packet& request, cio_packet& response);
		type_pair_bool_result_bool_complete _execute_write(const cio_packet& request, cio_packet& response);
		type_pair_bool_result_bool_complete _execute_transmit(const cio_packet& request, cio_packet& response);
		type_pair_bool_result_bool_complete _execute_cancel(const cio_packet& request, cio_packet& response);

		type_pair_bool_result_bool_complete _execute_aync(const cio_packet& request, cio_packet& response);
		
		bool _execute_general_error_response(cio_packet& request, cio_packet& response, cio_packet::type_error_reason n_reason = cio_packet::error_reason_none);

		type_pair_bool_result_bool_complete _execute_kernel(const cio_packet& request, cio_packet& response);
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
	private://callback of dev control

		/**
		* callback of clibhid_dev.start_read, start_write, start_write_read or start_cancel
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		static bool _cb_dev_io(cqitem_dev& qi, void*p_user);

	private:
		int m_n_cnt_open;//open counter

		std::mutex m_mutex;
		std::deque<cio_packet::type_ptr> m_q_ptr_cur_req;

	private://don't call these methods
		cdev_ctl();
		cdev_ctl(const cdev_ctl&);
		cdev_ctl& operator=(const cdev_ctl&);

	};

}//the end of _mp namespace

