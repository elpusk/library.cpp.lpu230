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

#include <server/mp_cserver_.h>
#include <server/mp_ckernel_ctl_.h>

#include <hid/mp_clibhid.h>
#include <_dev/mp_cdev_util.h>

namespace _mp
{
	ckernel_ctl::ckernel_ctl(clog* p_log) : cworker_ctl(p_log), cmain_ctl_fn(), cdev_ctl_fn()
	{
	}
	ckernel_ctl::~ckernel_ctl()
	{
	}

	/**
	* executed by worker thread.
	* processing request.
	* @paramter request request reference
	* @return true -> complete(with error or success), false -> not complete
	*/
	bool ckernel_ctl::_execute(cio_packet& request)
	{
		cio_packet response;
		cio_packet response_for_the_other_session;
		type_v_buffer v_rsp, v_rsp_for_the_other_session;
		bool b_completet(true);
		unsigned long n_owner_session(request.get_session_number());
		type_list_wstring list_wstring_data_field;
		response_for_the_other_session.set_cmd(cio_packet::cmd_invalid);

		do {
			if (request.is_response()) {
				b_completet = _execute_general_error_response(request, response, cio_packet::error_reason_request_type);
				continue;
			}

			size_t n_data(request.get_data_field_size());
			if (n_data > cws_server::const_default_max_rx_size_bytes) {
				if (request.get_data_field_type() == cio_packet::data_field_string_utf8) {
					m_p_log->log_fmt(L"[E] - %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					m_p_log->trace(L"[E] - %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
				}
				else {
					m_p_log->log_fmt(L"[E] - %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					m_p_log->trace(L"[E] - %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
				}
				b_completet = _execute_general_error_response(request, response, cio_packet::error_reason_overflow_buffer);
				continue;
			}
			switch (request.get_action())
			{
			case cio_packet::act_mgmt_dev_kernel_operation:
				b_completet = _execute_kernel_operation(request, response);
				break;
			default:
				b_completet = _execute_general_error_response(request, response, cio_packet::error_reason_action_code);
				break;
			}//end switch

		} while (false);

		do {
			if (!b_completet)
				continue;//response is differd
			if (request.is_self())
				continue;//no response need, this request is issued from server-self.
			//send response
			response.get_packet_by_json_format(v_rsp);
			cserver::get_instance().send_data_to_client_by_ip4(v_rsp, request.get_session_number());

			if (response_for_the_other_session.is_valid()) {
				response_for_the_other_session.get_packet_by_json_format(v_rsp_for_the_other_session);

				if (response_for_the_other_session.get_session_number() == _MP_TOOLS_INVALID_SESSION_NUMBER)
					cserver::get_instance().broadcast_by_ip4(n_owner_session, v_rsp_for_the_other_session);//boardcast
				else
					cserver::get_instance().send_data_to_client_by_ip4(v_rsp_for_the_other_session, response_for_the_other_session.get_session_number());
			}
		} while (false);

		return b_completet;
	}

	/**
	* executed by worker thread. when _execute return false(not complete),and none new request
	* @paramter request request reference
	* @return true -> complete(with error or success), false -> not complete(_continue() will be recalled at next time)
	*/
	bool ckernel_ctl::_continue(cio_packet& request)
	{
		return true;
	}

	bool ckernel_ctl::_execute_general_error_response(cio_packet& request, cio_packet& response, cio_packet::type_error_reason n_reason /*= cio_packet::error_reason_none*/)
	{
		response = request;
		response.set_cmd(cio_packet::cmd_response);
		if (n_reason == cio_packet::error_reason_none)
			response.set_data_error();
		else
			response.set_data_error(cio_packet::get_error_message(n_reason));
		//
		return true;//complete with error
	}

	bool ckernel_ctl::_execute_kernel_operation(cio_packet& request, cio_packet& response)
	{
		bool b_complete(true);
		bool b_result(false);
		type_list_wstring list_wstring_data_field;
		unsigned long n_session = request.get_session_number();
		cws_server::csession::type_ptr_session ptr_session;

		do {
			response = request;
			response.set_cmd(cio_packet::cmd_response);
			response.set_data_error();//1'st strng

			ptr_session = cserver::get_instance().get_session(n_session);
			if (!ptr_session)
				continue;
			if (request.get_data_field_type() != cio_packet::data_field_string_utf8)
				continue;//not supported format.
			if (request.get_data_field(list_wstring_data_field) == 0)
				continue;
			// packet accept OK.
			if (list_wstring_data_field.empty())
				continue;//not condition
			std::wstring s_req = list_wstring_data_field.front();

			type_list_wstring list_token;
			if (cconvert::tokenizer(list_token, s_req, L" ") < 2)
				continue;//not condition
			std::wstring s_action = list_token.front();	//load, unload, execute, cancel, list
			list_token.pop_front();
			std::wstring s_type = list_token.front(); //service, device
			list_token.pop_front();
			if (s_type.compare(L"device") == 0 && s_action.compare(L"list") == 0) {
				list_wstring_data_field.pop_front();//remove the first string
				b_complete = _execute_mgmt_get_device_list(m_p_log,list_wstring_data_field, request, response);
				continue;
			}

			// each kernel request is executed. 
			// TODO. NOt yet supported
			type_list_wstring list_result;
			list_result.push_back(L"error");
			b_result = false;
			b_complete = true;

			if (b_complete && list_result.empty()) {
				continue;
			}
			response.set_data_by_utf8(list_result);//1'st strng( may be "success" or "error")

		} while (false);

		return b_complete;//complete
	}

	bool ckernel_ctl::_execute_kernel(const cio_packet& request, cio_packet& response)
	{
		type_pair_bool_result_bool_complete pair_bool_result_bool_complete(true, true);
		bool b_complete(true);
		type_list_wstring list_wstring_data_field;
		unsigned long n_session = request.get_session_number();
		cws_server::csession::type_ptr_session ptr_session;

		bool b_result(false);

		do {
			response = request;
			response.set_cmd(cio_packet::cmd_response);
			response.set_data_error();//1'st strng

			if (request.get_data_field_type() != cio_packet::data_field_string_utf8)
				continue;//not supported format.
			if (request.get_data_field(list_wstring_data_field) == 0)
				continue;
			// packet accept OK.
			std::wstring s_req = list_wstring_data_field.front();
			type_list_wstring list_token;
			if (cconvert::tokenizer(list_token, s_req, L" ") == 0)
				continue;//not condition

			std::wstring s_action = list_token.front();	//load, unload, execute, cancel, open ,close
			list_token.pop_front();
			std::wstring s_type = list_token.front(); //service, device
			list_token.pop_front();
			//
			if (s_type.compare(L"device") == 0) {
				if (s_action.compare(L"open") == 0) {
					pair_bool_result_bool_complete = _execute_open_sync(m_p_log, request, response);
					b_result = pair_bool_result_bool_complete.first;
					b_complete = pair_bool_result_bool_complete.second;
					response.set_kernel_device_open(true);
					continue;
				}
				if (s_action.compare(L"close") == 0) {
					pair_bool_result_bool_complete = _execute_close_sync(m_p_log, request, response, get_dev_path());
					b_result = pair_bool_result_bool_complete.first;
					b_complete = pair_bool_result_bool_complete.second;
					response.set_kernel_device_close(true);
					continue;
				}
			}

			ptr_session = cserver::get_instance().get_session(n_session);
			if (!ptr_session)
				continue;

			if (s_type.compare(L"service") == 0) {
				if (s_action.compare(L"execute") != 0 && s_action.compare(L"cancel") != 0) {
					continue;
				}

				//TODO. not yet supported
				continue;
			}
		} while (false);

		if (b_result) {
			//push_info(_ns_tools::ct_color::COLOR_NORMAL, L"_execute_kernel[%c] : pushed : session = %u", (wchar_t)request.get_action(), request.get_session_number());
		}
		return b_complete;//complete
	}

}//_mp
