#pragma once

#include <i_device_of_client.h>
#include <cprotocol_lpu237.h>

class lpu237_of_client : public i_device_of_client
{

public:
	typedef std::shared_ptr< lpu237_of_client >	type_ptr_lpu237_of_client;
	//
public:
	virtual const std::wstring get_class_name()
	{
		return std::wstring(L"lpu237_of_client");
	}

private:
	enum : unsigned long {
		_const_default_mmsec_timeout_of_response = 1000
	};
public:
	lpu237_of_client();
	lpu237_of_client(unsigned long n_client_index, const std::wstring & s_device_path);
	virtual ~lpu237_of_client();

	lpu237_of_client(const lpu237_of_client& src);

	//cmd_x() : communicate with device. by device protocol.
	bool cmd_get_id();
	bool cmd_enter_config();
	bool cmd_leave_config();
	bool cmd_enter_opos();
	bool cmd_leave_opos();
	bool cmd_bypass(const _mp::type_v_buffer& v_tx, _mp::type_v_buffer& v_rx);

	int cmd_async_waits_data()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return _cmd_async_waits_rx(nullptr, nullptr, NULL, 0);
	}
	int cmd_async_waits_data(_mp::casync_parameter_result::type_callback p_fun, void* p_para)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return _cmd_async_waits_rx(p_fun, p_para, NULL, 0);
	}
	int cmd_async_waits_data(HWND h_wnd, UINT n_msg)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return _cmd_async_waits_rx(nullptr, nullptr, h_wnd, n_msg);
	}

private:
	bool _cmd_get(cprotocol_lpu237::type_cmd c_cmd);

	//return result index
	int _cmd_async_waits_rx(_mp::casync_parameter_result::type_callback p_fun, void* p_para, HWND h_wnd, UINT n_msg);

private:
	cprotocol_lpu237 m_protocol;

};

