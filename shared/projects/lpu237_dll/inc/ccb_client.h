#pragma once

#include <mp_clog.h>
#include <mp_casync_result_manager.h>

#include <cprotocol_lpu237.h>
#include <tg_lpu237_dll.h>

#include <manager_of_device_of_client.h>

#include <lpu237_of_client.h>

class ccb_client
{
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
	static void __stdcall _read(char c_action_code, unsigned long n_result, unsigned long n_device_index, unsigned char c_in_id, void* p_user, unsigned long n_rx, const unsigned char* s_rx)
	{
		bool b_result(false);

		do {
			if (n_result == _mp::cclient::RESULT_SUCCESS) {
				b_result = true;
			}

			unsigned long n_result_code(LPU237_DLL_RESULT_SUCCESS);
			if (!b_result) {
				n_result_code = LPU237_DLL_RESULT_ERROR;
			}

			int n_result_index(-1);

			//received data for binary type.
			_mp::type_v_buffer v_rx(0);
			std::set<std::wstring> set_s_out;//received data for multi string.
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
						n_result_code = LPU237_DLL_RESULT_ICC_INSERTED;
					}
					else {
						c_var = cprotocol_lpu237::msr_warning_icc_removed;
						if (v_rx[0] == c_var && v_rx[1] == c_var && v_rx[2] == c_var) {
							n_result_code = LPU237_DLL_RESULT_ICC_REMOVED;
						}
					}
				}
			}
			//
			capi_client::type_action last_action(capi_client::get_instance().get_last_action(n_device_index));
			if (c_action_code == 'o') {
				//open request.
				last_action = capi_client::get_instance().get_last_action(0);
			}

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
			default:
				break;
			}

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

