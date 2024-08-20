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
#include <server/mp_ckernel_ctl_.h>

namespace _mp
{
	ckernel_ctl::ckernel_ctl(unsigned long n_session) : m_n_session(n_session)
	{
	}
	ckernel_ctl::~ckernel_ctl()
	{
		for (auto s_item : m_set_loaded_dll_path) {
			_service_dll_removed(s_item);
		}//end for
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
				b_complete = _execute_mgmt_get_device_list(list_wstring_data_field, request, response);
				continue;
			}

			// each kernel request is executed.
			ckernel_ctl& kernel(ptr_session->get_kernel());

			type_list_wstring list_result;

			// process_for_manager_service process service type.
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete;
			pair_bool_result_bool_complete = kernel.process_for_manager_service(list_wstring_data_field, list_result, cdlg_page_main::_callback_for_service_execute, this);
			b_result = pair_bool_result_bool_complete.first;
			b_complete = pair_bool_result_bool_complete.second;

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
					pair_bool_result_bool_complete = _execute_open_sync(request, response);
					b_result = pair_bool_result_bool_complete.first;
					b_complete = pair_bool_result_bool_complete.second;
					response.set_kernel_device_open(true);
					continue;
				}
				if (s_action.compare(L"close") == 0) {
					pair_bool_result_bool_complete = _execute_close_sync(request, response);
					b_result = pair_bool_result_bool_complete.first;
					b_complete = pair_bool_result_bool_complete.second;
					response.set_kernel_device_close(true);
					continue;
				}
			}

			if (get_connected_session() != n_session) {
				response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_session));
				push_info(_ns_tools::ct_color::COLOR_ERROR, L"%s[%c] : mismatched : session = %u", __WFUNCTION__, (wchar_t)request.get_action(), request.get_session_number());
				continue;
			}
			ptr_session = cserver::get_instance().get_session(n_session);
			if (!ptr_session)
				continue;

			if (s_type.compare(L"service") == 0) {
				if (s_action.compare(L"execute") != 0 && s_action.compare(L"cancel") != 0) {
					continue;
				}
				_ws_tools::ckernel_ctl& kernel(ptr_session->get_kernel());

				_ns_tools::type_list_wstring list_result;
				_ns_tools::ct_async_dev& async_dev = _ns_tools::ct_universal_dev_manager::get_instance().get_async_dev(get_composite_dev_path());
				if (!async_dev.is_open())
					continue;

				if (s_action.compare(L"execute") == 0) {
					pair_bool_result_bool_complete = kernel.process_for_device_service_execute(
						list_wstring_data_field
						, list_result
						, cdlg_page_device::_sync_dev_transmit_for_c_language
						, async_dev.get_device_pointer()
						, cdlg_page_device::_callback_for_service_execute
						, this
					);
				}
				else {//s_action == L"cancel"
					pair_bool_result_bool_complete = kernel.process_for_device_service_cancel(
						list_wstring_data_field
						, list_result
						, cdlg_page_device::_sync_dev_cancel_for_c_language
						, async_dev.get_device_pointer()
					);
				}
				//
				b_result = pair_bool_result_bool_complete.first;
				b_complete = pair_bool_result_bool_complete.second;

				if (b_complete && list_result.empty()) {
					//when requst is completed, list_result must have the returned strings.
					continue;
				}
				response.set_data_by_utf8(list_result);
				continue;
			}
		} while (false);

		if (b_result) {
			push_info(_ns_tools::ct_color::COLOR_NORMAL, L"_execute_kernel[%c] : pushed : session = %u", (wchar_t)request.get_action(), request.get_session_number());
		}
		return b_complete;//complete
	}

	unsigned long ckernel_ctl::get_session_number() const
	{
		return m_n_session;
	}

	/**
		* process_for_manager_service.
		*
		* \param req_data_field - the data field of request.
		* \param list_result - if return value must be started with "success", "pending" or "error"
		* \param cb
		* \param p_user
		* \return
		*/
	type_pair_bool_result_bool_complete ckernel_ctl::process_for_manager_service(
		const type_list_wstring& req_data_field, type_list_wstring& list_result, const type_cb_sd_execute cb, void* p_user
	)
	{
		bool b_result(false);
		bool b_complete(true);

		do {
			list_result.clear();
			list_result.push_back(L"error");

			if (req_data_field.empty()) {
				ATLTRACE(L"%s :E: req_data_field.empty() : .\n", __WFUNCTION__);
				continue;//not condition
			}
			//req_data_field ~ the starting string of this list are load x, unload x, or execute x.
			std::wstring s_req = req_data_field.front();

			type_list_wstring list_token;
			if (cconvert::tokenizer(list_token, s_req, L" ") != 3) {
				ATLTRACE(L"%s :E: data field has not 3 tokens : .\n", __WFUNCTION__);
				continue;//not condition
			}
			std::wstring s_action = list_token.front();	//load, unload, execute, cancel, open ,close
			list_token.pop_front();
			std::wstring s_type = list_token.front(); //service, device
			list_token.pop_front();

			std::wstring s_path;
			if (!list_token.empty()) {
				s_path = list_token.front(); //default/xxx.dll, 3thpart/xxx.dll
				list_token.pop_front();
			}

			if (s_type.compare(L"service") != 0) {
				ATLTRACE(L"%s :E: type must be service : .\n", __WFUNCTION__);
				continue;
			}
			if (s_action.compare(L"load") == 0) {
				std::lock_guard<std::mutex> lock(m_mutex_for_path);
				if (!_service_dll_load_with_abs_path(s_path)) {
					ATLTRACE(L"%s :E: _service_dll_load_with_abs_path() : .\n", __WFUNCTION__);
					continue;//not condition
				}
				list_result.clear();
				list_result.push_back(L"success");//1'th string complete
				b_result = true;
				continue;
			}
			if (s_action.compare(L"unload") == 0) {
				std::lock_guard<std::mutex> lock(m_mutex_for_path);
				if (!_service_dll_unload(s_path)) {
					ATLTRACE(L"%s :E: _service_dll_unload() : .\n", __WFUNCTION__);
					continue;//not condition
				}
				list_result.clear();
				list_result.push_back(L"success");//1'th string compete
				b_result = true;
				continue;
			}
			if (s_action.compare(L"cancel") == 0) {
				std::lock_guard<std::mutex> lock(m_mutex_for_path);
				//not used device, thesefore p_fun_sd_device_cancel and p_dev is null.
				if (!_service_dll_cancel(s_path, nullptr, nullptr)) {
					ATLTRACE(L"%s :E: _service_dll_cancel() : .\n", __WFUNCTION__);
					continue;//not condition
				}
				list_result.clear();
				list_result.push_back(L"success");//1'th string compete
				b_result = true;
				continue;
			}
			if (s_action.compare(L"execute") != 0) {
				ATLTRACE(L"%s :E: action must be execute : .\n", __WFUNCTION__);
				continue;//not condition
			}
			//execute part
			std::lock_guard<std::mutex> lock(m_mutex_for_path);
			type_list_wstring list_parameters;

			list_parameters = req_data_field;
			list_parameters.pop_front();
			//not used device, thesefore p_fun_sd_device_io and p_dev is null.
			if (!_service_dll_execute(s_path, nullptr, nullptr, list_parameters, cb, p_user)) {
				ATLTRACE(L"%s :E: _service_dll_execute() : .\n", __WFUNCTION__);
				continue;//not condition
			}
			//
			// here!
			list_result.clear();
			list_result.push_back(L"pending");

			b_result = true;
			b_complete = false;
		} while (false);

		return std::make_pair(b_result, b_complete);
	}

	/**
		* process_for_device_service_execute.
		*
		* \param req_data_field - the data field of request.
		* \param list_result - if return value must be started with "success", "pending" or "error"
		* \param p_fun_sd_device_io - device io function pointer
		* \param cb
		* \param p_user
		* \return
		*/
	type_pair_bool_result_bool_complete ckernel_ctl::process_for_device_service_execute(
		const type_list_wstring& req_data_field
		, type_list_wstring& list_result
		, const type_fun_sd_device_io p_fun_sd_device_io
		, ct_i_dev* p_dev
		, const type_cb_sd_execute cb
		, void* p_user
	)
	{
		bool b_result(false);
		bool b_complete(true);

		do {
			list_result.clear();
			list_result.push_back(L"error");

			if (req_data_field.empty()) {
				ATLTRACE(L"%s :E: req_data_field.empty() : .\n", __WFUNCTION__);
				continue;//not condition
			}
			if (p_fun_sd_device_io == nullptr) {
				ATLTRACE(L"%s :E: p_fun_sd_device_io == nullptr : .\n", __WFUNCTION__);
				continue;//not condition
			}

			//req_data_field ~ the starting string of this list are execute x.
			std::wstring s_req = req_data_field.front();

			type_list_wstring list_token;
			if (cconvert::tokenizer(list_token, s_req, L" ") != 3) {
				ATLTRACE(L"%s :E: data field has not 3 tokens : .\n", __WFUNCTION__);
				continue;//not condition
			}
			std::wstring s_action = list_token.front();	//execute
			list_token.pop_front();
			std::wstring s_type = list_token.front(); //service, device
			list_token.pop_front();

			std::wstring s_path;
			if (!list_token.empty()) {
				s_path = list_token.front(); //default/xxx.dll, 3thpart/xxx.dll
				list_token.pop_front();
			}

			if (s_action.compare(L"execute") != 0) {
				ATLTRACE(L"%s :E: action must be execute : .\n", __WFUNCTION__);
				continue;
			}
			if (s_type.compare(L"service") != 0) {
				ATLTRACE(L"%s :E: type must be service : .\n", __WFUNCTION__);
				continue;
			}

			//execute part
			std::lock_guard<std::mutex> lock(m_mutex_for_path);
			type_list_wstring list_parameters;

			list_parameters = req_data_field;
			list_parameters.pop_front();
			if (!_service_dll_execute(s_path, p_fun_sd_device_io, p_dev, list_parameters, cb, p_user)) {
				ATLTRACE(L"%s :E: _service_dll_execute() : .\n", __WFUNCTION__);
				continue;//not condition
			}
			//
			// here!
			list_result.clear();
			list_result.push_back(L"pending");
			b_complete = false;
			b_result = true;
		} while (false);

		return std::make_pair(b_result, b_complete);
	}

	/**
		* process_for_device_service_cancel.
		*
		* \param req_data_field - the data field of request.
		* \param list_result - if return value must be started with "success", "pending" or "error"
		* \param p_fun_sd_device_io - device io function pointer
		* \param cb
		* \param p_user
		* \return
		*/
	type_pair_bool_result_bool_complete ckernel_ctl::process_for_device_service_cancel(
		const type_list_wstring& req_data_field
		, type_list_wstring& list_result
		, const type_fun_sd_device_cancel p_fun_sd_device_cancel
		, ct_i_dev* p_dev
	)
	{
		bool b_result(false);
		bool b_complete(true);

		do {
			list_result.clear();
			list_result.push_back(L"error");

			if (req_data_field.empty()) {
				ATLTRACE(L"%s :E: req_data_field.empty() : .\n", __WFUNCTION__);
				continue;//not condition
			}
			if (p_fun_sd_device_cancel == nullptr) {
				ATLTRACE(L"%s :E: p_fun_sd_device_cancel == nullptr : .\n", __WFUNCTION__);
				continue;//not condition
			}

			//req_data_field ~ the starting string of this list are execute x.
			std::wstring s_req = req_data_field.front();

			type_list_wstring list_token;
			if (cconvert::tokenizer(list_token, s_req, L" ") != 3) {
				ATLTRACE(L"%s :E: data field has not 3 tokens : .\n", __WFUNCTION__);
				continue;//not condition
			}
			std::wstring s_action = list_token.front();	//execute
			list_token.pop_front();
			std::wstring s_type = list_token.front(); //service, device
			list_token.pop_front();

			std::wstring s_path;
			if (!list_token.empty()) {
				s_path = list_token.front(); //default/xxx.dll, 3thpart/xxx.dll
				list_token.pop_front();
			}

			if (s_action.compare(L"cancel") != 0) {
				ATLTRACE(L"%s :E: action must be cancel : .\n", __WFUNCTION__);
				continue;
			}
			if (s_type.compare(L"service") != 0) {
				ATLTRACE(L"%s :E: type must be service : .\n", __WFUNCTION__);
				continue;
			}

			//cancel part
			std::lock_guard<std::mutex> lock(m_mutex_for_path);
			if (!_service_dll_cancel(s_path, p_fun_sd_device_cancel, p_dev)) {
				ATLTRACE(L"%s :E: _service_dll_cancel() : .\n", __WFUNCTION__);
				continue;//not condition
			}
			//
			// here!
			list_result.clear();
			list_result.push_back(L"success");
			b_result = true;
		} while (false);

		return std::make_pair(b_result, b_complete);
	}

	/**
		* _service_dll_unload.
		*
		* \param
		*		- s_dll : default/xxx.dll or 3thpart/xxx.dll
		* \return
		*/
	bool ckernel_ctl::_service_dll_unload(const std::wstring& s_dll)
	{
		bool b_result(false);
		do {
			bool b_default(true);
			std::wstring s_file = _check_service_path(b_default, s_dll);
			if (s_file.empty()) {
				ATLTRACE(L"%s :E: _check_service_path() : .\n", __WFUNCTION__);
				continue;
			}
			//
			if (b_default) {
				auto found_it = m_map_default_path_pair_ref_cnt_ptr_dll.find(s_dll);
				if (found_it == std::end(m_map_default_path_pair_ref_cnt_ptr_dll)) {
					ATLTRACE(L"%s :E: not sd_dll : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				if (found_it->second.first > 1) {
					found_it->second.first--;//decrease reference counter
					b_result = true;
					ATLTRACE(L"%s :I: UNLOADED(%d) : %s.\n", __WFUNCTION__, found_it->second.first, s_dll.c_str());
					continue;
				}

				_service_dll_removed(s_dll);
				m_map_default_path_pair_ref_cnt_ptr_dll.erase(found_it);
				m_set_loaded_dll_path.erase(s_dll);
				ATLTRACE(L"%s :I: UNLOADED(%d) : %s.\n", __WFUNCTION__, 0, s_dll.c_str());
			}
			else {
				auto found_it = m_map_3th_path_pair_ref_cnt_ptr_dll.find(s_dll);
				if (found_it == std::end(m_map_3th_path_pair_ref_cnt_ptr_dll)) {
					ATLTRACE(L"%s :E: not sd_dll : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				if (found_it->second.first > 1) {
					found_it->second.first--;//decrease reference counter
					b_result = true;
					ATLTRACE(L"%s :I: UNLOADED(%d) : %s.\n", __WFUNCTION__, found_it->second.first, s_dll.c_str());
					continue;
				}

				_service_dll_removed(s_dll);
				m_map_3th_path_pair_ref_cnt_ptr_dll.erase(found_it);
				m_set_loaded_dll_path.erase(s_dll);
				ATLTRACE(L"%s :I: UNLOADED(%d) : %s.\n", __WFUNCTION__, 0, s_dll.c_str());
			}
			//
			b_result = true;
		} while (false);
		return b_result;
	}

	/**
		* _service_dll_cancel.
		*
		* \param
		*		- s_dll : default/xxx.dll or 3thpart/xxx.dll
		*		- p_fun_sd_device_cancel : device cancel fnuction pointer
		*		- p_dev : p_fun_sd_device_cancel()'s the first parameter.
		* \return
		*/
	bool ckernel_ctl::_service_dll_cancel(const std::wstring& s_dll, const type_fun_sd_device_cancel p_fun_sd_device_cancel, ct_i_dev* p_dev)
	{
		bool b_result(false);

		do {
			bool b_default(true);
			std::wstring s_file = _check_service_path(b_default, s_dll);
			if (s_file.empty()) {
				ATLTRACE(L"%s :E: _check_service_path() : .\n", __WFUNCTION__);
				continue;
			}

			std::wstring s_device_path;
			if (p_dev)
				s_device_path = p_dev->dev_get_path();

			if (b_default) {
				auto found_it = m_map_default_path_pair_ref_cnt_ptr_dll.find(s_dll);
				if (found_it == std::end(m_map_default_path_pair_ref_cnt_ptr_dll)) {
					ATLTRACE(L"%s :E: not sd_dll : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				if (!found_it->second.second) {
					ATLTRACE(L"%s :E: not sd_dll instance : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				b_result = found_it->second.second->sd_cancel(m_n_session, s_device_path, p_fun_sd_device_cancel, p_dev);
			}
			else {
				auto found_it = m_map_3th_path_pair_ref_cnt_ptr_dll.find(s_dll);
				if (found_it == std::end(m_map_3th_path_pair_ref_cnt_ptr_dll)) {
					ATLTRACE(L"%s :E: not sd_dll : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				if (!found_it->second.second) {
					ATLTRACE(L"%s :E: not sd_dll instance : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				b_result = found_it->second.second->sd_cancel(m_n_session, s_device_path, p_fun_sd_device_cancel, p_dev);
			}
			//
		} while (false);
		return b_result;
	}

	/**
		* _service_dll_execute.
		*
		* \param s_dll - default/xxx.dll or 3thpart/xxx.dll
		* \param list_parameters[in] -  wstring list
		* \param p_fun_sd_device_io[in] - device io function poniter
		* \param p_dev[in] - p_fun_sd_device_io() 's the first parameter.
		* \param cb
		* \param p_user
		* \return : true - start success.
		*		   : else - error
		*/
	bool ckernel_ctl::_service_dll_execute(
		const std::wstring& s_dll
		, const type_fun_sd_device_io p_fun_sd_device_io
		, ct_i_dev* p_dev
		, const type_list_wstring& list_parameters
		, const type_cb_sd_execute cb, void* p_user
	)
	{
		bool b_result(false);

		do {
			bool b_default(true);
			std::wstring s_file = _check_service_path(b_default, s_dll);
			if (s_file.empty()) {
				ATLTRACE(L"%s :E: _check_service_path() : .\n", __WFUNCTION__);
				continue;
			}

			std::wstring s_device_path;
			if (p_dev)
				s_device_path = p_dev->dev_get_path();

			if (b_default) {
				auto found_it = m_map_default_path_pair_ref_cnt_ptr_dll.find(s_dll);
				if (found_it == std::end(m_map_default_path_pair_ref_cnt_ptr_dll)) {
					ATLTRACE(L"%s :E: not sd_dll : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				if (!found_it->second.second) {
					ATLTRACE(L"%s :E: not sd_dll instance : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				b_result = found_it->second.second->sd_execute(m_n_session, s_device_path, p_fun_sd_device_io, p_dev, list_parameters, cb, p_user);
			}
			else {
				auto found_it = m_map_3th_path_pair_ref_cnt_ptr_dll.find(s_dll);
				if (found_it == std::end(m_map_3th_path_pair_ref_cnt_ptr_dll)) {
					ATLTRACE(L"%s :E: not sd_dll : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				if (!found_it->second.second) {
					ATLTRACE(L"%s :E: not sd_dll instance : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				b_result = found_it->second.second->sd_execute(m_n_session, s_device_path, p_fun_sd_device_io, p_dev, list_parameters, cb, p_user);
			}
			//
		} while (false);
		return b_result;
	}

	/**
		* _service_dll_removed. - you must call this function when session is removed.
		*
		* \param s_dll - default/xxx.dll or 3thpart/xxx.dll
		* \return : none
		*/
	void ckernel_ctl::_service_dll_removed(const std::wstring& s_dll)
	{
		do {
			bool b_default(true);
			std::wstring s_file = _check_service_path(b_default, s_dll);
			if (s_file.empty()) {
				ATLTRACE(L"%s :E: _check_service_path() : .\n", __WFUNCTION__);
				continue;
			}

			if (b_default) {
				auto found_it = m_map_default_path_pair_ref_cnt_ptr_dll.find(s_dll);
				if (found_it == std::end(m_map_default_path_pair_ref_cnt_ptr_dll)) {
					ATLTRACE(L"%s :E: not sd_dll : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				if (!found_it->second.second) {
					ATLTRACE(L"%s :E: not sd_dll instance : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				found_it->second.second->sd_removed(m_n_session);
			}
			else {
				auto found_it = m_map_3th_path_pair_ref_cnt_ptr_dll.find(s_dll);
				if (found_it == std::end(m_map_3th_path_pair_ref_cnt_ptr_dll)) {
					ATLTRACE(L"%s :E: not sd_dll : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				if (!found_it->second.second) {
					ATLTRACE(L"%s :E: not sd_dll instance : %s.\n", __WFUNCTION__, s_dll.c_str());
					continue;
				}
				found_it->second.second->sd_removed(m_n_session);
			}
			//
		} while (false);
	}

	/**
		* _check_service_path.
		*
		* \param s_dll - default/xxx.dll or 3thpart/xxx.dll
		* \return dll name( dll name & extention only )
		*/
	std::wstring ckernel_ctl::_check_service_path(bool& b_default, const std::wstring& s_dll)
	{
		bool b_result(false);
		std::wstring s_file;

		do {
			if (s_dll.empty()) {
				ATLTRACE(L"%s :E: s_dll.empty(): .\n", __WFUNCTION__);
				continue;
			}
			//
			std::wstring s_out_drive;
			std::wstring s_out_dir;
			std::wstring s_out_file_name;
			std::wstring s_out_ext;

			cfile::split_path(s_dll, s_out_drive, s_out_dir, s_out_file_name, s_out_ext);
			if (!s_out_drive.empty()) {
				ATLTRACE(L"%s :E: !s_out_drive.empty() : .\n", __WFUNCTION__);
				continue;
			}
			if (s_out_file_name.empty()) {
				ATLTRACE(L"%s :E: s_out_file_name.empty() : .\n", __WFUNCTION__);
				continue;
			}
			if (s_out_ext.compare(L".dll") != 0) {
				ATLTRACE(L"%s :E: s_out_ext is not dll : .\n", __WFUNCTION__);
				continue;
			}
			if (s_out_dir.compare(L"default/") == 0)
				b_default = true;
			else if (s_out_dir.empty())
				b_default = true;
			else if (s_out_dir.compare(L"3thpart/") == 0) {
				b_default = false;
			}
			else {
				ATLTRACE(L"%s :E: dir is not default/ or 3thpart/ : .\n", __WFUNCTION__);
				continue;
			}
			//
			s_file = s_out_file_name + s_out_ext;
			b_result = true;
		} while (false);
		return s_file;
	}

	/**
		* _service_dll_load_with_abs_path.
		*
		* \param
		*		- s_dll : default/xxx.dll or 3thpart/xxx.dll
		* \return
		*/
	bool ckernel_ctl::_service_dll_load_with_abs_path(const std::wstring& s_dll_short)
	{
		bool b_result(false);
		cdll_service::type_ptr_cdll_service ptr_sdll;

		do {
			bool b_default(true);
			std::wstring s_file = _check_service_path(b_default, s_dll_short);
			if (s_file.empty()) {
				ATLTRACE(L"%s :E: _check_service_path() : .\n", __WFUNCTION__);
				continue;
			}

			std::wstring s_full_path;
			ckernel_ctl::_type_map_path_pair_ref_cnt_ptr_dll::iterator found_it;

			if (b_default) {
				found_it = m_map_default_path_pair_ref_cnt_ptr_dll.find(s_dll_short);
				if (found_it != std::end(m_map_default_path_pair_ref_cnt_ptr_dll)) {
					found_it->second.first++;//increased reference counter.
					b_result = true;//alreay loaded.
					ATLTRACE(L"%s :I: LOADED(%d) : %s.\n", __WFUNCTION__, found_it->second.first, s_full_path.c_str());
					continue;
				}

				s_full_path = ccoffee_path::get_path_of_elpusk_service_dll_folder_except_backslash(L"coffee_manager");
				s_full_path += L"\\";
				s_full_path += s_file;

#ifndef _DEBUG
				if (!csystem::verify_embedded_signature(s_full_path)) {
					clog::get_instance().log_fmt(L"[E] %s | verify_embedded_signature().\n", __WFUNCTION__);
					ATLTRACE(L"%s :E: verify code signature : %s.\n", __WFUNCTION__, s_full_path.c_str());
					continue;
				}
#endif _DEBUG
				ptr_sdll.swap(std::make_shared<cdll_service>());
				if (!ptr_sdll->load(s_full_path)) {
					ptr_sdll.reset();
					ATLTRACE(L"%s :E: LOADING : %s.\n", __WFUNCTION__, s_full_path.c_str());
					continue;
				}
				else {
					ATLTRACE(L"%s :I: LOADED(%d) : %s.\n", __WFUNCTION__, 1, s_full_path.c_str());
				}
				m_map_default_path_pair_ref_cnt_ptr_dll[s_dll_short] = std::make_pair(1, ptr_sdll);
			}
			else {
				//3thpart dll
				found_it = m_map_3th_path_pair_ref_cnt_ptr_dll.find(s_dll_short);
				if (found_it != std::end(m_map_3th_path_pair_ref_cnt_ptr_dll)) {
					found_it->second.first++;//increased reference counter.
					b_result = true;//alreay loaded.
					ATLTRACE(L"%s :I: LOADED(%d) : %s.\n", __WFUNCTION__, found_it->second.first, s_full_path.c_str());
					continue;
				}

				s_full_path = ccoffee_path::get_path_of_not_elpusk_service_dll_folder_except_backslash(L"coffee_manager");
				s_full_path += L"\\";
				s_full_path += s_file;
#ifndef _DEBUG
				if (!csystem::verify_embedded_signature(s_full_path)) {
					clog::get_instance().log_fmt(L"[E] %s | verify_embedded_signature().\n", __WFUNCTION__);
					ATLTRACE(L"%s :E: verify code signature : %s.\n", __WFUNCTION__, s_full_path.c_str());
					continue;
				}
#endif _DEBUG
				ptr_sdll.swap(std::make_shared<cdll_service>());
				if (!ptr_sdll->load(s_full_path)) {
					ptr_sdll.reset();
					ATLTRACE(L"%s :E: LOADING : %s.\n", __WFUNCTION__, s_full_path.c_str());
					continue;
				}
				else {
					ATLTRACE(L"%s :I: LOADED : %s.\n", __WFUNCTION__, s_full_path.c_str());
				}
				m_map_3th_path_pair_ref_cnt_ptr_dll[s_dll_short] = std::make_pair(1, ptr_sdll);
			}
			m_set_loaded_dll_path.insert(s_dll_short);
			b_result = true;
		} while (false);
		return b_result;
	}

}//_mp
