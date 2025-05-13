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

		cio_packet::type_ptr cdev_ctl::_execute(cio_packet::type_ptr& ptr_req_new, cio_packet::type_ptr& ptr_req_cur)
		{
			// true �� return (transaction �Ϸ�)��Ÿ���� ���ؼ��� client ���� ���� �����͸� return �ϱ� ����
			// send_data_to_client_by_ip4() �� �����ؾ� ��.
			// false �� return �ϸ�, _continue() �� transaction �� �Ϸ� �ɶ� ���� �ݺ������� ȣ��Ǽ�, ó���ϰ�,
			// _continue() ����  client ���� ���� �����͸� send_data_to_client_by_ip4() �� ����

			bool b_complete(true);
			cio_packet::type_ptr ptr_return;
			type_v_buffer v_rsp;
			cdev_ctl_fn::type_result_event result_from_fn;
			cbase_ctl_fn::cresult::type_ptr ptr_result_error;
			cio_packet::type_ptr ptr_response;

			do {
				if (ptr_req_new->is_response()) {
					ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_req_new);
					ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_request_type);
					continue;
				}
				size_t n_data(ptr_req_new->get_data_field_size());
				if (n_data > cws_server::const_default_max_rx_size_bytes) {
					if (ptr_req_new->get_data_field_type() == cio_packet::data_field_string_utf8) {
						m_p_log->get_instance().log_fmt(L"[E] - %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
						m_p_log->get_instance().trace(L"[E] - %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					else {
						m_p_log->log_fmt(L"[E] - %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
						m_p_log->trace(L"[E] - %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_req_new);
					ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_overflow_buffer);
					continue;
				}

				switch (ptr_req_new->get_action())
				{
				case cio_packet::act_dev_open:
				case cio_packet::act_dev_close:
				case cio_packet::act_dev_transmit:
				case cio_packet::act_dev_cancel:
				case cio_packet::act_dev_write:
				case cio_packet::act_dev_read:
					result_from_fn = m_fun.process_event(ptr_req_new);
					b_complete = std::get<1>(result_from_fn);
					if (!b_complete) {
						break;
					}
					
					// ����� ����� �Ǵ� �񵿱���� ���ۿ����� ��.
					if (std::get<2>(result_from_fn)) {
						// ó�� ����� success �̰ų� error. 
						if (_continue(ptr_req_new)) {
							break;
						}
						//���� ptr_req_new �� ���� ptr_rsp �� ó������ �ʾ�����,
						ptr_response = std::get<2>(result_from_fn);
						if (!ptr_response) {
							break;
						}
						if (ptr_response->is_self()) {
							break;
						}
						ptr_response->get_packet_by_json_format(v_rsp);
						cserver::get_instance().send_data_to_client_by_ip4(v_rsp, ptr_req_new->get_session_number());
					}
					else {
						ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_req_new);
						ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_misformat);
					}

					break;
				case cio_packet::act_dev_sub_bootloader:
				case cio_packet::act_dev_independent_bootloader:
				default:// ����� �������� �����Ƿ� �׳� ���� ó��.
					ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_req_new);
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
				cserver::get_instance().send_data_to_client_by_ip4(v_rsp, ptr_req_new->get_session_number());
				b_complete = true;
			} while (false);

			if (!b_complete) {
				ptr_return = ptr_req_new;
			}
			return ptr_return;
		}

		bool cdev_ctl::_continue(cio_packet::type_ptr& ptr_req_cur)
		{
			// _execute() �� return �� nullptr �� �ƴϸ�, vcworker::_worker() �� ���� ���� ȣ���.
			bool b_complete(false);
			do {
				// ����� read request �� ���� ���丸 ó�� �� ����.
				auto ptr_v_req_rsp = m_fun.get_all_complete_response();
				if (!ptr_v_req_rsp) {
					// ���� ����� ����.
					continue;
				}

				for (auto item : *ptr_v_req_rsp) {
					if (item.first.get() == ptr_req_cur.get()) {
						// complete �� �� ��. ���� ���� ���� request �� ������ contine �� cmplete �� �����ؼ�,
						// ���� ���μ������� ���� request �� ���� �޸� ������ ��û��.
						b_complete = true; 
					}

					if (item.first->is_self()) {
						continue;
					}

					type_v_buffer v_rsp;
					item.second->get_packet_by_json_format(v_rsp);
					cserver::get_instance().send_data_to_client_by_ip4(v_rsp, item.second->get_session_number());
				}//end for
				
			} while (false);
			return b_complete;
		}

}//the end of _mp namespace

