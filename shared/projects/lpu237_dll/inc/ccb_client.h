#pragma once

#include <mp_clog.h>
#include <mp_casync_result_manager.h>
#include <mp_cconvert.h>

#include <cprotocol_lpu237.h>
#include <manager_of_device_of_client.h>

#include <lpu237_of_client.h>

#ifdef _WIN32
#include <atltrace.h>
#endif

class ccb_client
{
public:
	//
	enum : unsigned long {
		const_dll_result_success = 0,// = LPU237_DLL_RESULT_SUCCESS and LPU237LOCK_DLL_RESULT_SUCCESS, LPU237_FW_RESULT_SUCCESS
		const_dll_result_error = 0xFFFFFFFF, //(-1) = LPU237_DLL_RESULT_ERROR and LPU237LOCK_DLL_RESULT_ERROR, LPU237_FW_RESULT_ERROR
		const_dll_result_cancel = 0xFFFFFFFE, //(-2) = LPU237_DLL_RESULT_CANCEL and LPU237LOCK_DLL_RESULT_CANCEL, LPU237_FW_RESULT_CANCEL
		const_dll_result_icc_inserted = 0xFFFFFFFC, //(-4) = LPU237_DLL_RESULT_ICC_INSERTED
		const_dll_result_icc_removed = 0xFFFFFFFB //(-5) = LPU237_DLL_RESULT_ICC_REMOVED
	};

	enum : unsigned long {//only used in tg_lpu237_fw.dll(libtg_lpu237_fw.so)
		const_dll_result_timeout = 0xFFFFFFFC, //(-4) = LPU237_FW_RESULT_TIMEOUT
		const_dll_result_no_msr = 0xFFFFFFFB //(-5) = LPU237_FW_RESULT_NO_MSR
	};

public://this class must be have a static member only.
	static _mp::cclient_cb::type_cb_callbacks& get_callbacks()
	{
		static _mp::cclient_cb::type_cb_callbacks callbacks =
		{
			ccb_client::_resolve,
			NULL,
			ccb_client::_connect,
			NULL,
			ccb_client::_handshake,
			NULL,
			ccb_client::_handshake_ssl,
			NULL,
			ccb_client::_read,
			NULL,
			ccb_client::_write,
			NULL,
			ccb_client::_close,
			NULL
		};

		return callbacks;
	}
	//static 

private:
	static void __stdcall _resolve(unsigned long n_result, void* p_user)
	{
		do {
			if (n_result != _mp::cclient::RESULT_SUCCESS) {
				continue;
			}

		} while (false);
	}
	static void __stdcall _connect(unsigned long n_result, void* p_user)
	{
		do {
			if (n_result != _mp::cclient::RESULT_SUCCESS) {
				continue;
			}

		} while (false);
	}
	static void __stdcall _handshake(unsigned long n_result, void* p_user)
	{
		do {
			if (n_result != _mp::cclient::RESULT_SUCCESS) {
				continue;
			}

		} while (false);
	}
	static void __stdcall _handshake_ssl(unsigned long n_result, void* p_user)
	{
		do {
			if (n_result != _mp::cclient::RESULT_SUCCESS) {
				continue;
			}

		} while (false);
	}
	static void __stdcall _read(
		char c_action_code
		, unsigned long n_result
		, unsigned long n_device_index
		, unsigned char c_in_id
		, void* p_user
		, unsigned long n_rx
		, const unsigned char* s_rx
	)
	{
		bool b_result(false);
		std::wstring s_deb;
		std::list<std::wstring> list_s_deb;
		long n_fw_step_cur(-1), n_fw_step_max(-1);
		wchar_t* endptr = nullptr;

		do {
			unsigned long n_result_code(ccb_client::const_dll_result_success);

			switch (n_result) {
			case _mp::cclient::RESULT_SUCCESS:
				b_result = true;
				break;
			case _mp::cclient::RESULT_CANCEL:
				n_result_code = ccb_client::const_dll_result_cancel;
				break;
			case _mp::cclient::RESULT_ERROR:
			default:
				n_result_code = ccb_client::const_dll_result_error;;
				break;
			}//end switch
			
			int n_result_index(-1);

			//received data for binary type.
			_mp::type_v_buffer v_rx(0);
			std::set<std::wstring> set_s_out;//received data for multi string.
			std::vector<std::wstring> v_s_out;//received data for multi string.
			std::wstring s_result;//received data for single string.

			if (n_rx > 0 && s_rx != NULL) {
				v_rx.assign(&s_rx[0], &s_rx[n_rx]);

				//try converting binary data to multi string.
				if (capi_client::help_strings_from_response(set_s_out, v_rx) == 1) {
					//rx data single string
					s_result = *std::begin(set_s_out);
				}
				//adjust result code.
				if (b_result && v_rx.size() >= 3) {
					unsigned char c_var = cprotocol_lpu237::msr_warning_icc_inserted;
					if (v_rx[0] == c_var && v_rx[1] == c_var && v_rx[2] == c_var) {
						n_result_code = ccb_client::const_dll_result_icc_inserted;
					}
					else {
						c_var = cprotocol_lpu237::msr_warning_icc_removed;
						if (v_rx[0] == c_var && v_rx[1] == c_var && v_rx[2] == c_var) {
							n_result_code = ccb_client::const_dll_result_icc_removed;
						}
					}
				}

				capi_client::help_strings_from_response(v_s_out, v_rx);
			}
			//
			capi_client::type_action last_action(capi_client::get_instance().get_last_action(n_device_index));
			std::wstring s_last_run_type(capi_client::get_instance().get_last_run_type_string(n_device_index));
			std::wstring s_last_key(capi_client::get_instance().get_last_key_string(n_device_index));
			std::wstring s_last_value(capi_client::get_instance().get_last_value_string(n_device_index));

			if (c_action_code == 'o') {
				//open request.
				last_action = capi_client::get_instance().get_last_action(0);
			}

#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"^^^^^^ %ls : %d(n_rx = %u).\n", __WFUNCTION__, last_action, n_rx);
#endif
			_mp::casync_parameter_result::type_ptr_ct_async_parameter_result ptr_result_for_boot;
			bool b_remove_result_index_with_pop(true);

			switch (last_action)
			{
			case capi_client::act_create:
				break;
			case capi_client::act_destory:
				break;
			case capi_client::act_connect:
				break;
			case capi_client::act_disconnect:
				break;
			case capi_client::act_echo:
				break;
			case capi_client::act_get_device_list:
				if (!manager_of_device_of_client<lpu237_of_client>::get_instance())
					break;
				n_result_index = manager_of_device_of_client<lpu237_of_client>::get_instance()->pop_async_parameter_result_index_for_manager();
				if (n_result_index >= 0) {
					_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_result = _mp::casync_result_manager::get_instance(std::wstring(L"manager_of_device_of_client")).get_async_parameter_result(n_device_index, n_result_index);
					ptr_result->set_result(b_result, set_s_out);
					ptr_result->notify();
				}
				break;
			case capi_client::act_open:
				n_result_index = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).pop_result_index(i_device_of_client::const_invalied_device_index);
				if (n_result_index >= 0) {
					_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_result = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).get_async_parameter_result(i_device_of_client::const_invalied_device_index, n_result_index);
					ptr_result->set_result(b_result, n_device_index);
					ptr_result->notify();
				}

				break;
			case capi_client::act_close:
				n_result_index = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).pop_result_index(n_device_index);
				if (n_result_index >= 0) {
					_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_result = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).get_async_parameter_result(n_device_index, n_result_index);
					ptr_result->set_result(b_result, n_result_code, _mp::type_v_buffer(0));
					ptr_result->notify();
				}
				break;
			case capi_client::act_write:
				n_result_index = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).pop_result_index(n_device_index);
				if (n_result_index >= 0) {
					_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_result = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).get_async_parameter_result(n_device_index, n_result_index);
					ptr_result->set_result(b_result, n_result_code, _mp::type_v_buffer(0));
					ptr_result->notify();
				}
				break;
			case capi_client::act_read:
				n_result_index = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).pop_result_index(n_device_index);
				if (n_result_index >= 0) {
					_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_result = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).get_async_parameter_result(n_device_index, n_result_index);
					ptr_result->set_result(b_result, n_result_code, v_rx);
					ptr_result->notify();
				}
				break;
			case capi_client::act_transmit:
				n_result_index = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).pop_result_index(n_device_index);
				if (n_result_index >= 0) {
					_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_result = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).get_async_parameter_result(n_device_index, n_result_index);
					ptr_result->set_result(b_result, n_result_code, v_rx);
					ptr_result->notify();
				}
				break;
			case capi_client::act_cancel:
				n_result_index = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).pop_result_index(n_device_index);
				if (n_result_index >= 0) {
					_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_result = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).get_async_parameter_result(n_device_index, n_result_index);
					ptr_result->set_result(b_result, n_result_code, _mp::type_v_buffer(0));
					ptr_result->notify();
				}
				break;
			case capi_client::act_sub_bootloader:
#if defined(_WIN32) && defined(_DEBUG)
				if (s_last_run_type.empty()) {
					ATLTRACE(L"~~~~~ %ls : act_sub_bootloader : s_last_run_type is empty.\n", __WFUNCTION__);
				}
				else {
					ATLTRACE(L"~~~~~ %ls : act_sub_bootloader : s_last_run_type = %ls.\n", __WFUNCTION__, s_last_run_type.c_str());
				}
				if (s_last_key.empty()) {
					if (s_last_value.empty()) {
						ATLTRACE(L"~~~~~ %ls : act_sub_bootloader : s_last_key is empty and s_last_value is empty.\n", __WFUNCTION__);
					}
					else {
						ATLTRACE(L"~~~~~ %ls : act_sub_bootloader : s_last_key is empty and s_last_value = %ls.\n", __WFUNCTION__, s_last_value.c_str());
					}
				}
				else {
					if (s_last_value.empty()) {
						ATLTRACE(L"~~~~~ %ls : act_sub_bootloader : s_last_key = %ls and s_last_value is empty.\n", __WFUNCTION__, s_last_key.c_str());
					}
					else {
						ATLTRACE(L"~~~~~ %ls : act_sub_bootloader : s_last_key = %ls and s_last_value = %ls.\n", __WFUNCTION__, s_last_key.c_str(), s_last_value.c_str());
					}
				}

				for (const auto& s : v_s_out) {
					ATLTRACE(L"~~~~~ ::::: %ls\n", s.c_str());
				}
#endif
				if (s_last_run_type.compare(L"start") == 0) {
					if (set_s_out.find(L"success") != set_s_out.end()) {
						// start 일 때는 fw 진행이므로, 정상 처리 응답에 해당하는 값인 "success" 가 있으면 제거해서는 안된다. 
						b_remove_result_index_with_pop = false;

						//
						if (v_s_out.size() >= 3) {
							// 처리 중인 fw update 단계(cur, max) 값이 있으므로, 값을 얻느다.
							// fw update 의 현재 처리 단계 값을 얻음.( zero base 값이므로 허용되는 값은 0 ~ n_fw_step_max-1 )
							if (_mp::cconvert::get_value(n_fw_step_cur, v_s_out[1], 10)) {
								// fw update 의 총 처리 단계 값을 얻음.
								if (_mp::cconvert::get_value(n_fw_step_max, v_s_out[2], 10)) {
									if (n_fw_step_cur >= (n_fw_step_max - 1)) {
										// 모두 정상으로 끝남.
										// 주의. (n_fw_step_max-1) == n_fw_step_cur 인 message 가 두번 오는데, 마지막 것은 무시해도 됨.
										// 끝난 것을 한 번 더 다름 메시지와 함께 보냄.
										// 이제 result index 를 제거해야 한다
										b_remove_result_index_with_pop = true;
									}
								}
								else {
									n_fw_step_cur = n_fw_step_max = -1;
								}
							}
							else {
								n_fw_step_cur = -1;
							}
						}
					}
				}

				//ptr_result_for_boot->notify() 호출 전에,
				// pop_result_index() 를 하면서 result index 를 제거 할지 결정 하기 위해서,
				// 위에서 처럼 b_remove_result_index_with_pop 값을 설정한다.
				n_result_index = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).pop_result_index(n_device_index, b_remove_result_index_with_pop);
				if (n_result_index < 0) {
					// 에러(n_result_index 없음)나서 계속 사용 중지.
#if defined(_WIN32) && defined(_DEBUG)
					ATLTRACE(L"~~~~~ +++++ none result index of device index %u.\n", n_device_index);
#endif
					break;//exit switch
				}
				ptr_result_for_boot = _mp::casync_result_manager::get_instance(std::wstring(L"lpu237_of_client")).get_async_parameter_result(n_device_index, n_result_index);
				ptr_result_for_boot->reset_notify_event(); // 재사용을 위한 조건 설정.
				ptr_result_for_boot->set_result(b_result, n_result_code);
				ptr_result_for_boot->set_result(b_result, v_s_out);
				ptr_result_for_boot->notify();
				break;
			default:
#if defined(_WIN32) && defined(_DEBUG)
				ATLTRACE(L"~~~~~ %ls : %d.\n", __WFUNCTION__, last_action);
#endif
				break;
			}//end switch

		} while (false);
	}
	static void __stdcall _write(unsigned long n_result, void* p_user)
	{
		do {
			if (n_result != _mp::cclient::RESULT_SUCCESS) {
				continue;
			}

		} while (false);
	}
	static void __stdcall _close(unsigned long n_result, void* p_user)
	{
		do {
			if (n_result != _mp::cclient::RESULT_SUCCESS) {
				_mp::clog::get_instance().trace(L" : ccb_client::close - error\n");
				continue;
			}
			_mp::clog::get_instance().trace(L" : ccb_client::close - success\n");

		} while (false);
	}

private://don' call these methods
	ccb_client();
	~ccb_client();

	ccb_client(const ccb_client&);
	ccb_client& operator=(const ccb_client&);

};

