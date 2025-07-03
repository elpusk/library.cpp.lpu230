#pragma once

#include <map>
#include <mutex>

#include <mp_type.h>
#include <mp_clog.h>
#include <i_device_of_client.h>

#ifdef _WIN32
#ifdef _DEBUG
//#undef __THIS_FILE_ONLY__
#define __THIS_FILE_ONLY__
#include <atltrace.h>
#endif
#endif

template <typename _T_SUB_DEVICE_OF_CLIENT>
class manager_of_device_of_client
{
private:
	typedef std::shared_ptr< _T_SUB_DEVICE_OF_CLIENT >	type_ptr_device_of_client;
	typedef	std::map<unsigned long, manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>::type_ptr_device_of_client > _type_map_device_index_ptr_device_of_client;
public:
	typedef	std::shared_ptr<manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>>	type_ptr_manager_of_device_of_client;

public:
	static type_ptr_manager_of_device_of_client get_instance(bool b_remove = false)
	{
		static type_ptr_manager_of_device_of_client ptr_mgmt;

		if (!ptr_mgmt) {
			ptr_mgmt = type_ptr_manager_of_device_of_client(new manager_of_device_of_client());

			/*
			if (!capi_client::get_instance().load()) {
				_mp::clog::get_instance().log(L" ***** [E] Load DLL.\n");
			}
			*/
		}

		if (b_remove) {
			ptr_mgmt.reset();//remove manager
			//capi_client::get_instance().unload();
		}

		return ptr_mgmt;
	}

public:
	~manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>()
	{
		std::for_each(std::begin(m_map_device_index_ptr_device_of_client), std::end(m_map_device_index_ptr_device_of_client), [&](auto item) {
			if (item.second)
				item.second.reset();
		});
		m_map_device_index_ptr_device_of_client.clear();
	}

	unsigned long get_client_index() const
	{
		return m_n_client_index;
	}

	bool connect
	(
		const _mp::cclient_cb::type_cb_callbacks & cbs
		, long n_msec_wait_time
		, long long ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete
		, long long ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss
		, long long ll_msec_timeout_ws_client_wait_for_idle_in_wss
		, long long ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss

	)
	{
		bool b_result(false);

		do {
			std::lock_guard<std::mutex> lock(m_mutex_for_dev_manager);

			unsigned long n_client_index = capi_client::get_instance().create_before_set_callback
			(
				ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete
				, ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss
				, ll_msec_timeout_ws_client_wait_for_idle_in_wss
				, ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss
			);
			if (n_client_index == _mp::cclient::UNDEFINED_INDEX) {
				continue;
			}

			m_n_client_index = n_client_index;

			capi_client::get_instance().set_callbacks(m_n_client_index, cbs);
			bool b_tls(true);

			//if (!capi_client::get_instance().start_after_set_callback(m_n_client_index)) {
			if (!capi_client::get_instance().start_after_set_callback_sync(m_n_client_index, b_tls, n_msec_wait_time)) {
				continue;
			}

			b_result = true;
		} while (false);
		return b_result;
	}
	bool disconnect()
	{
		bool b_result(false);

		do {
			std::lock_guard<std::mutex> lock(m_mutex_for_dev_manager);
			if (m_n_client_index == _mp::cclient::UNDEFINED_INDEX) {
				continue;
			}
			if (!capi_client::get_instance().stop_before_destory(m_n_client_index)) {
				continue;
			}
			if (!capi_client::get_instance().destory_after_stop(m_n_client_index)) {
				continue;
			}
			m_n_client_index = _mp::cclient::UNDEFINED_INDEX;
			b_result = true;
		} while (false);
		return b_result;
	}

	_mp::type_list_wstring get_device_list(const std::wstring& s_filter)
	{
		_mp::type_list_wstring list_device(0);
		int n_result_index(-1);

		do {
			std::lock_guard<std::mutex> lock(m_mutex_for_dev_manager);
			if (m_n_client_index == _mp::cclient::UNDEFINED_INDEX)
				continue;

			n_result_index = _create_async_result_for_manager();
			if (n_result_index < 0)
				continue;

			if (!capi_client::get_instance().get_device_list(m_n_client_index, s_filter)) {
				continue;
			}
			_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_async_parameter_result = get_async_parameter_result_for_manager(n_result_index);
			if (!ptr_async_parameter_result->waits())
				continue;

			_mp::type_set_wstring set_s_path;
			if (!ptr_async_parameter_result->get_result(set_s_path))
				continue;
			//
			for (auto s : set_s_path) {
				list_device.push_back(s);
			}//end for

		} while (false);
		remove_async_result_for_manager(n_result_index);
		return list_device;
	}

	_mp::type_list_wstring get_device_list(const _mp::type_list_wstring & list_s_filter)
	{
		_mp::type_list_wstring list_device(0);
		int n_result_index(-1);

		do {
			std::lock_guard<std::mutex> lock(m_mutex_for_dev_manager);
			if (m_n_client_index == _mp::cclient::UNDEFINED_INDEX) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__) && defined(_YSS_ENABLE_STDOUT_FOR_DEBUG_)
				ATLTRACE(L" @@@@@ %ls()-m_n_client_index == _mp::cclient::UNDEFINED_INDEX.\n", __WFUNCTION__);
				std::cout << " @@@@@ m_n_client_index == _mp::cclient::UNDEFINED_INDEX.\n";
#endif
				continue;
			}

			n_result_index = _create_async_result_for_manager();
			if (n_result_index < 0) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__) && defined(_YSS_ENABLE_STDOUT_FOR_DEBUG_)
				ATLTRACE(L" @@@@@ %ls()-n_result_index < 0.\n", __WFUNCTION__);
				std::cout << " @@@@@ n_result_index < 0.\n";
#endif
				continue;
			}

			if (!capi_client::get_instance().get_device_list_multi(m_n_client_index, list_s_filter)) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__) && defined(_YSS_ENABLE_STDOUT_FOR_DEBUG_)
				ATLTRACE(L" @@@@@ %ls()-get_device_list_multi() fail.\n", __WFUNCTION__);
				std::cout << " @@@@@ get_device_list_multi() fail.\n";
#endif
				continue;
			}
			_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_async_parameter_result = get_async_parameter_result_for_manager(n_result_index);
			if (!ptr_async_parameter_result->waits()) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__) && defined(_YSS_ENABLE_STDOUT_FOR_DEBUG_)
				ATLTRACE(L" @@@@@ %ls()-waits() fail.\n", __WFUNCTION__);
				std::cout << " @@@@@ waits() fail.\n";
#endif
				continue;
			}

			_mp::type_set_wstring set_s_path;
			if (!ptr_async_parameter_result->get_result(set_s_path)) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__) && defined(_YSS_ENABLE_STDOUT_FOR_DEBUG_)
				ATLTRACE(L" @@@@@ %ls()-get_result() fail\n", __WFUNCTION__);
				std::cout << " @@@@@ get_result() fail\n";
#endif
				continue;
			}
			//
			for (auto s : set_s_path) {
				list_device.push_back(s);
			}//end for

		} while (false);
		remove_async_result_for_manager(n_result_index);
		return list_device;
	}

	manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>::type_ptr_device_of_client& get_device(unsigned long n_device_index)
	{
		static manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>::type_ptr_device_of_client ptr_null_device(new _T_SUB_DEVICE_OF_CLIENT());

		std::lock_guard<std::mutex> lock(m_mutex_for_dev_manager);
		//
		typename manager_of_device_of_client::_type_map_device_index_ptr_device_of_client::iterator it = m_map_device_index_ptr_device_of_client.find(n_device_index);
		if (std::end(m_map_device_index_ptr_device_of_client) == it)
			return ptr_null_device;
		else
			return it->second;
	}
	manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>::type_ptr_device_of_client& get_device(const std::wstring& s_path)
	{
		static manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>::type_ptr_device_of_client ptr_null_device(new _T_SUB_DEVICE_OF_CLIENT());

		std::lock_guard<std::mutex> lock(m_mutex_for_dev_manager);
		//
		typename manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>::_type_map_device_index_ptr_device_of_client::iterator it = std::begin(m_map_device_index_ptr_device_of_client);
		//
		for (; it != std::end(m_map_device_index_ptr_device_of_client); ++it) {
			if (it->second->is_equal_to_your_device_path(s_path))
				break;
		}//end for

		if (it != std::end(m_map_device_index_ptr_device_of_client))
			return it->second;
		else
			return ptr_null_device;
	}
	unsigned long create_device(const std::wstring& s_path, bool b_shared_mode = false)
	{
		unsigned long n_device_index(i_device_of_client::const_invalied_device_index);

		do {
			std::lock_guard<std::mutex> lock(m_mutex_for_dev_manager);

			if (m_n_client_index == _mp::cclient::UNDEFINED_INDEX)
				continue;
			if (s_path.empty())
				continue;

			manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>::type_ptr_device_of_client ptr_new_device(new _T_SUB_DEVICE_OF_CLIENT(m_n_client_index, s_path));
			if (!ptr_new_device->open(b_shared_mode))
				continue;
			if (ptr_new_device->is_null_device())
				continue;

			n_device_index = ptr_new_device->get_device_index();
			m_map_device_index_ptr_device_of_client[n_device_index] = ptr_new_device;

		} while (false);
		return n_device_index;
	}
	bool remove_device(unsigned long n_device_index)
	{
		bool b_result(false);

		do {
			std::lock_guard<std::mutex> lock(m_mutex_for_dev_manager);

			if (m_n_client_index == _mp::cclient::UNDEFINED_INDEX)
				continue;

			typename manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>::_type_map_device_index_ptr_device_of_client::iterator it = m_map_device_index_ptr_device_of_client.find(n_device_index);
			if (std::end(m_map_device_index_ptr_device_of_client) == it)
				continue;
			if (it->second)
				it->second->close();
			m_map_device_index_ptr_device_of_client.erase(n_device_index);
			//
			b_result = true;
		} while (false);

		if (b_result) {
			_mp::casync_result_manager::get_instance(std::wstring(L"manager_of_device_of_client")).remove_async_result(n_device_index);
		}
		return b_result;
	}

	_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& get_async_parameter_result_for_manager_from_all_device(int n_result_index)
	{
		std::lock_guard<std::mutex> lock(m_mutex_for_dev_manager);

		typename manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>::_type_map_device_index_ptr_device_of_client::iterator it = std::begin(m_map_device_index_ptr_device_of_client);
		for (; it != std::end(m_map_device_index_ptr_device_of_client); ++it) {
			if (!it->second)
				continue;
			if (it->second->is_null_device())
				continue;
			std::wstring s_class_name = it->second->get_class_name();
			_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_result(
				_mp::casync_result_manager::get_instance(s_class_name).get_async_parameter_result_from_all_device(n_result_index)
			);

			if (ptr_result)
				return ptr_result;
		}

		_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_manager_result(
			_mp::casync_result_manager::get_instance(std::wstring(L"manager_of_device_of_client")).get_async_parameter_result_from_all_device(n_result_index)
		);
		
		return ptr_manager_result;
	}
	_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& get_async_parameter_result_for_manager(int n_result_index)
	{
		return _mp::casync_result_manager::get_instance(std::wstring(L"manager_of_device_of_client")).get_async_parameter_result(i_device_of_client::const_invalied_device_index, n_result_index);
	}
	bool remove_async_result_for_manager(int n_result_index)
	{
		return _mp::casync_result_manager::get_instance(std::wstring(L"manager_of_device_of_client")).remove_async_result(i_device_of_client::const_invalied_device_index, n_result_index);
	}

	int pop_async_parameter_result_index_for_manager(bool b_remove = true)
	{
		return _mp::casync_result_manager::get_instance(std::wstring(L"manager_of_device_of_client")).pop_result_index(i_device_of_client::const_invalied_device_index, b_remove);
	}

private:
	// return result index
	int _create_async_result_for_manager(_mp::casync_parameter_result::type_callback p_fun = nullptr, void* p_para = nullptr, HWND h_wnd = NULL, UINT n_msg = 0)
	{
		return _mp::casync_result_manager::get_instance(std::wstring(L"manager_of_device_of_client")).create_async_result(
			i_device_of_client::const_invalied_device_index
			, p_fun, p_para
			, h_wnd, n_msg
		);
	}

	manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>() :
		m_n_client_index(_mp::cclient::UNDEFINED_INDEX)
	{

	}

private:
	std::mutex m_mutex_for_dev_manager;
	unsigned long m_n_client_index;
	manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>::_type_map_device_index_ptr_device_of_client m_map_device_index_ptr_device_of_client;

private://don't call these methods
	manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>(const manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>&);
	manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>& operator=(const manager_of_device_of_client<_T_SUB_DEVICE_OF_CLIENT>&);
};