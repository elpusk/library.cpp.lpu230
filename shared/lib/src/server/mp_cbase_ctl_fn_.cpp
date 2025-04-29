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

	bool cbase_ctl_fn::_set_response(cio_packet& out_response, cqitem_dev& in_qi, cio_packet& in_request)
	{
		bool b_complete(false);

		do {
			cqitem_dev::type_result r(cqitem_dev::result_not_yet);//processing result
			type_v_buffer v;//received data
			std::wstring s;//information
			cio_packet::type_act c_act = cio_packet::act_mgmt_unknown;
			unsigned long n_session = _MP_TOOLS_INVALID_SESSION_NUMBER;
			unsigned long n_device_index = 0;
			//
			std::tie(r, v, s) = in_qi.get_result_all();
			if (r == cqitem_dev::result_not_yet) {
				continue;
			}
			//
			b_complete = true;
			//
			// build response
			out_response = in_request;
			c_act = out_response.get_action();
			n_session = out_response.get_session_number();
			n_device_index = out_response.get_device_index();
			out_response.set_cmd(cio_packet::cmd_response);
			//
			switch (r) {
			case cqitem_dev::result_success:
				if (v.empty()) {
					out_response.set_data_sucesss();
				}
				else {
					out_response.set_data(v);
				}
				break;
			case cqitem_dev::result_error:
				out_response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_device_operation));
				break;
			case cqitem_dev::result_cancel:
				out_response.set_data_cancel();
				break;
			default:
				out_response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_device_operation));
				break;
			}//end switch
			//
		} while (false);
		return b_complete;
	}

	void cbase_ctl_fn::_set_response_result_only(cio_packet& out_response, cqitem_dev::type_result result, const std::wstring& s_result, cio_packet& in_request)
	{
		cio_packet::type_act c_act = cio_packet::act_mgmt_unknown;
		unsigned long n_session = _MP_TOOLS_INVALID_SESSION_NUMBER;
		unsigned long n_device_index = 0;
		//
		// build response
		out_response = in_request;
		c_act = out_response.get_action();
		n_session = out_response.get_session_number();
		n_device_index = out_response.get_device_index();
		out_response.set_cmd(cio_packet::cmd_response);
		//
		switch (result) {
		case cqitem_dev::result_success:
			out_response.set_data_sucesss();
			break;
		case cqitem_dev::result_error:
			out_response.set_data_error(s_result);
			break;
		case cqitem_dev::result_cancel:
			out_response.set_data_cancel(s_result);
			break;
		default:
			out_response.set_data_error(s_result);
			break;
		}//end switch
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



	//=======================================//=======================================
	//=======================================//=======================================
	//cbase_ctl_fn::_cstate member
	// m_trans_table_base 는 exclusive 또는 하나의 session 의 기본 state transaction table.
	const int cbase_ctl_fn::_cstate::_m_trans_table_base[cbase_ctl_fn::_cstate::st_total][cbase_ctl_fn::_cstate::ev_total]
		= {
			{st_idl,st_not,st_not,st_not,st_not},
			{st_idl,st_not,st_idl,st_asy,st_idl},
			{st_asy,st_not,st_idl,st_asy,st_idl}
	};

	std::pair<bool, cbase_ctl_fn::_cstate::type_state> cbase_ctl_fn::_cstate::get_combination_state_in_map_except_selected_session
	(
		const cbase_ctl_fn::_cstate::type_map_ptr_state& map_ptr_state_cur,
		unsigned long n_selected_session_number
	)
	{
		bool b_exist(false);
		cbase_ctl_fn::_cstate::type_state st_result(cbase_ctl_fn::_cstate::st_not);

		do {
			if (map_ptr_state_cur.empty()) {
				continue;
			}

			cbase_ctl_fn::_cstate::type_map_ptr_state::const_iterator it_sel = map_ptr_state_cur.find(n_selected_session_number);
			if (it_sel != map_ptr_state_cur.cend()) {
				if (map_ptr_state_cur.size() == 1) {
					continue; // n_session 의 state 만 존재하는 경우. - another session is none.
				}
			}

			b_exist = true; //there are another session.
			st_result = cbase_ctl_fn::_cstate::st_not;

			cbase_ctl_fn::_cstate::type_map_ptr_state::const_iterator it = map_ptr_state_cur.begin();
			for (; it != map_ptr_state_cur.cend(); ++it) {
				if (it == it_sel) {
					continue;
				}
				if (it->second->get() <= st_result) {
					continue;
				}
				st_result = it->second->get();
			}//end for

		} while (false);
		return std::make_pair(b_exist, st_result);

	}

	cbase_ctl_fn::_cstate::type_evt cbase_ctl_fn::_cstate::get_event_from_request_action(cio_packet::type_act act)
	{
		cbase_ctl_fn::_cstate::type_evt evt(cbase_ctl_fn::_cstate::ev_none);

		switch (act)
		{
		case cio_packet::act_dev_open:
			evt = cbase_ctl_fn::_cstate::ev_open;
			break;
		case cio_packet::act_dev_close:
			evt = cbase_ctl_fn::_cstate::ev_close;
			break;
		case cio_packet::act_dev_transmit:
		case cio_packet::act_dev_cancel:
		case cio_packet::act_dev_write:
			evt = cbase_ctl_fn::_cstate::ev_sync;
			break;
		case cio_packet::act_dev_read:
			evt = cbase_ctl_fn::_cstate::ev_asy;
			break;
		case cio_packet::act_dev_sub_bootloader:
		default:
			break;
		}//end switch
		return evt;
	}

	int cbase_ctl_fn::_cstate::get_mask_from_state(_cstate::type_state st)
	{
		int n_mask = 0;

		do {

			if ((int)st <= (int)_cstate::st_undefined) {
				continue;
			}
			if ((int)st >= (int)_cstate::st_total) {
				continue;
			}

			n_mask = 0x1 << (int)st;

		} while (false);
		return n_mask;
	}

	cbase_ctl_fn::_cstate::type_state cbase_ctl_fn::_cstate::get_state_from_mask(int n_mask)
	{
		_cstate::type_state st(_cstate::st_undefined);

		switch (st) {
		case 0x01:	st = _cstate::st_not;	break;
		case 0x02:	st = _cstate::st_idl;	break;
		case 0x04:	st = _cstate::st_asy;	break;
		default: break;
		}// end swithc

		return st;
	}

	cbase_ctl_fn::_cstate::_cstate()
	{
		this->reset();
	}

	cbase_ctl_fn::_cstate::_cstate(const cbase_ctl_fn::_cstate& src)
	{
		*this = src;
	}

	cbase_ctl_fn::_cstate& cbase_ctl_fn::_cstate::operator=(const cbase_ctl_fn::_cstate& src)
	{
		m_st_cur = src.m_st_cur;
		m_ev_last = src.m_ev_last;
		return *this;
	}
	cbase_ctl_fn::_cstate::~_cstate()
	{
	}

	void cbase_ctl_fn::_cstate::reset()
	{
		m_st_cur = _cstate::st_not;
		m_ev_last = _cstate::ev_none;
	}

	cbase_ctl_fn::_cstate::type_state cbase_ctl_fn::_cstate::get() const
	{
		return m_st_cur;
	}

	cbase_ctl_fn::_cstate::type_evt cbase_ctl_fn::_cstate::get_last_event() const
	{
		return m_ev_last;
	}

	cbase_ctl_fn::_cstate::type_result_evt   cbase_ctl_fn::_cstate::set(_cstate::type_evt evt)
	{
		_cstate::type_state st_new(_cstate::st_not);
		_cstate::type_state st_old(_cstate::st_not);

		st_old = m_st_cur;
		st_new = (_cstate::type_state)_cstate::_m_trans_table_base[m_st_cur][evt];

		m_st_cur = st_new;
		m_ev_last = evt;

		bool b_changed(false);
		if (st_new != st_old) {
			b_changed = true;
		}
		return std::make_tuple(b_changed, st_new, st_old);

	}

	//=======================================//=======================================
	//=======================================//=======================================
	////cbase_ctl_fn::cresult  member
	cbase_ctl_fn::cresult::cresult()
	{
		reset();
	}

	cbase_ctl_fn::cresult::cresult(const cbase_ctl_fn::cresult& src)
	{
		*this = src;
	}

	cbase_ctl_fn::cresult::cresult(const cio_packet& request)
	{
		reset();
		m_ptr_req = std::make_shared<cio_packet>(request);
	}

	cbase_ctl_fn::cresult::cresult(const cio_packet::type_ptr& ptr_request)
	{
		reset();
		m_ptr_req = ptr_request;
	}

	cbase_ctl_fn::cresult& cbase_ctl_fn::cresult::operator=(const cbase_ctl_fn::cresult& src)
	{
		m_b_process_exist_session = src.m_b_process_exist_session;
		m_st_session_old = src.m_st_session_old;
		m_st_session_new = src.m_st_session_new;

		m_st_combination_old = src.m_st_combination_old;
		m_st_combination_new = src.m_st_combination_new;

		m_b_process_result = src.m_b_process_result;
		m_b_process_complete = src.m_b_process_complete;
		m_ptr_req = src.m_ptr_req;
		m_ptr_rsp = src.m_ptr_rsp;
		m_s_dev_path = src.m_s_dev_path;
		return *this;
	}

	void cbase_ctl_fn::cresult::reset()
	{
		m_b_process_exist_session = false;

		m_st_session_old = cbase_ctl_fn::_cstate::st_not;
		m_st_session_new = cbase_ctl_fn::_cstate::st_not;

		m_st_combination_old = cbase_ctl_fn::_cstate::st_not;
		m_st_combination_new = cbase_ctl_fn::_cstate::st_not;

		m_b_process_result = false;
		m_b_process_complete = true;

		m_s_dev_path.clear();
	}

	const cio_packet::type_ptr& cbase_ctl_fn::cresult::get_req() const
	{
		return m_ptr_req;
	}

	unsigned long cbase_ctl_fn::cresult::get_session_number() const
	{
		unsigned long n_session(_MP_TOOLS_INVALID_SESSION_NUMBER);
		if (m_ptr_req) {
			n_session = m_ptr_req->get_session_number();
		}
		return n_session;
	}

	std::wstring cbase_ctl_fn::cresult::get_dev_path() const
	{
		return m_s_dev_path;
	}


	size_t cbase_ctl_fn::cresult::get_rx(type_v_buffer& v_data) const
	{
		size_t n_size(0);

		do {
			if (!m_ptr_rsp) {
				continue;
			}

			n_size = m_ptr_rsp->get_packet_by_json_format(v_data);

		} while (false);
		
		return n_size;
	}

	cbase_ctl_fn::_cstate::type_evt cbase_ctl_fn::cresult::get_cur_event() const
	{
		_cstate::type_evt evt(_cstate::ev_none);

		do {
			if (!m_ptr_req) {
				continue;
			}
			evt = cbase_ctl_fn::_cstate::get_event_from_request_action(m_ptr_req->get_action());

		} while (false);
		return evt;
	}

	type_pair_bool_result_bool_complete cbase_ctl_fn::cresult::process_get_result() const
	{
		return std::make_pair(m_b_process_result, m_b_process_complete);
	}

	cio_packet::type_ptr cbase_ctl_fn::cresult::process_set_req_packet(const cio_packet& request)
	{
		m_ptr_req = std::make_shared<cio_packet>(request);
		m_ptr_rsp = std::make_shared<cio_packet>(request);

		m_ptr_rsp->set_cmd(cio_packet::cmd_response).set_data_error();

		return m_ptr_rsp;
	}

	void cbase_ctl_fn::cresult::after_processing_set_rsp_with_error_complete
	(
		cio_packet::type_error_reason n_reason,
		const std::wstring& s_dev_path /*= std::wstring()*/
	)
	{
		do {
			if (!m_ptr_req) {
				continue;
			}
			if (!m_ptr_rsp) {
				m_ptr_rsp = std::make_shared<cio_packet>(*m_ptr_req);
				m_ptr_rsp->set_cmd(cio_packet::cmd_response);
			}
			//
			m_b_process_exist_session = true;
			m_s_dev_path = s_dev_path;

			if (n_reason == cio_packet::error_reason_none)
				m_ptr_rsp->set_data_error();
			else {
				m_ptr_rsp->set_data_error(cio_packet::get_error_message(n_reason));

				if (n_reason == cio_packet::error_reason_session) {
					m_b_process_exist_session = false;
				}
			}
			//
		} while (false);

		m_b_process_result = false;
		m_b_process_complete = true;
	}

	void cbase_ctl_fn::cresult::after_processing_set_rsp_with_succss_complete(const _mp::type_v_buffer& v_rx, const std::wstring& s_dev_path)
	{
		do {
			if (!m_ptr_req) {
				continue;
			}
			if (!m_ptr_rsp) {
				m_ptr_rsp = std::make_shared<cio_packet>(*m_ptr_req);
				m_ptr_rsp->set_cmd(cio_packet::cmd_response);
			}
			//
			m_b_process_exist_session = true;
			m_s_dev_path = s_dev_path;

			m_ptr_rsp->set_data_sucesss();
			if (!v_rx.empty()) {
				m_ptr_rsp->set_data(v_rx, true);
			}
			//
		} while (false);

		m_b_process_result = true;
		m_b_process_complete = true;
	}

	void cbase_ctl_fn::cresult::after_processing_set_rsp_with_succss_complete(const std::wstring& s_data, const std::wstring& s_dev_path)
	{
		do {
			if (!m_ptr_req) {
				continue;
			}
			if (!m_ptr_rsp) {
				m_ptr_rsp = std::make_shared<cio_packet>(*m_ptr_req);
				m_ptr_rsp->set_cmd(cio_packet::cmd_response);
			}
			//
			m_b_process_exist_session = true;
			m_s_dev_path = s_dev_path;

			m_ptr_rsp->set_data_sucesss();
			if (!s_data.empty()) {
				m_ptr_rsp->set_data_by_utf8(s_data, true);
			}
			//
		} while (false);

		m_b_process_result = true;
		m_b_process_complete = true;
	}

	void cbase_ctl_fn::cresult::after_processing_set_rsp_with_succss_complete(const cio_packet::type_ptr& ptr_reponse, const std::wstring& s_dev_path)
	{
		do {
			if (!m_ptr_req) {
				continue;
			}
			if (!ptr_reponse) {
				m_ptr_rsp = std::make_shared<cio_packet>(*m_ptr_req);
				m_ptr_rsp->set_cmd(cio_packet::cmd_response);
				m_ptr_rsp->set_data_sucesss();
			}
			else {
				m_ptr_rsp = ptr_reponse;
			}
			//
			m_b_process_exist_session = true;
			m_s_dev_path = s_dev_path;
			//
		} while (false);

		m_b_process_result = true;
		m_b_process_complete = true;
	}

	void cbase_ctl_fn::cresult::after_starting_process_set_rsp_with_succss_ing
	(
		const std::wstring& s_dev_path /*= std::wstring()*/
	)
	{
		do {
			if (!m_ptr_req) {
				continue;
			}
			//
			m_b_process_exist_session = true;
			m_s_dev_path = s_dev_path;
			//
		} while (false);

		m_b_process_result = true;
		m_b_process_complete = false;
	}

	bool cbase_ctl_fn::cresult::is_changed_state_of_selected_session() const
	{
		if (m_st_session_old == m_st_session_new) {
			return false;
		}
		return true;
	}

	bool cbase_ctl_fn::cresult::is_changed_combination_state_of_all_session() const
	{
		if (m_st_combination_old == m_st_combination_new) {
			return false;
		}
		return true;
	}

	void cbase_ctl_fn::cresult::set_selected_session_state
	(
		cbase_ctl_fn::_cstate::type_state st_new,
		cbase_ctl_fn::_cstate::type_state st_old
	)
	{
		m_st_session_old = st_old;
		m_st_session_new = st_new;
	}

	void cbase_ctl_fn::cresult::set_selected_session_state(cbase_ctl_fn::_cstate::type_result_evt st_result)
	{
		m_st_session_new = std::get<1>(st_result);
		m_st_session_old = std::get<2>(st_result);
	}

	void cbase_ctl_fn::cresult::set_combination_state(cbase_ctl_fn::_cstate::type_state st_new, cbase_ctl_fn::_cstate::type_state st_old)
	{
		m_st_combination_old = st_old;
		m_st_combination_new = st_new;
	}

	std::pair<cbase_ctl_fn::_cstate::type_state, cbase_ctl_fn::_cstate::type_state> cbase_ctl_fn::cresult::get_combination_state() const
	{
		return std::make_pair(m_st_combination_new, m_st_combination_old);
	}

}//the end of _mp namespace