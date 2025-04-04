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

#include <server/mp_cserver_.h>
#include <server/mp_cdev_ctl_fn_.h>

#include <hid/mp_clibhid.h>

#if defined(_WIN32) && defined(_DEBUG)
#include <atltrace.h>
#endif


namespace _mp {
		cdev_ctl_fn::~cdev_ctl_fn()
		{
		}
		cdev_ctl_fn::cdev_ctl_fn() : 
			m_n_cnt_open(0),
			m_b_shared_mode(false)
		{
		}

		bool cdev_ctl_fn::_execute_general_error_response(
			clog* p_log,
			cio_packet& request, 
			cio_packet& response, 
			cio_packet::type_error_reason n_reason /*= cio_packet::error_reason_none*/
		)
		{
			response = request;
			response.set_cmd(cio_packet::cmd_response);
			if (n_reason == cio_packet::error_reason_none)
				response.set_data_error();
			else
				response.set_data_error(cio_packet::get_error_message(n_reason));
			//
			return true;//complete with error
		}

		bool cdev_ctl_fn::_set_response(cio_packet& out_response, cqitem_dev& in_qi, cio_packet& in_request)
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

		void cdev_ctl_fn::_set_response_result_only(cio_packet& out_response, cqitem_dev::type_result result, const std::wstring& s_result, cio_packet& in_request)
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
device support shared	user request is shared	current open status	result
0	0	0	openable
0	0	1	error
0	1	0	error
0	1	1	error
1	0	0	openable
1	0	1	error
1	1	0	openable
1	1	1	openable
		*/
		bool cdev_ctl_fn::_is_openable(bool b_is_support_by_shared_open_device, bool b_is_request_shared_open_by_user) const
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
			if (m_n_cnt_open > 0) {
				if (m_n_cnt_open >= cdev_ctl_fn::_const_max_n_cnt_open) {
					return false;
				}
				n_index += 0x01;
			}
			return ar_openable[n_index];
		}

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_open_sync(clog* p_log, const cio_packet& request, cio_packet& response)
		{
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"++ %09u + _execute_open_sync\n", request.get_session_number());
#endif

			bool b_complete(true);
			bool b_result(false);
			bool b_user_shared_mode(false);

			response = request;
			response.set_cmd(cio_packet::cmd_response);

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_open);
				response.set_data_error();

				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					continue;
				}
				
				type_list_wstring list_wstring_data_field;
				if (request.get_data_field_type() != cio_packet::data_field_string_utf8) {
					response.set_data_by_utf8(cio_packet::get_error_message(cio_packet::error_reason_device_path), true);
					p_log->log_fmt(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					p_log->trace(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					continue;//not supported format.
				}
				if (request.get_data_field(list_wstring_data_field) == 0) {
					response.set_data_by_utf8(cio_packet::get_error_message(cio_packet::error_reason_device_path), true);
					p_log->log_fmt(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					p_log->trace(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					continue;
				}
				// packet accept OK.
				std::wstring s_dev_path;

				s_dev_path = list_wstring_data_field.front();	// device path
				list_wstring_data_field.pop_front();

				if (!list_wstring_data_field.empty()) {
					std::wstring s_mode = list_wstring_data_field.front(); //none or share

					if (s_mode.compare(L"share") == 0) {
						b_user_shared_mode = true;
					}
				}
				_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(s_dev_path);
				if (wptr_dev.expired()) {
					p_log->log_fmt(L"[E] - %ls | lib_hid.get_device() is expired() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					p_log->trace(L"[E] - %ls | lib_hid.get_device() is expired() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					continue;
				}
				if (!wptr_dev.lock()->is_open()) {
					response.set_data_by_utf8(cio_packet::get_error_message(cio_packet::error_reason_device_open), true);
					p_log->log_fmt(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					p_log->trace(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					continue;
				}

				bool b_support_shared = wptr_dev.lock()->is_support_shared_open();
				if (!_is_openable(b_support_shared, b_user_shared_mode)) {
					// open 을 시도할 precondition 만족하지 않음.
					response.set_data_by_utf8(cio_packet::get_error_message(cio_packet::error_reason_device_open), true);
					p_log->log_fmt(L"[E] - %ls | open counter = %d : session = %u.\n", __WFUNCTION__, m_n_cnt_open, request.get_session_number());
					p_log->trace(L"[E] - %ls | open counter = %d : session = %u.\n", __WFUNCTION__, m_n_cnt_open, request.get_session_number());
					continue;
				}

				// m_b_shared_mode = b_user_shared_mode; don't this style code, Use below sytle for workaround a cocurrent problems.
				if (b_user_shared_mode) {
					if (!m_b_shared_mode)
						m_b_shared_mode = true;
				}
				else {
					if (m_b_shared_mode) {
						m_b_shared_mode = false;
					}
				}

				//m_map_parameter_for_independent_bootloader.clear();
				response.set_data_sucesss();
				++m_n_cnt_open;
				m_s_dev_path = s_dev_path;

				b_result = true;
			} while (false);

			if (b_result) {
				//push_info(_ns_tools::ct_color::COLOR_NORMAL, L"_execute_open : success : session = %u", request.get_session_number());
			}

#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"-- %09u + _execute_open_sync\n", request.get_session_number());
#endif
			return std::make_pair(b_result, b_complete);
		}

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_close_sync(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			const std::wstring& s_dev_path
		)
		{
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"++ %09u + _execute_close_sync\n", request.get_session_number());
#endif

			bool b_complete(true);
			bool b_result(false);

			response = request;
			response.set_cmd(cio_packet::cmd_response);

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_open);

				response.set_data_error();

				if (m_n_cnt_open <= 0) {
					continue;
				}

				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					continue;
				}

				_mp::clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(s_dev_path);
				if (wptr_dev.expired()) {
					p_log->log_fmt(L"[E] - %ls | lib_hid.get_device() is expired() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					p_log->trace(L"[E] - %ls | lib_hid.get_device() is expired() : session = %u.\n", __WFUNCTION__, request.get_session_number());
					continue;
				}

				response.set_data_sucesss();
				--m_n_cnt_open;
				if (m_n_cnt_open <= 0) {
					m_b_shared_mode = false;// set to default mode.
					m_s_dev_path.clear();
				}

				b_result = true;
			} while (false);

			if (b_result) {
				//push_info(_ns_tools::ct_color::COLOR_NORMAL, L"%ls : success : session = %u", __WFUNCTION__, request.get_session_number());
			}

#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"-- %09u + _execute_close_sync\n", request.get_session_number());
#endif
			return std::make_pair(b_result, b_complete);
		}

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_close(clog* p_log, const cio_packet& request, cio_packet& response)
		{
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"++ %09u + _execute_close\n", request.get_session_number());
#endif

			response = request;
			response.set_cmd(cio_packet::cmd_response);
			
			std::lock_guard<std::mutex> lock(m_mutex_for_open);

			if (m_n_cnt_open > 0) {
				response.set_data_sucesss();
				--m_n_cnt_open;

				if (m_n_cnt_open <=0 ) {
					m_b_shared_mode = false;// set to default mode.
					m_s_dev_path.clear();
				}
			}
			else {
				response.set_data_error();
			}

			type_pair_bool_result_bool_complete pair_bool_result_bool_complete(true, true);

#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"-- %09u + _execute_close\n", request.get_session_number());
#endif
			return pair_bool_result_bool_complete;
		}

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_read(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			const std::wstring& s_dev_path
		)
		{
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"++ %09u + _execute_read\n", request.get_session_number());
#endif
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(p_log, request, response, s_dev_path);
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"-- %09u + _execute_read\n", request.get_session_number());
#endif
			return pair_bool_result_bool_complete;
		}
		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_write(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			const std::wstring& s_dev_path
		)
		{
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"++ %09u + _execute_write\n", request.get_session_number());
#endif
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(p_log, request, response, s_dev_path);
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"-- %09u + _execute_write\n", request.get_session_number());
#endif
			return pair_bool_result_bool_complete;
		}
		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_transmit(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			const std::wstring& s_dev_path
		)
		{
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"++ %09u + _execute_transmit\n", request.get_session_number());
#endif
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(p_log, request, response, s_dev_path);
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"-- %09u + _execute_transmit\n", request.get_session_number());
#endif
			return pair_bool_result_bool_complete;
		}
		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_cancel(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			const std::wstring& s_dev_path
		)
		{
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"++ %09u + _execute_cancel\n", request.get_session_number());
#endif
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(p_log, request, response, s_dev_path);
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"-- %09u + _execute_cancel\n", request.get_session_number());
#endif
			return pair_bool_result_bool_complete;
		}

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_aync(
			clog* p_log, 
			const cio_packet& request,
			cio_packet& response,
			const std::wstring& s_dev_path
		)
		{
			bool b_complete(true);
			bool b_result(false);

			cwait::type_ptr ptr_evt;
			int n_evt(-1);
			cio_packet::type_ptr ptr_rsp;

			response = request;
			response.set_cmd(cio_packet::cmd_response);

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_open);
				response.set_data_error();

				if (m_n_cnt_open <= 0) {
					continue;
				}

				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					continue;
				}

				clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(s_dev_path);
				if (wptr_dev.expired()) {
					continue;
				}

				type_v_buffer v_tx;
				request.get_data_field(v_tx);

				switch (request.get_action())
				{
				case cio_packet::act_dev_transmit:
					std::tie(std::ignore,ptr_evt, std::ignore, ptr_rsp) = m_mgmt_q.sync_push_back(request);
#if defined(_WIN32) && defined(_DEBUG)
					ATLTRACE(L"++ %09u + _execute_aync-start_write_read(_cb_dev_for_sync_req)\n", request.get_session_number());
#endif
					wptr_dev.lock()->start_write_read(v_tx, cdev_ctl_fn::_cb_dev_for_sync_req, this, request.get_session_number());
					if (ptr_evt) {
						ptr_evt->wait_for_at_once();
						// b_complete 를 true 로 해서, cdev_ctl::_execute() 에서 응답을 client 에 전송.
						response = *ptr_rsp;
					}
					break;
				case cio_packet::act_dev_cancel:
					std::tie(std::ignore,ptr_evt, std::ignore, ptr_rsp) = m_mgmt_q.sync_push_back(request);
#if defined(_WIN32) && defined(_DEBUG)
					ATLTRACE(L"++ %09u + _execute_aync-start_cancel(_cb_dev_for_sync_req)\n", request.get_session_number());
#endif
					wptr_dev.lock()->start_cancel(cdev_ctl_fn::_cb_dev_for_sync_req, this, request.get_session_number());
					if (ptr_evt) {
						ptr_evt->wait_for_at_once();
						// b_complete 를 true 로 해서, cdev_ctl::_execute() 에서 응답을 client 에 전송.
						response = *ptr_rsp;
					}
					break;
				case cio_packet::act_dev_write:
					std::tie(std::ignore,ptr_evt, std::ignore, ptr_rsp) = m_mgmt_q.sync_push_back(request);
#if defined(_WIN32) && defined(_DEBUG)
					ATLTRACE(L"++ %09u + _execute_aync-start_write(_cb_dev_for_sync_req)\n", request.get_session_number());
#endif
					wptr_dev.lock()->start_write(v_tx, cdev_ctl_fn::_cb_dev_for_sync_req, this, request.get_session_number());
					if (ptr_evt) {
						ptr_evt->wait_for_at_once();
						// b_complete 를 true 로 해서, cdev_ctl::_execute() 에서 응답을 client 에 전송.
						response = *ptr_rsp;
					}
					break;
				case cio_packet::act_dev_read:
					if (m_mgmt_q.read_push_back(request)) {
						if (m_b_shared_mode) {
#if defined(_WIN32) && defined(_DEBUG)
							ATLTRACE(L"++ %09u + _execute_aync-start_read(_cb_dev_read_on_shared)\n", request.get_session_number());
#endif						
							wptr_dev.lock()->start_read(cdev_ctl_fn::_cb_dev_read_on_shared, this, request.get_session_number());
						}
						else {
#if defined(_WIN32) && defined(_DEBUG)
							ATLTRACE(L"++ %09u + _execute_aync-start_read(_cb_dev_read_on_exclusive)\n", request.get_session_number());
#endif
							wptr_dev.lock()->start_read(cdev_ctl_fn::_cb_dev_read_on_exclusive, this, request.get_session_number());
						}
					}
					// b_complete 를 false 로 해서, continue 함수 실행.
					b_complete = false;
					break;
				default:
					//error complete.
					continue;
				}//end switch

				b_result = true;
			} while (false);

			if (b_result) {
				//push_info(_ns_tools::ct_color::COLOR_NORMAL, L"_execute_aync[%c] : pushed : session = %u", (wchar_t)request.get_action(), request.get_session_number());
			}
			return std::make_pair(b_result, b_complete);
		}

		const std::wstring cdev_ctl_fn::_get_device_path() const
		{
			return m_s_dev_path;
		}

		cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full cdev_ctl_fn::_cq_mgmt::sync_push_back(const cio_packet& request)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			auto r = m_q_sync_req_evt_rsp.push_back(request);

			//
			// m_map_ptr_q_ptr_cur_req_read 에 기존 read 명령은 device start_X 명령 실행으로
			// 하위 device io 에서 자동 취소 되서, 기존 read 명령 실행시 설정한 callback 이 cancel result 를 가지고 호출된다.
			// 따라서, m_map_ptr_q_ptr_cur_req_read 에 기존 명령이 있어도 자동으로 처리된다.

			return r;
		}

		cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full cdev_ctl_fn::_cq_mgmt::sync_pop_front(bool b_remove)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			auto r = m_q_sync_req_evt_rsp.pop_front(b_remove);
			return r;
		}

		void cdev_ctl_fn::_cq_mgmt::sync_remove_front()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_q_sync_req_evt_rsp.remove_front();
		}

		bool cdev_ctl_fn::_cq_mgmt::read_is_empty()
		{
			bool b_empty(true);
			std::lock_guard<std::mutex> lock(m_mutex);
			b_empty = m_map_ptr_q_ptr_cur_req_read.empty();
			return b_empty;
		}

		bool cdev_ctl_fn::_cq_mgmt::read_push_back(const cio_packet& request)
		{
			bool b_first(false);

			// key 가 있으면, 항상 value 값인 ptr_q 는 항상 할당되어야 한다.
			std::lock_guard<std::mutex> lock(m_mutex);

			unsigned long n_session = request.get_session_number();
			cdev_ctl_fn::_cq_mgmt::_cq::type_ptr ptr_q;

			if (m_map_ptr_q_ptr_cur_req_read.empty()) {
				b_first = true;
			}

			auto it = m_map_ptr_q_ptr_cur_req_read.find(n_session);
			if (it == std::end(m_map_ptr_q_ptr_cur_req_read)) {
				ptr_q = std::make_shared<cdev_ctl_fn::_cq_mgmt::_cq>(n_session);
				m_map_ptr_q_ptr_cur_req_read[n_session] = ptr_q;
			}
			else {
				ptr_q = it->second;
			}

			if (b_first) {
				// start_read 를 발생시키는 read request ptr 를 m_ptr_cur_req_read 에 저장.
				std::tie(m_ptr_cur_req_read, std::ignore, std::ignore, std::ignore) = ptr_q->push_back(request);
			}
			else {
				ptr_q->push_back(request);
			}
			return b_first;
		}

		std::vector<cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full> cdev_ctl_fn::_cq_mgmt::read_pop_front(unsigned long n_session_number,bool b_remove)
		{
			cwait::type_ptr ptr_evt;
			int n_evet(-1);
			cio_packet::type_ptr ptr_req, ptr_rsp;
			std::vector<cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full> v_result;

			do {
				std::lock_guard<std::mutex> lock(m_mutex);

				if (m_map_ptr_q_ptr_cur_req_read.empty()) {
					continue;
				}

				_cq_mgmt::_type_map_ptr_q::iterator it;
				_cq_mgmt::_cq::type_ptr ptr_q;

				if (n_session_number != _MP_TOOLS_INVALID_SESSION_NUMBER) {
					it = m_map_ptr_q_ptr_cur_req_read.find(n_session_number);
					if (it == std::end(m_map_ptr_q_ptr_cur_req_read)) {
						continue;
					}
					ptr_q = it->second;
					v_result.push_back(ptr_q->pop_front(b_remove));
					if (ptr_q->empty()) {
						// q 에 item이 없으면 항상 q 자체를 소멸 시킴. 
						m_map_ptr_q_ptr_cur_req_read.erase(n_session_number);
					}
					continue;
				}

				//all session case
				it = std::begin(m_map_ptr_q_ptr_cur_req_read);

				for (; it != std::end(m_map_ptr_q_ptr_cur_req_read); ++it) {
					v_result.push_back(it->second->pop_front(b_remove));
					if (it->second->empty()) {
						// q 에 item이 없으면 항상 q 자체를 소멸 시킴. 
						it = m_map_ptr_q_ptr_cur_req_read.erase(it);
					}
				}//end for

			} while (false);
			return v_result;
		}

		std::vector<cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full> cdev_ctl_fn::_cq_mgmt::read_set_response_front(cqitem_dev& qi, bool b_also_cancel/*=false*/)
		{
			cwait::type_ptr ptr_evt;
			std::vector<cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full> v_result;

			do {
				std::lock_guard<std::mutex> lock(m_mutex);
				_cq_mgmt::_type_map_ptr_q::iterator it;

				if (m_map_ptr_q_ptr_cur_req_read.empty()) {
					continue;
				}

				it = std::begin(m_map_ptr_q_ptr_cur_req_read);

				if (b_also_cancel || qi.get_request_type() != cqitem_dev::req_cancel) {
					// cancel 에 대한 결과가 아니거나, canecel 일 때도 모든 session 에 결과를 setting 하라고 하면.

					for (; it != std::end(m_map_ptr_q_ptr_cur_req_read); ++it) {
						auto r = it->second->pop_front(false);
						ptr_evt = std::get<1>(r);
						if (!ptr_evt) {
							continue;
						}
						if (!cdev_ctl_fn::_set_response(*std::get<3>(r), qi, *std::get<0>(r))) {
							continue;
						}
						v_result.push_back(r);

						ptr_evt->set(std::get<2>(r));
					}//end for
					continue;
				}

				// b_also_cancel is false and qi.get_request_type() == cqitem_dev::req_cancel
				// 같은 session 에 있는 것만 결과 setting.
				for (; it != std::end(m_map_ptr_q_ptr_cur_req_read); ++it) {
					auto r = it->second->pop_front(false);
					ptr_evt = std::get<1>(r);
					if (!ptr_evt) {
						continue;
					}
					if (std::get<0>(r)->get_session_number() != qi.get_session_number()) {
						continue;
					}
					if (!cdev_ctl_fn::_set_response(*std::get<3>(r), qi, *std::get<0>(r))) {
						continue;
					}
					v_result.push_back(r);

					ptr_evt->set(std::get<2>(r));
				}//end for

			} while (false);
			return v_result;
		}

		std::vector<cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full> cdev_ctl_fn::_cq_mgmt::read_pop_front_remove_complete_of_all()
		{
			cwait::type_ptr ptr_evt;
			int n_evet(-1);
			cio_packet::type_ptr ptr_req, ptr_rsp;
			std::vector<cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full> v_result;

			do {
				std::lock_guard<std::mutex> lock(m_mutex);

				if (m_map_ptr_q_ptr_cur_req_read.empty()) {
					continue;
				}

				auto it = std::begin(m_map_ptr_q_ptr_cur_req_read);

				for (; it != std::end(m_map_ptr_q_ptr_cur_req_read); ++it) {
					_cq_mgmt::_cq::type_ptr ptr_q = it->second;
					auto r = ptr_q->pop_front(false);// event set 된 것만 삭제해야 하기 때문에 지금 pop 하면서 삭제 하면 안됌.
					ptr_evt = std::get<1>(r);
					if (!ptr_evt) {
						continue;
					}
					n_evet = std::get<2>(r);

					if (n_evet != ptr_evt->wait_for_one_at_time(0)) {
						// not triggered
						continue;
					}

					// event set됨.
					ptr_q->remove_front();// queue 에서 삭제.
					if (ptr_q->empty()) {
						// 빈 queue 는 map에서 삭제.
						it = m_map_ptr_q_ptr_cur_req_read.erase(it);
					}
					v_result.push_back(r);
				}//end for

			} while (false);
			return v_result;
		}

		void cdev_ctl_fn::_cq_mgmt::read_remove_front(unsigned long n_session_number)
		{
			do {
				std::lock_guard<std::mutex> lock(m_mutex);

				auto it = m_map_ptr_q_ptr_cur_req_read.find(n_session_number);
				if (it == std::end(m_map_ptr_q_ptr_cur_req_read)) {
					continue;
				}

				auto ptr_q = it->second;
				ptr_q->remove_front();
				if (ptr_q->empty()) {
					// q 에 item이 없으면 항상 q 자체를 소멸 시킴. 
					m_map_ptr_q_ptr_cur_req_read.erase(n_session_number);
				}

			} while (false);

		}

		void cdev_ctl_fn::_cq_mgmt::read_remove_front_all()
		{
			do {
				std::lock_guard<std::mutex> lock(m_mutex);

				if (m_map_ptr_q_ptr_cur_req_read.empty()) {
					continue;
				}

				auto it = std::begin(m_map_ptr_q_ptr_cur_req_read);

				for (; it != std::end(m_map_ptr_q_ptr_cur_req_read); ++it) {
					it->second->remove_front();
					if (it->second->empty()) {
						// q 에 item이 없으면 항상 q 자체를 소멸 시킴. 
						it = m_map_ptr_q_ptr_cur_req_read.erase(it);
					}
				}//end for

			} while (false);
		}

		/**
		* callback 에서 서버에 바로 전송하지 말자.
		*/
		bool cdev_ctl_fn::_cb_dev_read_on_exclusive(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			do {
				auto v = p_obj->m_mgmt_q.read_set_response_front(qi);
				if (v.empty()) {
					b_complete = false;
					continue;//more processing
				}
			} while (false);

			return b_complete;
		}

		/**
		* callback 에서 서버에 바로 전송하지 말자.
		*/
		bool cdev_ctl_fn::_cb_dev_read_on_shared(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			do {
				//응답 공유 상태에서는 모든 read 에 대해 응답을 동일하게 설정해주어야 함.
				std::vector<cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full> v_tuple = p_obj->m_mgmt_q.read_set_response_front(qi);
				if (v_tuple.empty()) {
					b_complete = false;
					continue;//more processing
				}

			} while (false);

			return b_complete;
		}

		/**
		* callback of clibhid_dev.start_write, cancel, write_read.
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		bool cdev_ctl_fn::_cb_dev_for_sync_req(cqitem_dev& qi, void* p_user)
		{
			//callback 에서 서버에 바로 전송하지 말자.
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			cio_packet::type_ptr ptr_req, ptr_rsp;
			cwait::type_ptr ptr_evt;
			int n_evt_index(-1);

			do {
				// pop 에서 request 얻는과 결과 setting 이 분리되어도, 동기식으로 event set 될때 까지 기다리니까 안전.
				std::tie(ptr_req, ptr_evt, n_evt_index, ptr_rsp) = p_obj->m_mgmt_q.sync_pop_front(false);
				if (!ptr_req) {
					continue;// 요청한 session 의 응답이 아니면, 무시.
				}

				if (!cdev_ctl_fn::_set_response(*ptr_rsp, qi, *ptr_req)) {
					//현재 요청은 아직 결과를 몰라서 큐에서 삭제하면 안됌.
					b_complete = false;
					continue;//more processing
				}
				p_obj->m_mgmt_q.sync_remove_front(); // 현재 동기 명령 정보 제거.
				//
				if (ptr_evt) {
					ptr_evt->set(n_evt_index); //동기식 응답 기다리는 event 기다림 해제.
				}

			} while (false);
			return b_complete;
		}

		////////////////////////////////////////////////
		// inner queue section _cq

		cdev_ctl_fn::_cq_mgmt::_cq::_cq() : m_n_session_number(_MP_TOOLS_INVALID_SESSION_NUMBER)
		{
		}

		cdev_ctl_fn::_cq_mgmt::_cq::_cq(unsigned long n_session_number) :
			m_n_session_number(n_session_number)
		{
		}

		cdev_ctl_fn::_cq_mgmt::_cq::~_cq()
		{
		}

		cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full cdev_ctl_fn::_cq_mgmt::_cq::push_back(const cio_packet& req)
		{
			cwait::type_ptr ptr_evt;
			int n_evt(-1);
			cio_packet::type_ptr ptr_rsp = std::make_shared<cio_packet>();
			cio_packet::type_ptr ptr_req = std::make_shared<cio_packet>(req);
			ptr_evt = std::make_shared<cwait>();

			// 생성 후, 첫 event 생성이므로 생성된 event index 는 항상 0.
			n_evt = ptr_evt->generate_new_event(); // return waits 0( construct and the first generation)
			m_q.push_back(std::make_tuple(ptr_req, ptr_evt, n_evt, ptr_rsp));

			return std::make_tuple(ptr_req,ptr_evt,n_evt, ptr_rsp);
		}

		cdev_ctl_fn::_cq_mgmt::_cq::type_tuple_full cdev_ctl_fn::_cq_mgmt::_cq::pop_front( bool b_remove )
		{
			cwait::type_ptr ptr_evt;
			cio_packet::type_ptr ptr_req, ptr_rsp;
			int n_evt_index(-1);

			do {
				//std::lock_guard<std::mutex> lock(m_mutex);

				if (m_q.empty()) {
					continue;
				}

				std::tie(ptr_req,ptr_evt, n_evt_index, ptr_rsp) = m_q.front();
				if (b_remove) {
					m_q.pop_front();
				}
				//
			} while (false);
			return std::make_tuple(ptr_req, ptr_evt,n_evt_index, ptr_rsp);
		}

		void cdev_ctl_fn::_cq_mgmt::_cq::remove_front()
		{
			do {
				if (m_q.empty()) {
					continue;
				}

				m_q.pop_front();
				//
			} while (false);
		}

		bool cdev_ctl_fn::_cq_mgmt::_cq::empty()
		{
			return m_q.empty();
		}

		void cdev_ctl_fn::_cq_mgmt::_cq::clear()
		{
			m_q.clear();
		}

}//the end of _mp namespace

