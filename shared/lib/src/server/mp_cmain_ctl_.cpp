#include <websocket/mp_win_nt.h>

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include <mp_type.h>
#include <mp_cqueue.h>
#include <mp_coffee_path.h>

#include <server/mp_cmain_ctl_.h>
#include <server/mp_cserver_.h>

#include <server/mp_cctl_svr_.h>
#include <hid/mp_clibhid.h>
#include <_dev/mp_cdev_util.h>

namespace _mp {

		cmain_ctl::~cmain_ctl()
		{
		}
		cmain_ctl::cmain_ctl(clog* p_log) : cworker_ctl(p_log), cmain_ctl_fn(p_log)
		{
			_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
		}

		cio_packet::type_ptr cmain_ctl::_execute(cio_packet::type_ptr& ptr_req_new, const cio_packet::type_ptr& ptr_req_cur)
		{
			cio_packet::type_ptr ptr_return;
			cio_packet response;
			cio_packet response_for_the_other_session;
			type_v_buffer v_rsp, v_rsp_for_the_other_session;
			bool b_complete(true);
			unsigned long n_owner_session(ptr_req_new->get_session_number());
			type_list_wstring list_wstring_data_field;
			response_for_the_other_session.set_cmd(cio_packet::cmd_invalid);

			do {
				if (ptr_req_new->is_response()) {
					b_complete = true;
					response = *_generate_error_response(*ptr_req_new, cio_packet::error_reason_request_type);
					continue;
				}

				size_t n_data(ptr_req_new->get_data_field_size());
				if (n_data > cws_server::const_default_max_rx_size_bytes) {
					if (ptr_req_new->get_data_field_type() == cio_packet::data_field_string_utf8) {
						m_p_log->log_fmt(L"[E] - %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
						m_p_log->trace(L"[E] - %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					else {
						m_p_log->log_fmt(L"[E] - %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
						m_p_log->trace(L"[E] - %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					b_complete = true;
					response = *_generate_error_response(*ptr_req_new, cio_packet::error_reason_overflow_buffer);
					continue;
				}
				switch (ptr_req_new->get_action())
				{
				case cio_packet::act_mgmt_get_echo:
					b_complete = _execute_mgmt_get_echo(m_p_log, *ptr_req_new, response);
					break;
				case cio_packet::act_mgmt_get_device_list:
					ptr_req_new->get_data_field(list_wstring_data_field);
					b_complete = _execute_mgmt_get_device_list(m_p_log, list_wstring_data_field, *ptr_req_new, response);
					break;
				case cio_packet::act_mgmt_ctl_show:
					b_complete = _execute_mgmt_ctl_show(m_p_log, *ptr_req_new, response);
					break;
				case cio_packet::act_mgmt_file_operation:
					b_complete = _execute_file_operation(m_p_log, *ptr_req_new, response);
					break;
				case cio_packet::act_mgmt_advance_operation:
					b_complete = _execute_advance_operation(m_p_log, *ptr_req_new, response, response_for_the_other_session);
					break;
				default:
					b_complete = true;
					response = *_generate_error_response(*ptr_req_new, cio_packet::error_reason_action_code);
					break;
				}//end switch

			} while (false);

			do {
				if (!b_complete)
					continue;//response is differd
				if (ptr_req_new->is_self())
					continue;//no response need, this request is issued from server-self.
				//send response
				response.get_packet_by_json_format(v_rsp);
				cserver::get_instance().send_data_to_client_by_ip4(v_rsp, ptr_req_new->get_session_number());

				if (response_for_the_other_session.is_valid()) {
					response_for_the_other_session.get_packet_by_json_format(v_rsp_for_the_other_session);

					if (response_for_the_other_session.get_session_number() == _MP_TOOLS_INVALID_SESSION_NUMBER)
						cserver::get_instance().broadcast_by_ip4(n_owner_session, v_rsp_for_the_other_session);//boardcast
					else
						cserver::get_instance().send_data_to_client_by_ip4(v_rsp_for_the_other_session, response_for_the_other_session.get_session_number());
				}
			} while (false);

			if (!b_complete) {
				ptr_return = ptr_req_new;
			}
			return ptr_return;
		}

}//the end of _mp namespace

