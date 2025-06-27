#pragma once

#include <string>
#include <vector>
#include <list>
#include <thread>
#include <future> 
#include <typeinfo>  
#include <set>
#include <functional>
#include <map>
#include <utility>

#include <mp_type.h>
#include <server/mp_cclient_manager.h>

class capi_client
{
public:
	typedef	enum {
		act_none = 0,
		act_create,
		act_destory,
		act_connect,
		act_disconnect,
		act_echo,
		act_echo_list_string,
		act_ctl_show,
		act_get_device_list,
		act_open,
		act_close,
		act_write,
		act_read,
		act_transmit,
		act_cancel,
		act_advance_set_session_name,
		act_advance_get_session_name,
		act_advance_send_data_to_session,
		act_advance_send_data_to_all_session,
		act_kernel
	} type_action;

	typedef	std::function< void(const wchar_t*) > type_this_class_log_function;

private:
	typedef	std::pair< capi_client::type_action, unsigned long>	_type_pair_action_result;
	typedef	std::map<unsigned long, capi_client::_type_pair_action_result>	_type_map_device_index_pair_action_result;

	typedef	std::shared_ptr< _mp::cclient_cb::type_cb_callbacks > _type_ptr_cb_callbacks;
	typedef	std::map< unsigned long, capi_client::_type_ptr_cb_callbacks >	_type_map_client_index_ptr_cb_callbacks;

	typedef	std::promise<bool>						_type_promise_bool;
	typedef	std::shared_ptr<_type_promise_bool>		_type_ptr_promise_bool;

	typedef	std::promise<unsigned long>				_type_promise_dword;
	typedef	std::shared_ptr<_type_promise_dword>	_type_ptr_promise_dword;

	static unsigned char& _get_in_id_for_sync()
	{
		static unsigned char c_in_id(0);
		return c_in_id;
	}
	static unsigned char& _get_out_id_for_sync()
	{
		static unsigned char c_out_id(0);
		return c_out_id;
	}

	static unsigned long& _get_device_index_for_sync()
	{
		static unsigned long n_device_index(0);
		return n_device_index;
	}
	static std::vector<unsigned char>& _get_rx_for_sync()
	{
		static std::vector<unsigned char> v_rx_for_sync;
		return v_rx_for_sync;
	}

	static _type_ptr_promise_dword& _get_promise_dword_rx_for_sync()
	{
		static _type_ptr_promise_dword ptr_promise_dword_for_sync;
		return ptr_promise_dword_for_sync;
	}
	static _type_ptr_promise_bool& _get_promise_bool_tx_for_sync()
	{
		static _type_ptr_promise_bool ptr_promise_bool_for_sync;
		return ptr_promise_bool_for_sync;
	}

	//local callback for sync type function
	static void __stdcall _before_handshake_cb(unsigned long n_result, void* p_user)
	{
		//dummy cb
		//capi_client::_get_promise_dword_rx_for_sync()->set_value(n_result);
	}
	static void __stdcall _handshake_cb(unsigned long n_result, void* p_user)
	{
		capi_client::_get_promise_dword_rx_for_sync()->set_value(n_result);
	}
	static void __stdcall _read_cb(char c_act_code, unsigned long n_result, unsigned long n_device_index, unsigned char c_in_id, void* p_user, unsigned long n_rx, const unsigned char* s_rx)
	{
		unsigned long dw_result(n_result);

		do {
			capi_client::_get_device_index_for_sync() = n_device_index;
			capi_client::_get_in_id_for_sync() = c_in_id;

			if (dw_result == _mp::cclient::RESULT_ERROR)
				continue;
			if (dw_result == _mp::cclient::RESULT_CANCEL)
				continue;
			if (n_rx == 0 || s_rx == NULL)
				continue;
			capi_client::_get_rx_for_sync().assign(&s_rx[0], &s_rx[n_rx]);
		} while (false);
		capi_client::_get_promise_dword_rx_for_sync()->set_value(dw_result);
	}
	static void __stdcall _write_cb(unsigned long n_result, void* p_user)
	{
		bool b_result(false);
		do {
			if (n_result != _mp::cclient::RESULT_SUCCESS) {
				continue;
			}
			b_result = true;
		} while (false);
		capi_client::_get_promise_bool_tx_for_sync()->set_value(b_result);
	}

	void _sync_before(unsigned long n_client_index, unsigned long n_device_index, unsigned char c_in_id, char unsigned c_out_id);
	void _sync_after(unsigned long n_client_index, unsigned long& n_device_index, unsigned char& c_in_id, std::vector<unsigned char>& v_rx);

public:
	static capi_client& get_instance()
	{
		static capi_client obj;
		return obj;
	}

	~capi_client();

	void unload();

	unsigned long get_last_result(unsigned long n_device_index) const;
	capi_client::type_action get_last_action(unsigned long n_device_index) const;
	void set_last_action_and_result(unsigned long n_device_index, const capi_client::type_action last_action, const unsigned long dw_last_result);

	unsigned long create_before_set_callback
	(
		long long ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete
		, long long ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss
		, long long ll_msec_timeout_ws_client_wait_for_idle_in_wss
		, long long ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss
	);
	bool destory_after_stop(unsigned long n_client_index);

	bool start_after_set_callback(unsigned long n_client_index, bool b_tls);
	bool start_after_set_callback_sync(unsigned long n_client_index, bool b_tls, long n_msec_wait_time = -1);
	bool stop_before_destory(unsigned long n_client_index);

	bool echo(unsigned long n_client_index, const std::vector<unsigned char>& v_tx);
	bool echo_sync(std::vector<unsigned char>& v_rx, unsigned long n_client_index, const std::vector<unsigned char>& v_tx);


	bool echo_list_string(unsigned long n_client_index, const std::list<std::wstring>& list_string);

	bool echo_list_string_sync(std::list<std::wstring>& list_response, unsigned long n_client_index, const std::list<std::wstring>& list_string);

	bool ctl_show(unsigned long n_client_index, bool b_show = true);

	bool ctl_show_sync(unsigned long n_client_index, bool b_show = true);

	bool get_device_list(unsigned long n_client_index, const std::wstring& s_filter = std::wstring());

	bool get_device_list_multi(unsigned long n_client_index, const std::list<std::wstring>& list_s_filter = std::list<std::wstring>());

	bool get_device_list_sync(std::set<std::wstring>& set_s_path, unsigned long n_client_index, const std::wstring& s_filter = std::wstring());

	bool get_device_list_multi_sync(std::set<std::wstring>& set_s_path, unsigned long n_client_index, const std::list<std::wstring>& list_s_filter = std::list<std::wstring>());


	bool open(unsigned long n_client_index, const std::wstring& s_device_path,bool b_shared_mode);

	bool open_sync(unsigned long& n_out_device_index, unsigned long n_client_index, const std::wstring& s_device_path, bool b_shared_mode);
	bool close(unsigned long n_client_index, unsigned long n_device_index);
	bool close_sync(unsigned long n_client_index, unsigned long n_device_index);
	bool write(unsigned long n_client_index, unsigned long n_device_index, unsigned char c_out_id, const std::vector<unsigned char>& v_data);
	bool write_sync(unsigned long n_client_index, unsigned long n_device_index, unsigned char c_out_id, const std::vector<unsigned char>& v_tx);
	bool read(unsigned long n_client_index, unsigned long n_device_index, unsigned char c_in_id);
	bool read_sync(std::vector<unsigned char>& v_rx, unsigned long n_client_index, unsigned long n_device_index, unsigned char c_in_id);
	bool transmit(unsigned long n_client_index, unsigned long n_device_index, unsigned char c_in_id, unsigned char c_out_id, const std::vector<unsigned char>& v_data);
	bool transmit_sync(std::vector<unsigned char>& v_rx, unsigned long n_client_index, unsigned long n_device_index, unsigned char c_in_id, unsigned char c_out_id, const std::vector<unsigned char>& v_tx);
	bool cancel(unsigned long n_client_index, unsigned long n_device_index);
	bool cancel_sync(unsigned long n_client_index, unsigned long n_device_index);

	bool advance_set_session_name(unsigned long n_client_index, const std::wstring& s_session_name);
	bool advance_set_session_name_sync(unsigned long& n_out_device_index, unsigned long n_client_index, const std::wstring& s_session_name);

	bool advance_get_session_name(unsigned long n_client_index);
	bool advance_get_session_name_sync(std::wstring& s_out_session_name, unsigned long n_client_index);

	bool advance_send_data_to_session(unsigned long n_client_index, const std::wstring& s_session_name, const std::list<std::wstring>& list_string);

	bool advance_send_data_to_session_sync(std::list<std::wstring>& list_response, unsigned long n_client_index, const std::wstring& s_session_name, const std::list<std::wstring>& list_string);

	bool advance_send_data_to_all_session(unsigned long n_client_index, const std::list<std::wstring>& list_string);

	bool advance_send_data_to_all_session_sync(std::list<std::wstring>& list_response, unsigned long n_client_index, const std::list<std::wstring>& list_string);

	bool kernel(unsigned long n_client_index, unsigned long n_device_index, const std::list<std::wstring>& list_string);

	bool kernel_sync(std::list<std::wstring>& list_response, unsigned long n_client_index, unsigned long n_device_index, const std::list<std::wstring>& list_string);

	void set_callbacks(unsigned long n_client_index, const _mp::cclient_cb::type_cb_callbacks& cbs);
	void set_callback_resolve(unsigned long n_client_index, const _mp::cclient_cb::type_cb_resolve cb, void* p_user);
	void set_callback_connect(unsigned long n_client_index, const _mp::cclient_cb::type_cb_connect cb, void* p_user);
	void set_callback_handshake(unsigned long n_client_index, const _mp::cclient_cb::type_cb_handshake cb, void* p_user);
	void set_callback_handshake_ssl(unsigned long n_client_index, const _mp::cclient_cb::type_cb_handshake_ssl cb, void* p_user);
	void set_callback_read(unsigned long n_client_index, const _mp::cclient_cb::type_cb_read cb, void* p_user);
	void set_callback_write(unsigned long n_client_index, const _mp::cclient_cb::type_cb_write cb, void* p_user);
	void set_callback_close(unsigned long n_client_index, const _mp::cclient_cb::type_cb_close cb, void* p_user);

	unsigned long send_to_server(unsigned long n_client_index, unsigned char* s_tx, unsigned long n_tx);
	//helper function
	static size_t help_strings_from_response(std::set<std::wstring>&set_s_dst, unsigned long n_receive, const unsigned char* s_receive)
	{
		std::wstring stemp;

		do {
			set_s_dst.clear();

			if (s_receive == NULL)
				continue;
			if (n_receive == 0)
				continue;

			for (unsigned long i = 0; i < n_receive; i++) {
				if (std::isprint(s_receive[i]) == 0) {
					if (s_receive[i] != 0x00) {
						set_s_dst.clear();
						break;//error exit for
					}
					//null terminatred string
					if (stemp.empty())
						continue;
					
					set_s_dst.insert(capi_client::_remove_colron_doubling(stemp));//"::" -> ":"
					stemp.clear();
					continue;
				}

				stemp.push_back((std::wstring::value_type)s_receive[i]);
			}//end for
			//
		} while (false);
		return set_s_dst.size();
	}

	static size_t help_strings_from_response(std::set<std::wstring>&set_s_dst, const std::vector<unsigned char>&v_receive)
	{
		size_t n_item(0);
		do {
			set_s_dst.clear();

			if (v_receive.empty())
				continue;

			std::set<std::wstring> set_t;
			n_item = capi_client::_get_string_set_from_multi_string(set_t, (unsigned long)(v_receive.size()), &v_receive[0]);
			if (n_item > 0) {
				for (auto item : set_t) {
					set_s_dst.insert(capi_client::_remove_colron_doubling(item));//"::" -> ":"
				}
			}

		} while (false);
		return n_item;
	}

	static size_t help_strings_from_response(std::list<std::wstring>&list_s_dst, const std::vector<unsigned char>&v_receive)
	{
		size_t n_item(0);
		do {
			list_s_dst.clear();

			if (v_receive.empty())
				continue;

			std::list<std::wstring> list_t;
			n_item = capi_client::_get_string_list_from_multi_string(list_t, (unsigned long)(v_receive.size()), &v_receive[0]);
			if (n_item > 0) {
				for (auto item : list_t) {
					list_s_dst.push_back(capi_client::_remove_colron_doubling(item));//"::" -> ":"
				}
			}

		} while (false);
		return n_item;
	}

	static bool help_reponse_contain_success(const std::vector<unsigned char>&v_receive, bool b_received_string_is_one = true)
	{
		std::set<std::wstring> set_s_dst;
		std::wstring s_success(L"success");
		std::set<std::wstring>::iterator it;

			bool b_result(false);

		do {
			size_t n_string = capi_client::help_strings_from_response(set_s_dst, v_receive);
			if (n_string != 1 && b_received_string_is_one)
				continue;

			it = set_s_dst.find(s_success);
			if (it == std::end(set_s_dst))
				continue;

			b_result = true;
		} while (false);
		return b_result;
	}

private:
	capi_client();

	/**
	* change "::" to ":" in s_src
	*/
	static std::wstring _remove_colron_doubling(const std::wstring& s_src)
	{
		std::wstring s_dst;

		int n_col = 0;//colron counter
		for (auto c_item : s_src) {
			if (c_item == L':') {
				if (n_col != 0) {
					n_col = 0;
					continue;//bypass ':'
				}
				++n_col;
			}
			else {
				n_col = 0;
			}
			s_dst.push_back(c_item);
		}//end for
		return s_dst;
	}
	static size_t _get_string_set_from_multi_string(std::set<std::wstring>&set_s_dst, unsigned long n_src, const unsigned char* s_src)
	{
		std::wstring stemp;

		do {
			set_s_dst.clear();

			if (s_src == NULL)
				continue;
			if (n_src == 0)
				continue;

			for (unsigned long i = 0; i < n_src; i++) {
				if (std::isprint(s_src[i]) == 0) {
					if (s_src[i] != 0x00) {
						set_s_dst.clear();
						break;//error exit for
					}
					//null terminatred string
					if (stemp.empty())
						continue;
					set_s_dst.insert(stemp);
					stemp.clear();
					continue;
				}

				stemp.push_back((std::wstring::value_type)s_src[i]);
			}//end for
		//
		} while (false);
		return set_s_dst.size();
	}
	static size_t _get_string_list_from_multi_string(std::list<std::wstring>&list_s_dst, unsigned long n_src, const unsigned char* s_src)
	{
		std::wstring stemp;

		do {
			list_s_dst.clear();

			if (s_src == NULL)
				continue;
			if (n_src == 0)
				continue;

			for (unsigned long i = 0; i < n_src; i++) {
				if (std::isprint(s_src[i]) == 0) {
					if (s_src[i] != 0x00) {
						list_s_dst.clear();
						break;//error exit for
					}
					//null terminatred string
					if (stemp.empty())
						continue;
					list_s_dst.push_back(stemp);
					stemp.clear();
					continue;
				}

				stemp.push_back((std::wstring::value_type)s_src[i]);
			}//end for
		//
		} while (false);
		return list_s_dst.size();
	}

	/*
	* p_ss_multi_dst can be null.
	* if p_ss_multi_dst is null, return the size of multi-string.( included separator null & string end mark( double nulls )
	* and unit is byte.
	* if p_ss_multi_dst isn't null, return the number of string in p_ss_multi_dst buffer.
	*/
	static size_t _get_multi_string_from_list(wchar_t* p_ss_multi_dst, const std::list<std::wstring>&list_s_src)
	{
		size_t n_string(0);
		do {

			size_t n_size = 0; //unit byte
			if (list_s_src.empty())
				continue;

			n_string = list_s_src.size();

			std::for_each(std::begin(list_s_src), std::end(list_s_src), [&](const std::wstring path) {
				n_size += ((path.size() + 1) * sizeof(wchar_t));
				});

			n_size += sizeof(wchar_t);	//add multi null size

			if (p_ss_multi_dst == NULL) {
				n_string = n_size;	//return only need buffer size( BYTE unit, including NULL & NULLs )
				continue;
			}

			std::for_each(std::begin(list_s_src), std::end(list_s_src), [&](const std::wstring& s_str) {

				for (size_t i = 0; i < s_str.length(); i++) {
					p_ss_multi_dst[i] = s_str[i];
				}//end for
				p_ss_multi_dst[s_str.length()] = L'\0';

				p_ss_multi_dst = &p_ss_multi_dst[s_str.length() + 1];
				});

			*p_ss_multi_dst = L'\0'; //make multi string

		} while (false);
		return n_string;
	}


	void _create_cb_buffer(unsigned long n_client_index);
	void _set_callbacks(unsigned long n_client_index, const _mp::cclient_cb::type_cb_callbacks& cbs);
	void _set_callback_resolve(unsigned long n_client_index, const _mp::cclient_cb::type_cb_resolve cb, void* p_user);
	void _set_callback_connect(unsigned long n_client_index, const _mp::cclient_cb::type_cb_connect cb, void* p_user);
	void _set_callback_handshake(unsigned long n_client_index, const _mp::cclient_cb::type_cb_handshake cb, void* p_user);
	void _set_callback_handshake_ssl(unsigned long n_client_index, const _mp::cclient_cb::type_cb_handshake_ssl cb, void* p_user);
	void _set_callback_read(unsigned long n_client_index, const _mp::cclient_cb::type_cb_read cb, void* p_user);
	void _set_callback_write(unsigned long n_client_index, const _mp::cclient_cb::type_cb_write cb, void* p_user);
	void _set_callback_close(unsigned long n_client_index, const _mp::cclient_cb::type_cb_close cb, void* p_user);

	void _ini();

	/*
	* p_ss_multi_dst can be null.
	* if p_ss_multi_dst is null, return the size of multi-string.( included separator null & string end mark( double nulls )
	* and unit is byte.
	* if p_ss_multi_dst isn't null, return the number of string in p_ss_multi_dst buffer.
	*/
	static size_t _change(wchar_t* p_ss_multi_dst, const std::list<std::wstring>&list_s_src)
	{
		size_t n_string(0);
		do {

			size_t n_size = 0; //unit byte
			if (list_s_src.empty())
				continue;

			n_string = list_s_src.size();

			std::for_each(std::begin(list_s_src), std::end(list_s_src), [&](const std::wstring path) {
				n_size += ((path.size() + 1) * sizeof(wchar_t));
				});

			n_size += sizeof(wchar_t);	//add multi null size

			if (p_ss_multi_dst == NULL) {
				n_string = n_size;	//return only need buffer size( BYTE unit, including NULL & NULLs )
				continue;
			}

			std::for_each(std::begin(list_s_src), std::end(list_s_src), [&](const std::wstring& s_str) {

				for (size_t i = 0; i < s_str.length(); i++) {
					p_ss_multi_dst[i] = s_str[i];
				}//end for
				p_ss_multi_dst[s_str.length()] = L'\0';

				p_ss_multi_dst = &p_ss_multi_dst[s_str.length() + 1];
				});

			*p_ss_multi_dst = L'\0'; //make multi string

		} while (false);
		return n_string;
	}

private:
	std::mutex m_mutex_for_sync;
	capi_client::_type_map_client_index_ptr_cb_callbacks m_map_cur_callbacks;

	capi_client::_type_map_device_index_pair_action_result m_map_device_index_pair_action_result;


	capi_client::type_action m_last_action;
	unsigned long m_dw_last_result;
private://don't call these methods
	capi_client(const capi_client&);
	capi_client& operator=(const capi_client&);
};
