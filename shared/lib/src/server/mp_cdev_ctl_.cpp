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
		cdev_ctl::cdev_ctl(clog* p_log) : cworker_ctl(p_log), m_fun(p_log)
		{
		}

		bool cdev_ctl::_execute(cio_packet::type_ptr& ptr_request)
		{
			// true 로 return (transaction 완료)나타내기 위해서는 client 에세 보낼 데이터를 return 하기 전에
			// send_data_to_client_by_ip4() 로 전송해야 함.
			// false 로 return 하면, _continue() 이 transaction 이 완료 될때 까지 반복적으로 호출되서, 처리하고,
			// _continue() 에서  client 에세 보낼 데이터를 send_data_to_client_by_ip4() 로 전송

			bool b_complete(true);
			type_v_buffer v_rsp;
			type_pair_bool_result_bool_complete result_from_fn;
			cbase_ctl_fn::cresult::type_ptr ptr_result_error;

			do {
				if (ptr_request->is_response()) {
					ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_request);
					ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_request_type);
					continue;
				}
				size_t n_data(ptr_request->get_data_field_size());
				if (n_data > cws_server::const_default_max_rx_size_bytes) {
					if (ptr_request->get_data_field_type() == cio_packet::data_field_string_utf8) {
						m_p_log->get_instance().log_fmt(L"[E] - %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
						m_p_log->get_instance().trace(L"[E] - %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					else {
						m_p_log->log_fmt(L"[E] - %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
						m_p_log->trace(L"[E] - %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_request);
					ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_overflow_buffer);
					continue;
				}

				switch (ptr_request->get_action())
				{
				case cio_packet::act_dev_open:
				case cio_packet::act_dev_close:
				case cio_packet::act_dev_transmit:
				case cio_packet::act_dev_cancel:
				case cio_packet::act_dev_write:
				case cio_packet::act_dev_read:
					result_from_fn = m_fun.process_event(ptr_request);
					b_complete = result_from_fn.second;
					if (!b_complete) {
						break;
					}
					if (result_from_fn.first) {
						_continue(ptr_request);
					}
					else {
						ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_request);
						ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_misformat);
					}

					break;
				case cio_packet::act_dev_sub_bootloader:
				case cio_packet::act_dev_independent_bootloader:
				default:
					ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_request);
					ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_action_code);
					break;
				}//end switch

			} while (false);

			do {
				if (!ptr_result_error) {
					continue;
				}
				//send error in this layer
				if (ptr_result_error->get_req()->is_self()) {
					continue;
				}
				ptr_result_error->get_rx(v_rsp);
				cserver::get_instance().send_data_to_client_by_ip4(v_rsp, ptr_request->get_session_number());
				b_complete = true;
			} while (false);

			return b_complete;
		}

		bool cdev_ctl::_continue(cio_packet::type_ptr& ptr_request)
		{
			// _execute() 가 false 를 return 하면, vcworker::_worker() 에 의해 여기 호출됨.
			bool b_complete(false);
			do {
				// 여기는 read request 에 대한 응답만 처리 될 것임.
				auto ptr_v_r = m_fun.get_all_complete_response();
				if (!ptr_v_r) {
					// 아직 결과가 없음.
					continue;
				}
				
				b_complete = true;

				for (auto item : *ptr_v_r) {
					if (item->is_self()) {
						continue;
					}

					type_v_buffer v_rsp;
					item->get_packet_by_json_format(v_rsp);
					cserver::get_instance().send_data_to_client_by_ip4(v_rsp, item->get_session_number());
				}
				
			} while (false);
			return b_complete;
		}

}//the end of _mp namespace

