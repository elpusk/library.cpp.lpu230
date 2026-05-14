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
		_const_default_mmsec_timeout_of_response = 3000
	};
public:
	lpu237_of_client();
	lpu237_of_client(unsigned long n_client_index, const std::wstring & s_device_path);
	virtual ~lpu237_of_client();

	lpu237_of_client(const lpu237_of_client& src);

	_mp::type_v_buffer get_name() const;
	std::string get_name_by_string() const;
	std::wstring get_name_by_wstring() const;

	cprotocol_lpu237::type_version get_system_version() const;

	cprotocol_lpu237::type_function get_device_function() const;

	//cmd_x() : communicate with device. by device protocol.
	bool cmd_get_system_information_with_name();
	bool cmd_get_id();
	bool cmd_enter_config();
	bool cmd_leave_config();
	bool cmd_enter_opos();
	bool cmd_leave_opos();
	bool cmd_bypass(const _mp::type_v_buffer& v_tx, _mp::type_v_buffer& v_rx);

	/**
	* @brief enable which a iButton data send .
	*/
	bool cmd_ibutton_enable();

	/**
	* @brief disable which a iButton data send .
	*/
	bool cmd_ibutton_disble();

	/**
	* @brief 비동기 데이터 수신을 시작한다. return 값으로 받은 result index 로, polling 방식으로 결과를 받을 수 있다.
	* @return result index
	*/
	int cmd_async_waits_data();

	/**
	* @brief 비동기 데이터 수신을 시작한다. return 값으로 받은 result index 로, p_fun 이 call 되었을 때, 수신 데이터를 받을 수 있다.
	* @param p_fun : 콜백함수 포인터. 콜백함수는 _mp::casync_parameter_result::type_callback 형식이어야 한다.
	* @param p_para : 콜백함수에 전달할 사용자 데이터 포인터.
	* @return result index
	*/
	int cmd_async_waits_data(
		_mp::casync_parameter_result::type_callback p_fun
		, void* p_para
	);

	/**
	* @brief 비동기 데이터 수신을 시작한다. return 값으로 받은 result index 로, h_wnd 의 n_msg 핸들러가 호출 되었을 때,수신 데이터를 받을 수 있다.
	* @param h_wnd : 윈도우 핸들. 메시지를 받을 윈도우의 핸들.
	* @param n_msg : 윈도우 메시지 번호. 수신 데이터를 받을 메시지 번호.
	* @return result index
	*/
	int cmd_async_waits_data(HWND h_wnd, UINT n_msg);

	/**
	* @brief 시작된 비동기 transaction 의 각 phase 끝의 콜백에서 호출되서 다음 phase 시작한다.
	*
	* 각 phase 가 완료되면(성공 또는 실패), p_fun(p_para) 가 호출된다.
	*
	* cmd_start_async_get_parameters(), cmd_start_async_set_parameters(),
	* 
	* cmd_start_async_get_parameters_except_combination(), cmd_start_async_set_parameters_except_combination() 시작한 후, 사용한다.
	* 
	* @param p_fun : 콜백함수 포인터. 콜백함수는 _mp::casync_parameter_result::type_callback 형식이어야 한다.
	* @param p_para : 콜백함수에 전달할 사용자 데이터 포인터.
	* @param n_result_index : result 을 얻기 위한 index code.
	* @return 
	* 
	*	std::get<0> - true : phase 시작 성공
	* 
	*	std::get<1> - result index - std::get<0> 가 true 일때, 유효한 result index, std::get<0> 가 false 일때는 -1
	* 
	*	std::get<2> - remainder phase number - std::get<0> 가 true 일때	유효한 remainder phase number, 현재 transaction 에서 남은 phase 수.
	* 
	*	std::get<3> - true : transaction 완료, false : transaction 아직 진행 중. 
	* 
	*/
	std::tuple<bool, int, size_t, bool> cmd_start_async_next_phase(
		_mp::casync_parameter_result::type_callback p_fun
		, void* p_para
		, int n_result_index
	);

	/**
	* @brief 비동기적으로 필요한 모든 parameter 받기 transaction을 시작한다. return 값으로 받은 result index 로, p_fun 이 call 되었을 때, 수신 데이터를 받을 수 있다.
	*  
	* 각 phase 가 완료되면(성공 또는 실패), p_fun(p_para) 가 호출된다.
	* 
	* @param p_fun : 콜백함수 포인터. 콜백함수는 _mp::casync_parameter_result::type_callback 형식이어야 한다.
	* @param p_para : 콜백함수에 전달할 사용자 데이터 포인터.
	* @return first - true : transaction 시작 성공, result index - first 가 true 일때	유효한 result index, first 가 false 일때는 -1
	*/
	std::pair<bool, int> cmd_start_async_get_parameters(
		_mp::casync_parameter_result::type_callback p_fun
		, void* p_para
	);

	/**
	* @brief 비동기적으로 저장 할 parameter 저장 transaction을 시작한다. return 값으로 받은 result index 로, p_fun 이 call 되었을 때, 수신 데이터를 받을 수 있다.
	*
	* 각 phase 가 완료되면(성공 또는 실패), p_fun(p_para) 가 호출된다.
	*
	* @param p_fun : 콜백함수 포인터. 콜백함수는 _mp::casync_parameter_result::type_callback 형식이어야 한다.
	* @param p_para : 콜백함수에 전달할 사용자 데이터 포인터.
	* @return first - true : transaction 시작 성공, result index - first 가 true 일때	유효한 result index, first 가 false 일때는 -1
	*/
	std::pair<bool, int> cmd_start_async_set_parameters(
		_mp::casync_parameter_result::type_callback p_fun
		, void* p_para
	);

	/**
	* @brief 비동기적으로 필요한 모든 parameter 받기 transaction을 시작한다. return 값으로 받은 result index 로, p_fun 이 call 되었을 때, 수신 데이터를 받을 수 있다.
	*
	* combination 관련 항목은 제외
	* 
	* 각 phase 가 완료되면(성공 또는 실패), p_fun(p_para) 가 호출된다.
	*
	* @param p_fun : 콜백함수 포인터. 콜백함수는 _mp::casync_parameter_result::type_callback 형식이어야 한다.
	* @param p_para : 콜백함수에 전달할 사용자 데이터 포인터.
	* @return first - true : transaction 시작 성공, result index - first 가 true 일때	유효한 result index, first 가 false 일때는 -1
	*/
	std::pair<bool, int> cmd_start_async_get_parameters_except_combination(
		_mp::casync_parameter_result::type_callback p_fun
		, void* p_para
	);

	/**
	* @brief 비동기적으로 저장 할 parameter 저장 transaction을 시작한다. return 값으로 받은 result index 로, p_fun 이 call 되었을 때, 수신 데이터를 받을 수 있다.
	*
	* combination 관련 항목은 제외
	* 각 phase 가 완료되면(성공 또는 실패), p_fun(p_para) 가 호출된다.
	*
	* @param p_fun : 콜백함수 포인터. 콜백함수는 _mp::casync_parameter_result::type_callback 형식이어야 한다.
	* @param p_para : 콜백함수에 전달할 사용자 데이터 포인터.
	* @return first - true : transaction 시작 성공, result index - first 가 true 일때	유효한 result index, first 가 false 일때는 -1
	*/
	std::pair<bool, int> cmd_start_async_set_parameters_except_combination(
		_mp::casync_parameter_result::type_callback p_fun
		, void* p_para
	);

private:
	bool _cmd_get(cprotocol_lpu237::type_cmd c_cmd);

	/**
	* @brief 비동기적으로 데이터를 한번	기다리는 명령어. 콜백함수 또는 윈도우 메시지로 결과를 받을 수 있다.
	* @return result index
	*/
	int _cmd_async_waits_rx(_mp::casync_parameter_result::type_callback p_fun, void* p_para, HWND h_wnd, UINT n_msg);

	void _set_name(const _mp::type_v_buffer& v_name);

	void _set_system_version(const cprotocol_lpu237::type_version &version);

	void _set_device_function(cprotocol_lpu237::type_function device_function);

private:
	cprotocol_lpu237 m_protocol;
	_mp::type_v_buffer m_v_name;
	cprotocol_lpu237::type_version m_system_version;
	cprotocol_lpu237::type_function m_device_function;
};

