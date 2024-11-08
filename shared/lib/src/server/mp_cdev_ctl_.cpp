#include <websocket/mp_win_nt.h>

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include <mp_type.h>
#include <mp_cqueue.h>

#include <server/mp_cserver_.h>
#include <server/mp_cdev_ctl_.h>

#include <hid/mp_clibhid.h>

namespace _mp {
		cdev_ctl::~cdev_ctl()
		{
		}
		cdev_ctl::cdev_ctl(clog* p_log) : cworker_ctl(p_log), cdev_ctl_fn()
		{
		}


		/**
		* executed by worker thread.
		* processing request.
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete
		*/
		bool cdev_ctl::_execute(cio_packet& request)
		{
			cio_packet response;
			type_v_buffer v_rsp;
			bool b_completet(true);
			bool b_result(false);
			//_ns_tools::ct_warp::cevent* p_event_start_worker_for_next_step(nullptr);

			do {
				if (request.is_response()) {
					b_completet = _execute_general_error_response(m_p_log, request, response, cio_packet::error_reason_request_type);
					continue;
				}
				size_t n_data(request.get_data_field_size());
				if (n_data > cws_server::const_default_max_rx_size_bytes) {
					if (request.get_data_field_type() == cio_packet::data_field_string_utf8) {
						m_p_log->get_instance().log_fmt(L"[E] - %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
						m_p_log->get_instance().trace(L"[E] - %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					else {
						m_p_log->log_fmt(L"[E] - %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
						m_p_log->trace(L"[E] - %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					b_completet = _execute_general_error_response(m_p_log, request, response, cio_packet::error_reason_overflow_buffer);
					continue;
				}

				switch (request.get_action())
				{
				case cio_packet::act_dev_open:
					std::tie(b_result,b_completet) = _execute_open_sync(m_p_log, request, response);
					if (b_result) {
						set_connected_session(request.get_session_number());
					}
					break;
				case cio_packet::act_dev_close:
					b_completet = _execute_close(m_p_log, request, response).second;
					break;
				case cio_packet::act_dev_transmit:
					b_completet = _execute_transmit(m_p_log, request, response, get_connected_session(), get_dev_path()).second;
					break;
				case cio_packet::act_dev_cancel:
					b_completet = _execute_cancel(m_p_log, request, response, get_connected_session(), get_dev_path()).second;
					break;
				case cio_packet::act_dev_write:
					b_completet = _execute_write(m_p_log, request, response, get_connected_session(), get_dev_path()).second;
					break;
				case cio_packet::act_dev_read:
					b_completet = _execute_read(m_p_log, request, response, get_connected_session(), get_dev_path()).second;
					break;
				/*
				case cio_packet::act_dev_sub_bootloader:
					std::tie(b_completet, p_event_start_worker_for_next_step) = _execute_sub_bootloader(request, response);
					break;
				case cio_packet::act_dev_independent_bootloader:
					b_completet = _execute_independent_bootloader(request, response);
					break;
				*/
				default:
					b_completet = _execute_general_error_response(m_p_log, request, response, cio_packet::error_reason_action_code);
					break;
				}//end switch

			} while (false);

			do {
				if (!b_completet)
					continue;//response is defferd
				if (request.is_self())
					continue;//no response need, this request is issued from server-self.
				//send response
				response.get_packet_by_json_format(v_rsp);
				cserver::get_instance().send_data_to_client_by_ip4(v_rsp, request.get_session_number());
				/*
				if (p_event_start_worker_for_next_step) {
					p_event_start_worker_for_next_step->set();
				}
				*/
			} while (false);

			return b_completet;
		}

}//the end of _mp namespace

