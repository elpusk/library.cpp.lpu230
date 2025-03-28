#pragma once

/*
* Never Call Virtual Functions during Construction or Destruction
*/
#include <memory>
#include <mutex>

#include <mp_type.h>
#include <mp_casync_result_manager.h>
#include <capi_client.h>


/**
 * class i_device_of_client.
 * this class is interface that a device use ccoffee_hlp_client class
 * the base class of x_device_of_client
 */
class i_device_of_client
{
public:
	enum {
		const_invalied_device_index = 0
	};
	typedef std::shared_ptr< i_device_of_client >	type_ptr_i_device_of_client;

public:
	virtual const std::wstring get_class_name() = 0;

public:

	/**
	 * contructure for null device.
	 * 
	 */
	i_device_of_client()
		: m_n_client_index(_mp::cclient::UNDEFINED_INDEX)
		, m_n_device_index(const_invalied_device_index)
		, m_v_id(0) 
	{}

	/**
	 * contructure for generic device..
	 * 
	 * \param n_client_index
	 * \param s_device_path
	 */
	i_device_of_client(unsigned long n_client_index, const std::wstring& s_device_path)
		: m_n_client_index(n_client_index)
		, m_n_device_index(const_invalied_device_index)
		, m_s_device_path(s_device_path)
		, m_v_id(0)
	{
		
	}

	i_device_of_client(const i_device_of_client& src)
	{
		m_n_client_index = src.m_n_client_index;
		m_n_device_index = src.m_n_device_index;
		m_s_device_path = src.m_s_device_path;
		m_v_id = src.m_v_id;
	}

	i_device_of_client& operator=(const i_device_of_client& src)
	{
		m_n_client_index = src.m_n_client_index;
		m_n_device_index = src.m_n_device_index;
		m_s_device_path = src.m_s_device_path;
		m_v_id = src.m_v_id;
		return *this;
	}

	virtual ~i_device_of_client()
	{
	}

	bool open(bool b_shared_mode)
	{
		bool b_result(false);
		do {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_n_client_index == _mp::cclient::UNDEFINED_INDEX)
				continue;
			if (m_n_device_index != const_invalied_device_index)
				continue;
			b_result = _open(b_shared_mode);
		} while (false);
		return b_result;
	}
	bool close()
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_n_client_index != _mp::cclient::UNDEFINED_INDEX)
			return _close();
		else
			return false;
	}
	bool is_null_device()
	{
		if (m_n_client_index == _mp::cclient::UNDEFINED_INDEX)
			return true;
		if (m_n_device_index == const_invalied_device_index)
			return true;
		return false;
	}

	// getter
	unsigned long get_device_index() const
	{
		return m_n_device_index;
	}
	unsigned long get_client_index() const
	{
		return m_n_client_index;
	}
	std::wstring get_device_path() const
	{
		return m_s_device_path;
	}
	_mp::type_v_buffer get_device_id() const
	{
		return m_v_id;
	}

	bool is_equal_to_your_device_index(const unsigned long n_device_index) const
	{
		if (m_n_device_index == n_device_index)
			return true;
		else
			return false;
	}

	bool is_equal_to_your_client_index(const unsigned long n_client_index) const
	{
		if (m_n_client_index == n_client_index)
			return true;
		else
			return false;
	}

	bool is_equal_to_your_device_path(const std::wstring& s_device_path) const
	{
		if (m_s_device_path.compare(s_device_path) == 0)
			return true;
		else
			return false;
	}
	bool is_equal_to_your_device_id(const _mp::type_v_buffer& v_id)
	{
		if (v_id == m_v_id)
			return true;
		else
			return false;
	}

	//reset() : communicate with CF. be not used device protocol.
	bool reset()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return _reset();
	}

	_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& get_async_parameter_result_for_transaction(int n_result_index_for_transaction)
	{
		return _mp::casync_result_manager::get_instance(get_class_name()).get_async_parameter_result(m_n_device_index, n_result_index_for_transaction);
	}

	bool remove_async_result_for_transaction(int n_result_index_for_transaction)
	{
		return _mp::casync_result_manager::get_instance(get_class_name()).remove_async_result(m_n_device_index, n_result_index_for_transaction);
	}

	bool remove_async_result_for_transaction()
	{
		return _mp::casync_result_manager::get_instance(get_class_name()).remove_async_result(m_n_device_index);
	}

	bool empty_async_result_for_transaction()
	{
		return _mp::casync_result_manager::get_instance(get_class_name()).empty_queue(m_n_device_index);
	}

protected:
	void _set_device_id(const _mp::type_v_buffer& v_id)
	{
		m_v_id = v_id;
	}

	bool _open(bool b_shared_mode)
	{
		bool b_result(false);
		unsigned long n_device_index(const_invalied_device_index);
		int n_result_index(_mp::casync_result_manager::const_invalied_result_index);
		do {
			if (!is_null_device())
				continue;
			if (m_s_device_path.empty())
				continue;

			n_result_index = _create_async_result_for_transaction();
			if (n_result_index < 0)
				continue;
			if (!capi_client::get_instance().open(m_n_client_index, m_s_device_path, b_shared_mode)) {
				continue;
			}
			_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_async_parameter_result = get_async_parameter_result_for_transaction(n_result_index);
			if (!ptr_async_parameter_result->waits())
				continue;
			if (!ptr_async_parameter_result->get_result(n_device_index))
				continue;
			remove_async_result_for_transaction(n_result_index);

			m_n_device_index = n_device_index;
			b_result = true;
		} while (false);
		remove_async_result_for_transaction(n_result_index);
		return b_result;
	}

	bool _close()
	{
		bool b_result(false);
		int n_result_index(_mp::casync_result_manager::const_invalied_result_index);

		do {
			if (is_null_device())
				continue;
			n_result_index = _create_async_result_for_transaction();
			if (n_result_index < 0)
				continue;
			if (!capi_client::get_instance().close(m_n_client_index, m_n_device_index)) {
				continue;
			}
			_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_async_parameter_result = get_async_parameter_result_for_transaction(n_result_index);
			if (!ptr_async_parameter_result->waits())
				continue;
			if (!ptr_async_parameter_result->get_result())
				continue;
			//
			m_n_device_index = const_invalied_device_index;
			remove_async_result_for_transaction();
			b_result = true;
		} while (false);
		remove_async_result_for_transaction(n_result_index);
		return b_result;
	}

	// return result index
	int _create_async_result_for_transaction(_mp::casync_parameter_result::type_callback p_fun, void* p_para, HWND h_wnd, UINT n_msg)
	{
		std::wstring s_name(get_class_name());
		return _mp::casync_result_manager::get_instance(s_name).create_async_result(
			m_n_device_index
			, p_fun, p_para
			, h_wnd, n_msg
		);

	}
	// return result index
	int _create_async_result_for_transaction()
	{
		std::wstring s_name(get_class_name());
		return _mp::casync_result_manager::get_instance(s_name).create_async_result(
			m_n_device_index
			, nullptr, nullptr
			, NULL, 0
		);

	}

	/**
	 * this function will cancel the current client-server transaction.
	 * this function is sync type.
	 * 
	 * \return boolean
	 */
	bool _reset()
	{
		bool b_result(false);
		int n_result_index(_mp::casync_result_manager::const_invalied_result_index);
		do {
			if (is_null_device())
				continue;
			n_result_index = _create_async_result_for_transaction();
			if (n_result_index < 0)
				continue;
			if (!capi_client::get_instance().cancel(m_n_client_index, m_n_device_index)) {
				continue;
			}
			_mp::casync_parameter_result::type_ptr_ct_async_parameter_result& ptr_async_parameter_result = get_async_parameter_result_for_transaction(n_result_index);
			if (!ptr_async_parameter_result->waits())
				continue;
			if (!ptr_async_parameter_result->get_result())
				continue;
			//
			b_result = true;
		} while (false);
		remove_async_result_for_transaction(n_result_index);
		return b_result;
	}

protected:
	std::mutex m_mutex;

	unsigned long m_n_client_index;
	unsigned long m_n_device_index;
	std::wstring m_s_device_path;

	_mp::type_v_buffer m_v_id;
};