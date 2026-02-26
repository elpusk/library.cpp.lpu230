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

#include <server/mp_cmain_ctl_fn_.h>
#include <server/mp_cserver_.h>
#include <cupdater_mgmt_.h>

#include <server/mp_cctl_svr_.h>
#include <hid/mp_clibhid.h>
#include <_dev/mp_cdev_util.h>

namespace _mp {

		cmain_ctl_fn::~cmain_ctl_fn()
		{
		}
		cmain_ctl_fn::cmain_ctl_fn(clog* p_log) : cbase_ctl_fn(p_log)
		{
			_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
		}

		bool cmain_ctl_fn::_execute_mgmt_get_echo(clog* p_log, cio_packet& request, cio_packet& response)
		{
			response = request;
			response.set_cmd(cio_packet::cmd_response);
			return true;//complete
		}


		bool cmain_ctl_fn::_execute_mgmt_recover_operation(
			clog* p_log
			, cio_packet& request
			, cio_packet& response
		)
		{
			response = request;
			response.set_cmd(cio_packet::cmd_response);
			//
			do {
				_mp::type_deque_wstring deque_s_data;
				cio_packet::type_data_field data_field_type(request.get_data_field_type());

				if (data_field_type == cio_packet::data_field_binary) {//may be run bootloader commnad.
					response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_device_misformat));
					continue;//bootloader request dosen't support binary data.
				}
				if (request.get_data_field(deque_s_data) <= 0) {
					response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_device_misformat));
					continue;
				}

				if (deque_s_data[0].compare(L"set") == 0) {
					//firmware update 를 위한 parameter setting
					if (deque_s_data.size() < 3) {
						response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_device_misformat));
						continue;
					}
					if ((deque_s_data.size() - 1) % 2 != 0) {//"set", key0, value0, key1, value1, .......
						response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_device_misformat));
						continue;
					}

					cupdater_param::type_ptr ptr_boot_param;
					std::tie(std::ignore, ptr_boot_param) = cupdater_mgmt::get_instance().insert(request.get_session_number());
					if (!ptr_boot_param) {
						response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_allocation_memory));
						continue;
					}

					if (ptr_boot_param->is_used_already()) {
						// 전에 다운로드에 사용된 파라메타들 이므로 재 할당 받아서 새롭게 저장해야함.
						ptr_boot_param->reset();
					}
					for (size_t i = 0; i < ((deque_s_data.size() - 1) / 2); i++) {
						std::wstring s_key(deque_s_data[2 * i + 1]);

						std::wstring s_value(deque_s_data[2 * (i + 1)]);
						ptr_boot_param->set(s_key, s_value);
					}//end for

					response.set_data_sucesss();
					continue;
				}

				if (deque_s_data[0].compare(L"start") == 0) {
					if (deque_s_data.size() != 1) {
						response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_device_misformat));
						continue;
					}

					cupdater_mgmt& update_param_mgmt(cupdater_mgmt::get_instance());

					cupdater_param::type_ptr ptr_boot_param = update_param_mgmt.get(request.get_session_number());
					if (!ptr_boot_param) {
						response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_allocation_memory));
						continue;
					}
					ptr_boot_param->set_log_obj(m_p_ctl_fun_log);
					ptr_boot_param->set_used();// 다음 사용 전에 재할당 요망.

					std::wstring s_rom_file = ccoffee_path::get_path_of_virtual_drive_root_with_backslash();
					s_rom_file += ccoffee_path::get_virtual_path_of_temp_rom_file_of_session(request.get_session_number());
					if (!_mp::cfile::is_exist_file(s_rom_file)) {
						response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_file_none_file));
						continue;
					}

					ptr_boot_param->insert_run_by_cf2_mode();
					ptr_boot_param->insert_file(s_rom_file);

					ptr_boot_param->set_packet_info_for_notify_in_recover(request);//?

					ptr_boot_param->set_exe_full_abs_path(ccoffee_path::get_abs_full_path_of_updater());

					bool b_ok(false);
					std::wstring s_info;
					std::tie(b_ok, s_info) = ptr_boot_param->can_be_start_firmware_update(true);
					m_p_ctl_fun_log->log_fmt(L"[I] - %ls | %ls\n", __WFUNCTION__, s_info.c_str());
					m_p_ctl_fun_log->trace(L"[I] - %ls | %ls\n", __WFUNCTION__, s_info.c_str());
					if (!b_ok) {
						// bootloader start command 에 필요한 parameter 가 설정되지 않음.
						response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_bootload_mismatch_condition));
						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | %ls\n", __WFUNCTION__, L"bootload_mismatch_condition");
						m_p_ctl_fun_log->trace(L"[E] - %ls | %ls\n", __WFUNCTION__, L"bootload_mismatch_condition");
						continue;
					}
					// 
					if (!update_param_mgmt.run_update(request.get_session_number(),true)) {
						response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_bootload_mismatch_condition));
						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | %ls\n", __WFUNCTION__, L"update_param_mgmt.run_update(recover)");
						m_p_ctl_fun_log->trace(L"[E] - %ls | %ls\n", __WFUNCTION__, L"update_param_mgmt.run_update(recover)");
						continue;
					}

					response.set_data_sucesss();
					continue;
				}

				// unknown recover command
				// now, only "set", "start" command supported.
				response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_device_misformat));

			} while (false);

			return true;//complete. 이 요청은 항상 complete 이어야. 한다.
		}

		bool cmain_ctl_fn::_execute_mgmt_get_version(clog* p_log, cio_packet& request, cio_packet& response)
		{
			response = request;
			response.set_cmd(cio_packet::cmd_response);
			response.set_data_by_utf8(_mp::_coffee::CONST_WS_COFFEE_MGMT_VERSION);
			return true;//complete
		}

		bool cmain_ctl_fn::_execute_mgmt_get_device_list(
			clog* p_log,
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
				// coffee framework 1'st edition 과 호환성 유지를 위해서 lpu200 filter 인지 확인하는 절차가 필요함. (lpu200 filter 는 "lpu200" 로 시작하는 형태의 filter)
				std::tie(b_lpu200_filter, s_first, s_second) = cdev_util::is_lpu200_filter(list_wstring_filter);
				if (b_lpu200_filter) {
					//lpu200 filter
					v_type.clear();
					v_type.push_back(cdev_util::get_device_type_from_lpu200_filter(s_second));

					//get composite type path
					clibhid_dev_info::type_set set_dev = lib_hid.get_cur_device_set();
					n_dev = clibhid_dev_info::filter_dev_info_set(set_dev, set_dev, v_type);//filtering
					set_dev_path = clibhid_dev_info::get_dev_path_by_wstring(set_dev);
					continue;
				}
				//filter 의 문자를 소문자로 바꿔서 비교하기 위해서 소문자로 바꿔서 저장.
				if (!cio_packet::adjust_device_filter(out_set_wstring_filter))
					continue;
				//
				v_type = cdev_util::get_device_type_from_device_filter(out_set_wstring_filter);


				clibhid_dev_info::type_set set_dev = lib_hid.get_cur_device_set();
				if (out_set_wstring_filter.empty()) {
					n_dev = set_dev.size();
					set_dev_path = clibhid_dev_info::get_dev_path_by_wstring(set_dev);
				}
				else {
					clibhid_dev_info::type_set filtered_set_dev;
					n_dev = clibhid_dev_info::filter_dev_info_set(filtered_set_dev, set_dev, out_set_wstring_filter); // filtering 된 device set 얻음.
					set_dev_path = clibhid_dev_info::get_dev_path_by_wstring(filtered_set_dev);
				}
				//n_dev = _ns_tools::ct_universal_dev_manager::get_instance().get_dev_set_except_compositive_type(set_dev_path, out_set_wstring_filter, v_type);

			} while (false);

			for (auto item : set_dev_path) {
				p_log->log_fmt(L"[I] - %ls | dev : %ls.\n", __WFUNCTION__, item.c_str());
				p_log->trace(L"[I] - %ls | dev : %ls.\n", __WFUNCTION__, item.c_str());
			}

			response = request;
			response.set_cmd(cio_packet::cmd_response);
			response.set_data_by_utf8(set_dev_path);
			return true;//complete
		}
		
		bool cmain_ctl_fn::_execute_mgmt_ctl_show(clog* p_log, cio_packet& request, cio_packet& response)
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
						//cf v1.0 에서는 mgmt 를 표시하는 것이었으나 지금은 아무 것도 하지 않음.
						// 과거 호환성을 위해 남겨둠.
					}
					else if (set_wstring_data_field.find(L"hide") != std::end(set_wstring_data_field)) {
						//cf v1.0 에서는 mgmt 를 감추는 것이었으나 지금은 아무 것도 하지 않음.
						// 과거 호환성을 위해 남겨둠.
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
		
		bool cmain_ctl_fn::_execute_file_operation(clog* p_log, cio_packet& request, cio_packet& response)
		{
			_mp::type_deque_wstring deque_wstring_data_field;
			unsigned long n_session = request.get_session_number();
			_mp::cws_server::csession::type_ptr_session ptr_session;
			std::wstring s_file, s_sub;

			do {
				response = request;
				response.set_cmd(cio_packet::cmd_response);
				response.set_data_error();

				ptr_session = cserver::get_instance().get_session(n_session);
				if (!ptr_session) {
					p_log->log_fmt(L"[E] - %ls | get_session() == NULL.\n", __WFUNCTION__);
					p_log->trace(L"[E] - %ls | get_session() == NULL.\n", __WFUNCTION__);
					continue;
				}
				if (request.get_data_field_type() == cio_packet::data_field_binary) {
					//append mode
					_mp::type_v_buffer v_data(0);
					if (request.get_data_field(v_data) == 0) {
						p_log->log_fmt(L"[E] - %ls | get_data_field() == empty.\n", __WFUNCTION__);
						p_log->trace(L"[E] - %ls | get_data_field() == empty.\n", __WFUNCTION__);
						continue;
					}
					if (!ptr_session->get_first_opened_file_path(s_file)) {
						p_log->log_fmt(L"[E] - %ls | get_first_opened_file_path().\n", __WFUNCTION__);
						p_log->trace(L"[E] - %ls | get_first_opened_file_path().\n", __WFUNCTION__);
						continue;
					}

					if (!ptr_session->file_write(s_file, v_data)) {
						p_log->log_fmt(L"[E] - %ls | file_write().\n", __WFUNCTION__);
						p_log->trace(L"[E] - %ls | file_write().\n", __WFUNCTION__);
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (request.get_data_field(deque_wstring_data_field) == 0) {
					p_log->log_fmt(L"[E] - %ls | get_data_field() none.\n", __WFUNCTION__);
					p_log->trace(L"[E] - %ls | get_data_field() none.\n", __WFUNCTION__);
					continue;
				}
				//
				if (deque_wstring_data_field[0].compare(L"firmware") == 0) {
					if (deque_wstring_data_field.size() != 2) {
						p_log->log_fmt(L"[E] - %ls | firmware | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						p_log->trace(L"[E] - %ls | firmware | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						continue;
					}
					s_sub = deque_wstring_data_field[1];
					std::wstring s_virtual_full_file(ccoffee_path::get_virtual_path_of_temp_rom_file_of_session(n_session));
					if (!ptr_session->file_firmware(s_sub, s_virtual_full_file)) {
						p_log->log_fmt(L"[E] - %ls | file_firmware(%ls,%ls).\n", __WFUNCTION__, s_sub.c_str(), s_virtual_full_file.c_str());
						p_log->trace(L"[E] - %ls | file_firmware(%ls,%ls).\n", __WFUNCTION__, s_sub.c_str(), s_virtual_full_file.c_str());
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"create") == 0) {
					if (deque_wstring_data_field.size() != 2) {
						p_log->log_fmt(L"[E] - %ls | create | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						p_log->trace(L"[E] - %ls | create | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						continue;
					}
					s_file = deque_wstring_data_field[1];
					if (!ptr_session->file_create(s_file)) {
						p_log->log_fmt(L"[E] - %ls | file_create(%ls).\n", __WFUNCTION__, s_file.c_str());
						p_log->trace(L"[E] - %ls | file_create(%ls).\n", __WFUNCTION__, s_file.c_str());
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"open") == 0) {
					if (deque_wstring_data_field.size() != 2) {
						p_log->log_fmt(L"[E] - %ls | open | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						p_log->trace(L"[E] - %ls | open | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						continue;
					}
					s_file = deque_wstring_data_field[1];
					if (!ptr_session->file_open(s_file)) {
						p_log->log_fmt(L"[E] - %ls | file_open(%ls).\n", __WFUNCTION__, s_file.c_str());
						p_log->trace(L"[E] - %ls | file_open(%ls).\n", __WFUNCTION__, s_file.c_str());
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"close") == 0) {
					if (deque_wstring_data_field.size() != 1) {
						p_log->log_fmt(L"[E] - %ls | close | deque_wstring_data_field.size() != 1.\n", __WFUNCTION__);
						p_log->trace(L"[E] - %ls | close | deque_wstring_data_field.size() != 1.\n", __WFUNCTION__);
						continue;
					}
					if (!ptr_session->get_first_opened_file_path(s_file)) {
						p_log->log_fmt(L"[E] - %ls | get_first_opened_file_path().\n", __WFUNCTION__);
						p_log->trace(L"[E] - %ls | get_first_opened_file_path().\n", __WFUNCTION__);
						continue;
					}
					if (!ptr_session->file_close(s_file)) {
						p_log->log_fmt(L"[E] - %ls | file_close(%ls).\n", __WFUNCTION__, s_file.c_str());
						p_log->trace(L"[E] - %ls | file_close(%ls).\n", __WFUNCTION__, s_file.c_str());
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"delete") == 0) {
					if (deque_wstring_data_field.size() != 2) {
						p_log->log_fmt(L"[E] - %ls | delete | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						p_log->trace(L"[E] - %ls | delete | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						continue;
					}
					s_file = ccoffee_path::get_path_of_virtual_drive_root_except_backslash();
#ifdef _WIN32
					s_file += L"\\";
#else
					s_file += L"/";
#endif
					s_file += deque_wstring_data_field[1];
					if (!cfile::delete_file(s_file)) {
						p_log->log_fmt(L"[E] - %ls | delete_file(%ls).\n", __WFUNCTION__, s_file.c_str());
						p_log->trace(L"[E] - %ls | delete_file(%ls).\n", __WFUNCTION__, s_file.c_str());
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"truncate") == 0) {
					if (deque_wstring_data_field.size() != 1) {
						p_log->log_fmt(L"[E] - %ls | truncate | deque_wstring_data_field.size() != 1.\n", __WFUNCTION__);
						p_log->trace(L"[E] - %ls | truncate | deque_wstring_data_field.size() != 1.\n", __WFUNCTION__);
						continue;
					}
					if (!ptr_session->get_first_opened_file_path(s_file)) {
						p_log->log_fmt(L"[E] - %ls | get_first_opened_file_path().\n", __WFUNCTION__);
						p_log->trace(L"[E] - %ls | get_first_opened_file_path().\n", __WFUNCTION__);
						continue;
					}
					if (!ptr_session->file_truncate(s_file)) {
						p_log->log_fmt(L"[E] - %ls | file_truncate(%ls).\n", __WFUNCTION__, s_file.c_str());
						p_log->trace(L"[E] - %ls | file_truncate(%ls).\n", __WFUNCTION__, s_file.c_str());
						continue;
					}
					//
					response.set_data_sucesss();
					continue;
				}

				if (deque_wstring_data_field[0].compare(L"get") == 0) {
					if (deque_wstring_data_field.size() != 2) {
						p_log->log_fmt(L"[E] - %ls | delete | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						p_log->trace(L"[E] - %ls | delete | deque_wstring_data_field.size() != 2.\n", __WFUNCTION__);
						continue;
					}
					if (deque_wstring_data_field[1].compare(L"size") == 0) {
						if (!ptr_session->get_first_opened_file_path(s_file)) {
							p_log->log_fmt(L"[E] - %ls | get_first_opened_file_path().\n", __WFUNCTION__);
							p_log->trace(L"[E] - %ls | get_first_opened_file_path().\n", __WFUNCTION__);
							continue;
						}
						int n_size = ptr_session->file_get_size(s_file);
						if (n_size < 0) {
							p_log->log_fmt(L"[E] - %ls | file_get_size(%ls) = %d.\n", __WFUNCTION__, s_file.c_str(), n_size);
							p_log->trace(L"[E] - %ls | file_get_size(%ls) = %d.\n", __WFUNCTION__, s_file.c_str(), n_size);
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
						cfile::get_find_file_list(list_found, s_root, L"*", cfile::folder_area_current_folder, false);

						//extraction file name & extention from result.
						response.set_data_by_utf8(L"success", false);

						for (type_list_wstring::iterator it = std::begin(list_found); it != std::end(list_found); ++it) {
							*it = cfile::get_file_name_and_extention_only_from_path(*it);
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

		bool cmain_ctl_fn::_execute_advance_operation(clog* p_log, cio_packet& request, cio_packet& response, cio_packet& response_for_the_other_session)
		{
			_mp::type_deque_wstring deque_wstring_data_field;
			unsigned long n_session = request.get_session_number();
			_mp::cws_server::csession::type_ptr_session ptr_session;
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
						s_session_name = cctl_svr::get_instance().generate_session_name(n_session);//auto generated session name.
					else
						s_session_name = deque_wstring_data_field[1];
					if (!cctl_svr::get_instance().set_session_name(n_session, s_session_name)) {
						continue;//already session has a name.
					}
					//
					response.set_data_sucesss();
					continue;
				}
				if (deque_wstring_data_field[0].compare(L"get_session_name") == 0) {
					if (deque_wstring_data_field.size() != 1)
						continue;
					s_session_name = cctl_svr::get_instance().get_session_name(n_session);
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
					unsigned long n_session_dest(cctl_svr::get_instance().get_session_number_from_session_name(s_session_name));
					if (n_session_dest == _MP_TOOLS_INVALID_SESSION_NUMBER)
						continue;
					if (n_session == n_session_dest)
						continue;//source and destination cannot be equal.

					s_session_name = cctl_svr::get_instance().get_session_name(n_session);
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
					s_session_name = cctl_svr::get_instance().get_session_name(n_session);
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

}//the end of _mp namespace

