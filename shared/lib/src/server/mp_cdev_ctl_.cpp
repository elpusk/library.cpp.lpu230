#include <websocket/mp_win_nt.h>

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <cassert>

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
			cio_packet::type_ptr ptr_req_ing;
			type_v_buffer v_rsp;
			cbase_ctl_fn::cresult::type_ptr ptr_result_error, ptr_result;
			cio_packet::type_ptr ptr_response;
			cdev_ctl_fn::cdev_ctl_fn::type_ptr_v_pair_ptr_req_ptr_rsp ptr_v_req_rsp;

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
					ptr_result = m_fun.process_event(ptr_req_new, ptr_req_cur);
					assert(ptr_result); // process_event() �� ���� ����.

					std::tie(std::ignore, b_complete) = ptr_result->process_get_result();
					if (!b_complete) {
						ptr_req_ing = ptr_result->get_req();// ��� ���� ���� req �� return ���� ����.
						break;
					}
					
					// ó���� ���� ���.
					ptr_response = ptr_result->get_rsp();
					if (!ptr_response) {
						ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_req_new);
						ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_misformat);
						continue;
					}
					break;
				case cio_packet::act_dev_sub_bootloader:
				case cio_packet::act_dev_independent_bootloader:
				default:// ����� �������� �����Ƿ� �׳� ���� ó��.
					ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_req_new);
					ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_action_code);
					continue;
				}//end switch

			} while (false);

			// ptr_result_error �� �Ҵ�Ǿ� ������ ���� ����.
			if (ptr_result_error) {
				// ó�� �� ����� ����
				if (!ptr_result_error->get_req()->is_self()) {
					// ������ ����
					ptr_result_error->get_rx(v_rsp);
					cserver::get_instance().send_data_to_client_by_ip4(v_rsp, ptr_req_new->get_session_number());
				}
				if (ptr_req_cur) {
					// � ���� �߿� ���ο� ���� �� ���¸�, ���ο� �� ���� ���� ���� �̹Ƿ�
					// ���� �ϴ� �� ��� ����
					ptr_req_ing = ptr_req_cur;
				}
			}
			else {
				// ó�� �� ���� ����. 
				if (!ptr_req_ing) {
					// ���Ӱ� ó�� ���� ���� ������(���ο� req ó�� �Ϸ�.)
					assert(ptr_response); // ptr_response ���� ������ ������ ptr_result_error �� �־��.

					// open close complete ���, q �� ���Ե��� �ʾ�, 
					// get_all_complete_response() �� ptr_response �� ���Ե��� �ʴ� ���, �����ϵ���
					// get_all_complete_response() �� ���ڷ� ptr_req_new, ptr_response pair �� �ش�.
					ptr_v_req_rsp = m_fun.get_all_complete_response(std::make_pair(ptr_req_new, ptr_response));//complete �� ��� req, rsp pair �� ���
					
					// ���� �ȉ�.
					if (!ptr_v_req_rsp) {
						//complete �� �� ���� ���
						ptr_req_ing = ptr_req_cur; // ���� ó�� ���� ���� ������. �� ���� ��� ó��. ������ ���� ���� return.
					}
					else {
						// complete �� ���� ���� �ϸ�, ������ ������ �����µ� .......
						// �������� ptr_req_ing �� null �� ����.
						for (auto item : *ptr_v_req_rsp) {
							if (item.first.get() == ptr_req_cur.get()) {
								// ���� ���� ���̴� req�� complete �Ǿ reference decrease �ǰ�, 
								// �Ʒ����� server �� ����.
								ptr_req_cur.reset(); 
							}
							else if (item.first.get() == ptr_req_new.get()) {
								// ���ο� req ó�� �Ϸ�.
								// get_all_complete_response() �� return vector item ������.
								// ���� ���� ���̴� ptr_req_cur �� complete �Ǹ� , �� ���� ���� ����
								// �� �ڵ忡�� �׻� ptr_req_cur.reset() ��.
								// �� �� ó�� ��, ptr_req_cur.reset() �� ������� ����.
								ptr_req_ing = ptr_req_cur; // ��� ���� �� req�� ����.
							}

							if (item.first->is_self()) {
								continue;
							}

							type_v_buffer v_rsp;
							item.second->get_packet_by_json_format(v_rsp);
							cserver::get_instance().send_data_to_client_by_ip4(v_rsp, item.second->get_session_number());
						}//end for

						if (!ptr_req_ing) {
							// ���� ��� �����ؾ��ϴ� req �� ������, read q ���� complete ���� ���� req�� ã�Ƽ�
							// ���ؾ��ϴ� req�� ����.
							auto v_ptr_req = m_fun.get_front_of_read_queue();
							if (!v_ptr_req.empty()) {
								ptr_req_ing = v_ptr_req[0];
							}
						}
					}
				}
				else {
					// ���Ӱ� ó�� ���� ���� ������, ���� ó�� ���� ���� �ִ� �Ͱ� �����ϰ�, 
					// ���ο� �� ��� ó��.
				}
			}

			return ptr_req_ing;
		}

		bool cdev_ctl::_continue(cio_packet::type_ptr& ptr_req_cur)
		{
			// _execute() �� return �� nullptr �� �ƴϸ�, vcworker::_worker() �� ���� ���� ȣ���.
			bool b_complete(false);
			do {
				// ����� read request �� ���� ���丸 ó�� �� ����.
				cdev_ctl_fn::cdev_ctl_fn::type_ptr_v_pair_ptr_req_ptr_rsp ptr_v_req_rsp = m_fun.get_all_complete_response();
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

