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

#if defined(_WIN32) && defined(_DEBUG)
#include <atltrace.h>
#endif

namespace _mp {
		cdev_ctl::~cdev_ctl()
		{
		}
		cdev_ctl::cdev_ctl(clog* p_log) : cworker_ctl(p_log), cdev_ctl_fn(p_log)
		{
		}

		bool cdev_ctl::_execute(cio_packet& request)
		{
			// true �� return (transaction �Ϸ�)��Ÿ���� ���ؼ��� client ���� ���� �����͸� return �ϱ� ����
			// send_data_to_client_by_ip4() �� �����ؾ� ��.
			// false �� return �ϸ�, _continue() �� transaction �� �Ϸ� �ɶ� ���� �ݺ������� ȣ��Ǽ�, ó���ϰ�,
			// _continue() ����  client ���� ���� �����͸� send_data_to_client_by_ip4() �� ����

			type_v_buffer v_rsp;
			cdev_ctl_fn::_cstatus_mgmt::_cresult result(request);

			do {
				if (request.is_response()) {
					result.process_set_rsp_with_error_complete(cio_packet::error_reason_request_type);
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
					result.process_set_rsp_with_error_complete(cio_packet::error_reason_overflow_buffer);
					continue;
				}

				switch (request.get_action())
				{
				case cio_packet::act_dev_open:
				case cio_packet::act_dev_close:
				case cio_packet::act_dev_transmit:
				case cio_packet::act_dev_cancel:
				case cio_packet::act_dev_write:
				case cio_packet::act_dev_read:
					result = m_mgmt_state.process_event(request);
					break;
				case cio_packet::act_dev_sub_bootloader:
					//std::tie(b_complete, p_event_start_worker_for_next_step) = _execute_sub_bootloader(request, response);
					break;
				/*
				case cio_packet::act_dev_independent_bootloader:
					b_complete = _execute_independent_bootloader(request, response);
					break;
				*/
				default:
					result.process_set_rsp_with_error_complete(cio_packet::error_reason_action_code);
					break;
				}//end switch

			} while (false);

			do {
				if (!result.b_process_complete)
					continue;//response is differd
				if (request.is_self())
					continue;//no response need, this request is issued from server-self.
				
				// �񵿱� ��, ���⿡ ���� ��ҵ� ���, ��� �� �� ���� ����.
				_send_all_complete_response();

				//send response complete �� true �Ǿ�� ���� ���Ƿ�, ����� ��ɿ� ���� ���丸 ���⼭ ó����.
				result.ptr_rsp->get_packet_by_json_format(v_rsp);
				cserver::get_instance().send_data_to_client_by_ip4(v_rsp, request.get_session_number());
			} while (false);

			return result.b_process_complete;
		}

		bool cdev_ctl::_continue(cio_packet& request)
		{
			// _execute() �� false �� return �ϸ�, vcworker::_worker() �� ���� ���� ȣ���.
			bool b_complete(false);

			do {
#if defined(_WIN32) && defined(_DEBUG)
				ATLTRACE(L"++ %09u + _continue : complete.\n", request.get_session_number());
#endif
				if (request.is_self()) {
					continue;//no response need, this request is issued from server-self.
				}
				// ����� read request �� ���� ���丸 ó�� �� ����.
				auto r = _send_all_complete_response();
				if (r.first == 0) {
					// ���� ����� ����.
					continue;
				}

				b_complete = true;
				
			} while (false);
			return b_complete;
		}

		std::pair<int, int> cdev_ctl::_send_all_complete_response()
		{
			int n_complete(0);
			int n_sent(0);

			do {
				std::vector<cdev_ctl_fn::type_tuple_full> v_result = m_mgmt_state.read_pop_front_remove_complete_of_all();
				if (v_result.empty()) {
					// ���� ����� ����.
					continue;
				}

				n_complete = (int)v_result.size();
				cio_packet::type_ptr ptr_rsp;

				for (auto item : v_result) {
					ptr_rsp = std::get<3>(item);

					type_v_buffer v_rsp;
					ptr_rsp->get_packet_by_json_format(v_rsp);

					if (!cserver::get_instance().send_data_to_client_by_ip4(v_rsp, ptr_rsp->get_session_number())) {
						//p_obj->p_log->trace( L"_callback_request_result : error send data : session = %u : device_index = %u", n_session, n_device_index);
					}
					else {
						++n_sent;
					}

				}//end for

			} while (false);

			return std::make_pair(n_complete, n_sent);
		}

}//the end of _mp namespace

