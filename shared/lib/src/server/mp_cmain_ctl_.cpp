#include <websocket/mp_win_nt.h>

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include <mp_type.h>
#include <mp_cqueue.h>

#include <server/mp_cmain_ctl_.h>
#include <server/mp_cserver_.h>
#include <hid/mp_clibhid.h>
#include <_dev/mp_cdev_util.h>

namespace _mp {

		cmain_ctl::~cmain_ctl()
		{
		}
		cmain_ctl::cmain_ctl(clog* p_log) : cworker_ctl(p_log)
		{
			_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
		}

		/**
		* executed by worker thread.
		* processing request.
		* @paramter request request reference
		* @return true -> complete(with error or success), false -> not complete 
		*/
		bool cmain_ctl::_execute(cio_packet& request)
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
						m_p_log->log_fmt(L"[E] %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					else {
						m_p_log->log_fmt(L"[E] %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					b_completet = _execute_general_error_response(request, response, cio_packet::error_reason_overflow_buffer);
					continue;
				}
				switch (request.get_action())
				{
				case cio_packet::act_mgmt_get_echo:
					b_completet = _execute_mgmt_get_echo(request, response);
					break;
				case cio_packet::act_mgmt_get_device_list:
					request.get_data_field(list_wstring_data_field);
					b_completet = _execute_mgmt_get_device_list(list_wstring_data_field, request, response);
					break;
					/*
				case cio_packet::act_mgmt_ctl_show:
					b_completet = _execute_mgmt_ctl_show(request, response);
					break;
				case cio_packet::act_mgmt_file_operation:
					b_completet = _execute_file_operation(request, response);
					break;
				case cio_packet::act_mgmt_advance_operation:
					b_completet = _execute_advance_operation(request, response, response_for_the_other_session);
					break;
				case cio_packet::act_mgmt_dev_kernel_operation:
					b_completet = _execute_kernel_operation(request, response);
					break;
					*/
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

		bool cmain_ctl::_execute_general_error_response(cio_packet& request, cio_packet& response, cio_packet::type_error_reason n_reason /*= cio_packet::error_reason_none*/)
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

		bool cmain_ctl::_execute_mgmt_get_echo(cio_packet& request, cio_packet& response)
		{
			response = request;
			response.set_cmd(cio_packet::cmd_response);
			return true;//complete
		}

		bool cmain_ctl::_execute_mgmt_get_device_list(
			const type_list_wstring& list_wstring_filter,
			cio_packet& request,
			cio_packet& response
		)
		{
			type_set_wstring out_set_wstring_filter;

			for (auto s_item : list_wstring_filter) {
				out_set_wstring_filter.insert(s_item);
			}//end for
			size_t n_dev(0);
			type_set_wstring set_dev_path;
			type_v_bm_dev v_type =
			{
				type_bm_dev_winusb,
				type_bm_dev_hid,
				type_bm_dev_virtual
			};

			do {
				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					continue;
				}

				if (out_set_wstring_filter.size() == 0) {
					//all primitive device. using default.
					v_type = { type_bm_dev_hid,type_bm_dev_winusb };

					_mp::clibhid_dev_info::type_set st = lib_hid.get_cur_device_set();
					n_dev = st.size();
					continue;
				}


				bool b_lpu200_filter(false);
				std::wstring s_first, s_second;
				std::tie(b_lpu200_filter, s_first, s_second) = cdev_util::is_lpu200_filter(list_wstring_filter);
				if (b_lpu200_filter) {
					//lpu200 filter
					v_type.clear();
					v_type.push_back(cdev_util::get_device_type_from_lpu200_filter(s_second));

					//get composite type path
					clibhid_dev_info::type_set set_dev = lib_hid.get_cur_device_set();
					n_dev = set_dev.size();
					set_dev_path = clibhid_dev_info::get_dev_path_by_wstring(set_dev);
					//n_dev = _ns_tools::ct_universal_dev_manager::get_instance().get_dev_set(set_dev_path, type_set_wstring(), v_type);
					continue;
				}
				if (!cio_packet::adjust_device_filter(out_set_wstring_filter))
					continue;
				//
				v_type = cdev_util::get_device_type_from_device_filter(out_set_wstring_filter);


				clibhid_dev_info::type_set set_dev = lib_hid.get_cur_device_set();
				n_dev = set_dev.size();
				set_dev_path = clibhid_dev_info::get_dev_path_by_wstring(set_dev);
				//n_dev = _ns_tools::ct_universal_dev_manager::get_instance().get_dev_set_except_compositive_type(set_dev_path, out_set_wstring_filter, v_type);

			} while (false);

			for (auto item : set_dev_path) {
				m_p_log->log_fmt(L"[I] %ls | dev : %ls.\n", __WFUNCTION__, item.c_str());
			}

			response = request;
			response.set_cmd(cio_packet::cmd_response);
			response.set_data_by_utf8(set_dev_path);
			return true;//complete
		}
		/*
		bool cmain_ctl::_execute_mgmt_ctl_show(cio_packet& request, cio_packet& response)
		{
			bool b_error(false);
			type_set_wstring set_wstring_data_field;

			if (request.get_data_field(set_wstring_data_field) != 3) {
				b_error = true;
			}
			else {
				if ((set_wstring_data_field.find(L"espresso") == std::end(set_wstring_data_field)) ||
					(set_wstring_data_field.find(L"cappuccino") == std::end(set_wstring_data_field))
					) {
					//not found unlock code.
					b_error = true;
				}
				else {
					if (set_wstring_data_field.find(L"show") != std::end(set_wstring_data_field)) {
						if (m_h_root_dlg)
							::PostMessage(m_h_root_dlg, _shared_const::WM_MSG_SHOW, 0, 0);
					}
					else if (set_wstring_data_field.find(L"hide") != std::end(set_wstring_data_field)) {
						if (m_h_root_dlg)
							::PostMessage(m_h_root_dlg, _shared_const::WM_MSG_HIDE, 0, 0);
					}
					else {
						b_error = true;
					}
				}
			}

			response = request;
			response.set_cmd(cio_packet::cmd_response);

			if (b_error)	response.set_data_error();//don't assign error reason for security.
			else		response.set_data_sucesss();

			return true;//complete
		}

		bool cmain_ctl::_execute_file_operation(cio_packet& request, cio_packet& response)
		{
			CcoffeemanagerDlg* p_dlg_parent(nullptr);
			_ns_tools::type_deque_wstring deque_wstring_data_field;
			unsigned long n_session = request.get_session_number();
			_ws_tools::cws_server::csession::type_ptr_session ptr_session;
			std::wstring s_file, s_sub;

			do {
				response = request;
				response.set_cmd(cio_packet::cmd_response);
				response.set_data_error();

				ptr_session = cserver::get_instance().get_session(n_session);
				if (!ptr_session) {
					m_p_log->log(L"[E] %ls | get_session() == NULL.\n", __WFUNCTION__);
					continue;
				}
				if (request.get_data_field_type() == cio_packet::data_field_binary) {
					//append mode
					_ns_tools::type_v_buffer v_data(0);
					if (request.get_data_field(v_data) == 0) {
						m_p_log->log(L"[E] %ls | get_data_field() == empty.\n", __WFUNCTION__);
						continue;
					}
					if (!ptr_session->get_first_opened_file_path(s_file)) {
						m_p_log->log(L"[E] %ls | get_first_opened_file_path().\n", __WFUNCTION__);
						continue;
					}

					if (!ptr_session->file_write(s_file, v_data)) {
						m_p_log->log(L"[E] %ls | file_write().\n", __WFUNCTION__);
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (request.get_data_field(deque_wstring_data_field) == 0) {
					m_p_log->log(L"[E] %ls | get_data_field() none.\n", __WFUNCTION__);
					continue;
				}
				//
				if (deque_wstring_data_field[0].compare(L"firmware") == 0) {
					if (deque_wstring_data_field.size() != 2) {
						m_p_log->log_fmt(L"[E] %ls | firmware | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						continue;
					}
					s_sub = deque_wstring_data_field[1];
					std::wstring s_virtual_abs_file(ccoffee_path::get_virtual_path_of_temp_rom_file_of_session(n_session));
					if (!ptr_session->file_firmware(s_sub, s_virtual_abs_file)) {
						m_p_log->log_fmt(L"[E] %ls | file_firmware(%ls,%ls).\n", __WFUNCTION__, s_sub.c_str(), s_virtual_abs_file.c_str());
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"create") == 0) {
					if (deque_wstring_data_field.size() != 2) {
						m_p_log->log_fmt(L"[E] %ls | create | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						continue;
					}
					s_file = deque_wstring_data_field[1];
					if (!ptr_session->file_create(s_file)) {
						m_p_log->log_fmt(L"[E] %ls | file_create(%ls).\n", __WFUNCTION__, s_file.c_str());
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"open") == 0) {
					if (deque_wstring_data_field.size() != 2) {
						m_p_log->log_fmt(L"[E] %ls | open | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						continue;
					}
					s_file = deque_wstring_data_field[1];
					if (!ptr_session->file_open(s_file)) {
						m_p_log->log_fmt(L"[E] %ls | file_open(%ls).\n", __WFUNCTION__, s_file.c_str());
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"close") == 0) {
					if (deque_wstring_data_field.size() != 1) {
						m_p_log->log_fmt(L"[E] %ls | close | deque_wstring_data_field.size() != 1.\n", __WFUNCTION__);
						continue;
					}
					if (!ptr_session->get_first_opened_file_path(s_file)) {
						m_p_log->log_fmt(L"[E] %ls | get_first_opened_file_path().\n", __WFUNCTION__);
						continue;
					}
					if (!ptr_session->file_close(s_file)) {
						m_p_log->log_fmt(L"[E] %ls | file_close(%ls).\n", __WFUNCTION__, s_file.c_str());
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"delete") == 0) {
					if (deque_wstring_data_field.size() != 2) {
						m_p_log->log_fmt(L"[E] %ls | delete | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						continue;
					}
					s_file = ccoffee_path::get_path_of_virtual_drive_root_except_backslash();
					s_file += L"\\";
					s_file += deque_wstring_data_field[1];
					if (!DeleteFile(s_file.c_str())) {
						m_p_log->log_fmt(L"[E] %ls | DeleteFile(%ls).\n", __WFUNCTION__, s_file.c_str());
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"truncate") == 0) {
					if (deque_wstring_data_field.size() != 1) {
						m_p_log->log_fmt(L"[E] %ls | truncate | deque_wstring_data_field.size() != 1.\n", __WFUNCTION__);
						continue;
					}
					if (!ptr_session->get_first_opened_file_path(s_file)) {
						m_p_log->log_fmt(L"[E] %ls | get_first_opened_file_path().\n", __WFUNCTION__);
						continue;
					}
					if (!ptr_session->file_truncate(s_file)) {
						m_p_log->log_fmt(L"[E] %ls | file_truncate(%ls).\n", __WFUNCTION__, s_file.c_str());
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}

				if (deque_wstring_data_field[0].compare(L"get") == 0) {
					if (deque_wstring_data_field.size() != 2) {
						m_p_log->log_fmt(L"[E] %ls | delete | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						continue;
					}
					if (deque_wstring_data_field[1].compare(L"size") == 0) {
						if (!ptr_session->get_first_opened_file_path(s_file)) {
							m_p_log->log_fmt(L"[E] %ls | get_first_opened_file_path().\n", __WFUNCTION__);
							continue;
						}
						int n_size = ptr_session->file_get_size(s_file);
						if (n_size < 0) {
							m_p_log->log_fmt(L"[E] %ls | file_get_size(%ls) = %d.\n", __WFUNCTION__, s_file.c_str(), n_size);
							continue;
						}
						//
						response.set_data_by_utf8(L"success", false);
						response.set_data_by_utf8(std::to_wstring(n_size), true);
						continue;
					}
					if (deque_wstring_data_field[1].compare(L"list") == 0) {
						std::wstring s_root = ccoffee_path::get_path_of_virtual_drive_root_except_backslash();
						type_list_wstring list_found;
						//find only files.
						_ns_tools::ct_file::get_find_file_list(list_found, s_root, L"*", _ns_tools::ct_file::folder_area_current_folder, false);

						//extraction file name & extention from result.
						response.set_data_by_utf8(L"success", false);

						for (type_list_wstring::iterator it = std::begin(list_found); it != std::end(list_found); ++it) {
							*it = _ns_tools::ct_file::get_file_name_and_extention_only_from_path(*it);
							response.set_data_by_utf8(*it, true);
						}//end for
						continue;
					}
					continue;
				}

				//
			} while (false);

			return true;//complete
		}

		bool cmain_ctl::_execute_advance_operation(cio_packet& request, cio_packet& response, cio_packet& response_for_the_other_session)
		{
			CcoffeemanagerDlg* p_dlg_parent(nullptr);
			_ns_tools::type_deque_wstring deque_wstring_data_field;
			unsigned long n_session = request.get_session_number();
			_ws_tools::cws_server::csession::type_ptr_session ptr_session;
			std::wstring s_file, s_sub, s_session_name;

			do {
				response_for_the_other_session.set_cmd(cio_packet::cmd_invalid);

				response = request;
				response.set_cmd(cio_packet::cmd_response);
				response.set_data_error();

				ptr_session = cserver::get_instance().get_session(n_session);
				if (!ptr_session)
					continue;
				if (request.get_data_field_type() == cio_packet::data_field_binary) {
					//not supported format.
					continue;
				}
				if (request.get_data_field(deque_wstring_data_field) == 0)
					continue;
				//
				if (deque_wstring_data_field[0].compare(L"set_session_name") == 0) {
					if (deque_wstring_data_field.size() > 2)
						continue;
					if (deque_wstring_data_field.size() == 1)
						s_session_name = ccontroller::get_instance().generate_session_name(n_session);//auto generated session name.
					else
						s_session_name = deque_wstring_data_field[1];
					if (!ccontroller::get_instance().set_session_name(n_session, s_session_name)) {
						continue;//already session has a name.
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"get_session_name") == 0) {
					if (deque_wstring_data_field.size() != 1)
						continue;
					s_session_name = ccontroller::get_instance().get_session_name(n_session);
					if (s_session_name.empty())
						continue;
					//
					response.set_data_sucesss();
					response.set_data_by_utf8(s_session_name, true);
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"send_data_to_session") == 0) {
					if (deque_wstring_data_field.size() < 3)
						continue;
					s_session_name = deque_wstring_data_field[1];//target session name
					unsigned long n_session_dest(ccontroller::get_instance().get_session_number_from_session_name(s_session_name));
					if (n_session_dest == _NS_TOOLS_INVALID_SESSION_NUMBER)
						continue;
					if (n_session == n_session_dest)
						continue;//source and destination cannot be equal.

					s_session_name = ccontroller::get_instance().get_session_name(n_session);
					if (s_session_name.empty())
						continue;
					// redirect to n_session_dest session.
					response_for_the_other_session.set_data_from_send_data_to_session_request(request, s_session_name, n_session_dest);
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"send_data_to_all") == 0) {
					if (deque_wstring_data_field.size() < 2)
						continue;
					s_session_name = ccontroller::get_instance().get_session_name(n_session);
					if (s_session_name.empty())
						continue;
					// redirect to boardcast
					response_for_the_other_session.set_data_from_send_data_to_session_request(request, s_session_name);
					response.set_data_sucesss();
					continue;
				}
				//
			} while (false);

			return true;//complete
		}

		bool cmain_ctl::_execute_kernel_operation(cio_packet& request, cio_packet& response)
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
				_ws_tools::ckernel& kernel(ptr_session->get_kernel());

				type_list_wstring list_result;

				// process_for_manager_service process service type.
				_ns_tools::type_pair_bool_result_bool_complete pair_bool_result_bool_complete;
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
		*/

}//the end of _mp namespace

