#include <websocket/mp_win_nt.h>
#include <server/mp_cbase_ctl_fn_.h>
#include <hid/mp_clibhid.h>


#if defined(_WIN32) && defined(_DEBUG)
#include <atltrace.h>
#endif
namespace _mp {
	cbase_ctl_fn::cbase_ctl_fn(clog* p_log) : m_p_ctl_fun_log(p_log)
	{
	}

	cbase_ctl_fn::~cbase_ctl_fn()
	{
	}

	cio_packet::type_ptr cbase_ctl_fn::_generate_error_response(const cio_packet& request, cio_packet::type_error_reason n_reason /*= cio_packet::error_reason_none*/)
	{
		// logg code 를 여기에 넣을시, m_p_ctl_fun_log 를 사용해야 하므로 static 으로 만들면 안돼!

		cio_packet::type_ptr ptr_rsp = std::make_shared<cio_packet>(request);

		ptr_rsp->set_cmd(cio_packet::cmd_response);
		if (n_reason == cio_packet::error_reason_none)
			ptr_rsp->set_data_error();
		else
			ptr_rsp->set_data_error(cio_packet::get_error_message(n_reason));
		//
		return ptr_rsp;//complete with error setting
	}

	std::pair<std::wstring, bool> cbase_ctl_fn::_get_device_path_from_req(const cio_packet::type_ptr& ptr_request)
	{
		// 에러일 경우는 result 에 결과를 설정한다.
		// 성공일 경우는 이 함수는 하나의 transaction 중에 하나의 phase 이므로 결과를 설정 불가하다.
		std::wstring s_dev_path;
		bool b_user_shared_mode = false;

		do {
			// client 부터 받은 parameter 얻기.
			_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
			if (!lib_hid.is_ini()) {
				continue;
			}

			type_list_wstring list_wstring_data_field;
			if (ptr_request->get_data_field_type() != cio_packet::data_field_string_utf8) {
				continue;//not supported format.
			}
			if (ptr_request->get_data_field(list_wstring_data_field) == 0) {
				continue;
			}
			// packet accept OK.
			s_dev_path = list_wstring_data_field.front();	// device path
			list_wstring_data_field.pop_front();

			if (!list_wstring_data_field.empty()) {
				std::wstring s_mode = list_wstring_data_field.front(); //none or share

				if (s_mode.compare(L"share") == 0) {
					b_user_shared_mode = true;
				}
			}

		} while (false);

		return std::make_pair(s_dev_path, b_user_shared_mode);
	}

	bool cbase_ctl_fn::_open_device_by_req
	(
		const std::wstring& s_dev_path,
		bool b_open_by_shared_mode,
		size_t n_open_cnt
	)
	{
		bool b_result(false);

		do {

			_mp::clibhid& lib_hid(_mp::clibhid::get_instance());

			// 하위 라이브러리로 open 가능성 조사.
			_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(s_dev_path);
			if (wptr_dev.expired()) {
				//p_log->log_fmt(L"[E] - %ls | lib_hid.get_device() is expired() : session = %u.\n", __WFUNCTION__, result.ptr_req->get_session_number());
				//p_log->trace(L"[E] - %ls | lib_hid.get_device() is expired() : session = %u.\n", __WFUNCTION__, result.ptr_req->get_session_number());
				continue;
			}
			if (!wptr_dev.lock()->is_open()) {
				//result.ptr_rsp->set_data_by_utf8(cio_packet::get_error_message(cio_packet::error_reason_device_open), true);
				//p_log->log_fmt(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, result.ptr_req->get_session_number());
				//p_log->trace(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, result.ptr_req->get_session_number());
				continue;
			}

			bool b_support_shared = wptr_dev.lock()->is_support_shared_open();

			// shared 를 device 에서 지원하는 여부, 사용자가 shared 를 요청했는지 여부, 현재 device 가 열려 있는지 여부를 고려해서
			// 사용자가 요청한 모드의 open 을 할 수 있는지 검사 한다,
			if (!cbase_ctl_fn::_is_openable(b_support_shared, b_open_by_shared_mode, n_open_cnt)) {
				// open 을 시도할 precondition 만족하지 않음.
				continue;
			}

			// 여기는 openable 한 상태 인데, 

			b_result = true;
		} while (false);


		return b_result;
	}

	/*
		device support shared:	user request is shared:	current open status:	result
		0	0	0	openable
		0	0	1	error
		0	1	0	error
		0	1	1	error
		1	0	0	openable
		1	0	1	error
		1	1	0	openable
		1	1	1	openable
	*/
	bool cbase_ctl_fn::_is_openable(bool b_is_support_by_shared_open_device, bool b_is_request_shared_open_by_user, size_t n_open_cnt)
	{
		const std::array<bool, 8> ar_openable = {
			1,0,0,0,1,0,1,1
		};

		int n_index(0);

		if (b_is_support_by_shared_open_device) {
			n_index += 0x04;
		}
		if (b_is_request_shared_open_by_user) {
			n_index += 0x02;
		}

		if (n_open_cnt > 0) {
			if (n_open_cnt >= cbase_ctl_fn::_const_max_n_cnt_open) {
				return false;
			}
			n_index += 0x01;
		}
		return ar_openable[n_index];
	}


	void cbase_ctl_fn::_debug_win_enter_x(const cio_packet& request, const std::wstring& s_fun_name)
	{
#if defined(_WIN32) && defined(_DEBUG)
		ATLTRACE(L"++ %09u + %s\n", request.get_session_number(), s_fun_name.c_str());
#endif
	}
	void cbase_ctl_fn::_debug_win_leave_x(const cio_packet& request, const std::wstring& s_fun_name)
	{
#if defined(_WIN32) && defined(_DEBUG)
		ATLTRACE(L"-- %09u + %s\n", request.get_session_number(), s_fun_name.c_str());
#endif
	}

}//the end of _mp namespace