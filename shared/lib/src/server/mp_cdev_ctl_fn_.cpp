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

			return std::make_pair(b_result, b_complete);
		}

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_close_sync(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
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

				if (n_conected_session_number != request.get_session_number()) {
					response.set_data_by_utf8(cio_packet::get_error_message(cio_packet::error_reason_session), true);
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
			return pair_bool_result_bool_complete;
		}

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_read(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response, unsigned long n_conected_session_number, const std::wstring& s_dev_path
		)
		{
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"++ %09u + _execute_read\n", request.get_session_number());
#endif
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(p_log, request, response, n_conected_session_number, s_dev_path);
			return pair_bool_result_bool_complete;
		}
		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_write(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		)
		{
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"++ %09u + _execute_write\n", request.get_session_number());
#endif

			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(p_log, request, response, n_conected_session_number, s_dev_path);
			return pair_bool_result_bool_complete;
		}
		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_transmit(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		)
		{
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"++ %09u + _execute_transmit\n", request.get_session_number());
#endif
			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(p_log, request, response, n_conected_session_number, s_dev_path);
			return pair_bool_result_bool_complete;
		}
		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_cancel(
			clog* p_log,
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
		)
		{
#if defined(_WIN32) && defined(_DEBUG)
			ATLTRACE(L"++ %09u + _execute_cancel\n", request.get_session_number());
#endif

			type_pair_bool_result_bool_complete pair_bool_result_bool_complete = _execute_aync(p_log, request, response, n_conected_session_number, s_dev_path);
			return pair_bool_result_bool_complete;
		}

		type_pair_bool_result_bool_complete cdev_ctl_fn::_execute_aync(
			clog* p_log, 
			const cio_packet& request,
			cio_packet& response,
			unsigned long n_conected_session_number, const std::wstring& s_dev_path
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

				if (n_conected_session_number != request.get_session_number()) {
					response.set_data_error(cio_packet::get_error_message(cio_packet::error_reason_session));
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
					//m_b_shared 가 true 일 때만 복구를 위한 저장이 실행됨. 
					std::tie(ptr_evt, n_evt, ptr_rsp) = _push_back_request_for_response(request,true);
#if defined(_WIN32) && defined(_DEBUG)
					ATLTRACE(L"++ %09u + _execute_aync-start_write_read(_cb_dev_for_sync_req)\n", n_conected_session_number);
#endif
					wptr_dev.lock()->start_write_read(v_tx, cdev_ctl_fn::_cb_dev_for_sync_req, this, n_conected_session_number);
					if (ptr_evt) {
						ptr_evt->wait_for_at_once();
						// b_complete 를 true 로 해서, cdev_ctl::_execute() 에서 응답을 client 에 전송.
						response = *ptr_rsp;
					}
					break;
				case cio_packet::act_dev_cancel:
					//m_b_shared 가 true 일 때만 복구를 위한 저장이 실행됨. 
					std::tie(ptr_evt, n_evt, ptr_rsp) = _push_back_request_for_response(request,true);
#if defined(_WIN32) && defined(_DEBUG)
					ATLTRACE(L"++ %09u + _execute_aync-start_cancel(_cb_dev_cancel)\n", n_conected_session_number);
#endif
					wptr_dev.lock()->start_cancel(cdev_ctl_fn::_cb_dev_cancel, this, n_conected_session_number);
					if (ptr_evt) {
						ptr_evt->wait_for_at_once();
						// b_complete 를 true 로 해서, cdev_ctl::_execute() 에서 응답을 client 에 전송.
						response = *ptr_rsp;
					}
					break;
				case cio_packet::act_dev_write:
					//m_b_shared 가 true 일 때만 복구를 위한 저장이 실행됨. 
					std::tie(ptr_evt, n_evt, ptr_rsp) = _push_back_request_for_response(request,true);
#if defined(_WIN32) && defined(_DEBUG)
					ATLTRACE(L"++ %09u + _execute_aync-start_write(_cb_dev_for_sync_req)\n", n_conected_session_number);
#endif
					wptr_dev.lock()->start_write(v_tx, cdev_ctl_fn::_cb_dev_for_sync_req, this, n_conected_session_number);
					if (ptr_evt) {
						ptr_evt->wait_for_at_once();
						// b_complete 를 true 로 해서, cdev_ctl::_execute() 에서 응답을 client 에 전송.
						response = *ptr_rsp;
					}
					break;
				case cio_packet::act_dev_read:
					if (!_push_back_request_for_response_check_running_read_request(request)) {
						// 현재 살행 read 명령 저장. read 명령은 recover 를 위한 backup은 불요.
						// 현재 실행 중인 read 가 없으므로 실행.
						if (m_b_shared_mode) {
#if defined(_WIN32) && defined(_DEBUG)
							ATLTRACE(L"++ %09u + _execute_aync-start_read(_cb_dev_read_on_shared)\n", n_conected_session_number);
#endif
							wptr_dev.lock()->start_read(cdev_ctl_fn::_cb_dev_read_on_shared, this, n_conected_session_number);
						}
						else {
#if defined(_WIN32) && defined(_DEBUG)
							ATLTRACE(L"++ %09u + _execute_aync-start_read(_cb_dev_read_on_exclusive)\n", n_conected_session_number);
#endif
							wptr_dev.lock()->start_read(cdev_ctl_fn::_cb_dev_read_on_exclusive, this, n_conected_session_number);
						}
					}
					// b_complete 를 false 로 해서, callback 함수에서 응답을 client 에 전송.
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

		// 이 함수는 read, write, write-read, cancl 시작 전에 호출되어 작업후,
		// 콜백함수에서의 뒷처리를 위해 현재 request 를 queue 에 push back 한다. 
		// 반환되는 event 가 콜백 함수에 의해 trigged 될 때 까지 기달려서 동기식 처리도 가능하다.
		// 동기식으로 처리하기 위해서는 항상 b_save_for_recover 를 true 로 해서 호출해야 한다.
		std::tuple<cwait::type_ptr, int, cio_packet::type_ptr> cdev_ctl_fn::_push_back_request_for_response(const cio_packet& req, bool b_save_for_recover)
		{
			cwait::type_ptr ptr_evt;
			int n_evt(-1);
			cio_packet::type_ptr ptr_response;

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_map);
				cio_packet::type_act act = req.get_action();
				//
				switch (act) {
				case cio_packet::act_dev_transmit:
				case cio_packet::act_dev_cancel:
				case cio_packet::act_dev_write:
				case cio_packet::act_dev_read:
					break;
				default:
					continue;// unsupported type.
				}//end switch
				//

				// build new item for pushing
				unsigned long n_session_number = req.get_session_number();
				cdev_ctl_fn::_type_map_ptr_q::iterator it_cur_ptr_q = m_map_ptr_q_ptr_cur_req.emplace(n_session_number, std::make_shared<cdev_ctl_fn::_cq>(n_session_number)).first;
				cdev_ctl_fn::_cq::type_ptr ptr_q = it_cur_ptr_q->second;
				cdev_ctl_fn::_cq::type_tuple_full cur_front_item = ptr_q->pop_front(false);
				cio_packet::type_ptr ptr_cur_cio_packet = std::get<0>(cur_front_item);

				do {
					// session number  에 의해 선택되는  queue 에서,
					// 현재 실행 중인 요청(ptr_cur_cio_packet->get_action()) 과
					// 새로운 요청(req.get_action()) 이 같은 경우는 cio_packet::act_dev_read 일 경우 밖에 없음.
					// 왜냐면, 나머지는 start_X 후 동기식 처럼 처리하기 때문에.
					// cio_packet::act_dev_read 경우, 현재 cio_packet::act_dev_read 은 하단 api_briage 에서 cancel 처리 되므로, 기존 사용 중인
					// session number 를 소우한 프로세스에서 콜백을 바꾸기 위해 고의로 새로운 cio_packet::act_dev_read 를 전송했다고 보고,
					// 그냥 하단 api_briage 로 보내서 처리 함.
					// 현재 cio_packet::act_dev_read 를 처리 중인데, cio_packet::act_dev_read 외의 새로운 명령의 경우, b_save_for_recover 가 true로
					// 설정되어 호출되기 때문에 아래 코드는 콜백함수에서 복구를 위해 현재 cio_packet::act_dev_read 저장하는 것임. 
					//
					if (!m_b_shared_mode) {
						continue;
					}
					if (!b_save_for_recover) {
						continue;
					}

					//q 에서 제거하지 않고, 현재 item을 얻어 복구를 위해 저장.
					if (ptr_cur_cio_packet){
						// 현재 명령이 cancel 이면 같은 session 의 현재 명령은 복구하지 않는다.
						if (act != cio_packet::act_dev_cancel) {
							ptr_cur_cio_packet->set_recover_reserve(true);
							ptr_q->save_for_recover(cur_front_item);
						}
					}

					// 응답 공유 모드에서는 다른 session 큐에 있는 것들은 복구를 위해 저장해야함.
					cdev_ctl_fn::_type_map_ptr_q::iterator it_another_ptr_q = m_map_ptr_q_ptr_cur_req.begin();
					for (; it_another_ptr_q != m_map_ptr_q_ptr_cur_req.end(); ++it_another_ptr_q) {
						if (it_another_ptr_q->first == n_session_number) {
							continue;//이 것은 위 코드에서 이미 처리됨.
						}
						//
						cdev_ctl_fn::_cq::type_ptr ptr_another_q = it_another_ptr_q->second;
						cdev_ctl_fn::_cq::type_tuple_full another_front_item = ptr_another_q->pop_front(false);
						cio_packet::type_ptr ptr_another_cio_packet = std::get<0>(another_front_item);
						if (!ptr_another_cio_packet) {
							continue;
						}

						ptr_another_cio_packet->set_recover_reserve(true);
						ptr_another_q->save_for_recover(another_front_item);
					}//end for
				} while (false);


				// push new item to queue.
				std::tie(ptr_evt, n_evt, ptr_response) = ptr_q->push_back(cio_packet(req));
			} while (false);

			return std::make_tuple(ptr_evt,n_evt, ptr_response);
		}

		bool cdev_ctl_fn::_push_back_request_for_response_check_running_read_request(const cio_packet& req)
		{
			bool b_running_read(false);

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_map);
				cio_packet::type_act act = req.get_action();
				//
				switch (act) {
				case cio_packet::act_dev_read:
					break;
				default:
					continue;// unsupported type.
				}//end switch
				//
				// push 하기 전애 read 가 실행 중인지 검사.
				cdev_ctl_fn::_type_map_ptr_q::iterator it_ptr_q = m_map_ptr_q_ptr_cur_req.begin();
				cio_packet::type_ptr ptr_req;

				for (; it_ptr_q != m_map_ptr_q_ptr_cur_req.end(); ++it_ptr_q) {
					cdev_ctl_fn::_cq::type_ptr ptr_q = it_ptr_q->second;
					if (!ptr_q) {
						continue;
					}

					if (ptr_q->empty()) {
						continue;
					}

					std::tie(ptr_req, std::ignore, std::ignore, std::ignore) = ptr_q->pop_front(false);
					if (!ptr_req) {
						continue;
					}

					if (ptr_req->get_action() == cio_packet::act_dev_read) {
						b_running_read = true;
						break;
					}
				}//end for

				// build new item for pushing
				unsigned long n_session_number = req.get_session_number();
				cdev_ctl_fn::_type_map_ptr_q::iterator it_cur_ptr_q = m_map_ptr_q_ptr_cur_req.emplace(n_session_number, std::make_shared<cdev_ctl_fn::_cq>(n_session_number)).first;

				// push new item to queue.
				std::tie(std::ignore, std::ignore, std::ignore) = it_cur_ptr_q->second->push_back(cio_packet(req));
			} while (false);

			return b_running_read;
		}

		cdev_ctl_fn::_cq::type_tuple_full cdev_ctl_fn::_pop_front_cur_reqeust_for_response(unsigned long n_session_number,bool b_remove)
		{
			cwait::type_ptr ptr_evt;
			cio_packet::type_ptr ptr_req, ptr_rsp;
			int n_evt_index(-1);


			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_map);
				cdev_ctl_fn::_type_map_ptr_q::iterator it_ptr_q = m_map_ptr_q_ptr_cur_req.find(n_session_number);
				if (it_ptr_q == m_map_ptr_q_ptr_cur_req.end()) {
					continue;
				}

				if (!(it_ptr_q->second)) {
					continue;
				}
				if (it_ptr_q->second->empty()) {
					continue;
				}
				std::tie(ptr_req, ptr_evt, n_evt_index, ptr_rsp) = it_ptr_q->second->pop_front(b_remove);
			} while (false);
			return std::make_tuple(ptr_req, ptr_evt, n_evt_index, ptr_rsp);
		}

		std::list<cdev_ctl_fn::_cq::type_tuple_full> cdev_ctl_fn::_pop_front_cur_reqeust_for_response_as_act(cio_packet::type_act act,bool b_remove)
		{
			std::list<cdev_ctl_fn::_cq::type_tuple_full> set_out;

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_map);
				cdev_ctl_fn::_type_map_ptr_q::iterator it_ptr_q = m_map_ptr_q_ptr_cur_req.begin();

				for (; it_ptr_q != m_map_ptr_q_ptr_cur_req.end(); ++it_ptr_q) {
					cdev_ctl_fn::_cq::type_ptr ptr_q = it_ptr_q->second;
					if (!ptr_q) {
						continue;
					}
					if (ptr_q->empty()) {
						continue;
					}

					auto tu = ptr_q->pop_front(b_remove);
					if (std::get<0>(tu)->get_action() == act) {
						set_out.emplace_back(tu);
					}
				}//end for
			} while (false);
			return set_out;
		}

		std::vector<cdev_ctl_fn::_cq::type_tuple_full> cdev_ctl_fn::_pop_front_and_remove_cur_reqeust_for_response_as_act(
			cio_packet::type_act act, cqitem_dev& qi
		)
		{
			std::vector<cdev_ctl_fn::_cq::type_tuple_full> v_out;
			cio_packet::type_ptr ptr_rsp;

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_map);
				cdev_ctl_fn::_type_map_ptr_q::iterator it_ptr_q = m_map_ptr_q_ptr_cur_req.begin();

				for (; it_ptr_q != m_map_ptr_q_ptr_cur_req.end(); ++it_ptr_q) {
					cdev_ctl_fn::_cq::type_ptr ptr_q = it_ptr_q->second;
					if (!ptr_q) {
						continue;
					}
					if (ptr_q->empty()) {
						continue;
					}

					auto tu = ptr_q->pop_front(false);//pop front and not remove
					if (std::get<0>(tu)->get_action() == act) {
						// found act.
						ptr_rsp = std::get<3>(tu);
						if (_set_response(*ptr_rsp, qi, *(std::get<0>(tu)))) {
							v_out.emplace_back(tu);
							ptr_q->remove_front();//remove
						}
					}
				}//end for
			} while (false);
			return v_out;
		}

		std::set<std::pair<unsigned long, cio_packet::type_act>> cdev_ctl_fn::_remove_front_for_response(unsigned long n_session_number, bool b_recover_on_all_q)
		{
			std::set<std::pair<unsigned long, cio_packet::type_act>> set_recoverd_session_number_and_act;

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_map);
				cdev_ctl_fn::_type_map_ptr_q::iterator it_ptr_q = m_map_ptr_q_ptr_cur_req.begin();

				for (; it_ptr_q != m_map_ptr_q_ptr_cur_req.end(); ++it_ptr_q) {
					cdev_ctl_fn::_cq::type_ptr ptr_q = it_ptr_q->second;
					if (!ptr_q) {
						continue;
					}

					if (it_ptr_q->first == n_session_number) {
						if (!ptr_q->empty()) {
							ptr_q->remove_front();
						}
					}
					//
					if (b_recover_on_all_q) {
						if (ptr_q->recover_to_front()) {
							auto tuple_of_front = ptr_q->pop_front(false);

							set_recoverd_session_number_and_act.insert(
								std::make_pair(it_ptr_q->first,std::get<0>(tuple_of_front)->get_action() )
							);
						}
					}

				}//end for
			} while (false);
			return set_recoverd_session_number_and_act;
		}

		bool cdev_ctl_fn::is_running_read_request()
		{
			bool b_reading(false);

			std::lock_guard<std::mutex> lock(m_mutex_for_map);
			cdev_ctl_fn::_type_map_ptr_q::iterator it_ptr_q = m_map_ptr_q_ptr_cur_req.begin();
			cio_packet::type_ptr ptr_req;

			for (; it_ptr_q != m_map_ptr_q_ptr_cur_req.end(); ++it_ptr_q) {
				cdev_ctl_fn::_cq::type_ptr ptr_q = it_ptr_q->second;
				if (!ptr_q) {
					continue;
				}

				if (!ptr_q->empty()) {
					continue;
				}

				std::tie(ptr_req,std::ignore,std::ignore, std::ignore) = ptr_q->pop_front(false);
				if (!ptr_req) {
					continue;
				}

				if (ptr_req->get_action() == cio_packet::act_dev_read) {
					b_reading = true;
					break;
				}
			}//end for
			return b_reading;
		}

		const std::wstring cdev_ctl_fn::_get_device_path() const
		{
			return m_s_dev_path;
		}

		bool cdev_ctl_fn::_cb_dev_read_on_exclusive(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			cio_packet::type_ptr ptr_req, ptr_rsp;
			cwait::type_ptr ptr_evt;
			int n_evt_index(-1);

			do {
				std::tie(ptr_req, ptr_evt, n_evt_index, ptr_rsp) = p_obj->_pop_front_cur_reqeust_for_response(qi.get_session_number(), false);
				if (!ptr_req) {
					continue;// 요청한 session 의 응답이 아니면, 무시.
				}

				if (!p_obj->_set_response(*ptr_rsp, qi, *ptr_req)) {
					//현재 요청은 아직 결과를 몰라서 큐에서 삭제하면 안됌.
					b_complete = false;
					continue;//more processing
				}

				// exclusive 에서는 back 이 없으므로 recover 도 필요 없음.
				auto set_recover = p_obj->_remove_front_for_response(qi.get_session_number(), false);// 현재 요청 큐에서 삭제. and NO recover!
				//
				if (ptr_evt) {
					ptr_evt->set(n_evt_index);
				}

				type_v_buffer v_rsp;
				ptr_rsp->get_packet_by_json_format(v_rsp);

				if (!cserver::get_instance().send_data_to_client_by_ip4(v_rsp, ptr_rsp->get_session_number())) {
					//p_obj->p_log->trace( L"_callback_request_result : error send data : session = %u : device_index = %u", n_session, n_device_index);
				}

			} while (false);


			return b_complete;
		}

		bool cdev_ctl_fn::_cb_dev_read_on_shared(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			cio_packet::type_ptr ptr_req, ptr_rsp;
			cwait::type_ptr ptr_evt;
			int n_evt_index(-1);


			do {
				// 현재 실행 중인 act_dev_read 요청을 모든 session 큐에서 제거 하고 응답을 만들어서 얻어 온다.
				std::vector<cdev_ctl_fn::_cq::type_tuple_full> v_tuple = p_obj->_pop_front_and_remove_cur_reqeust_for_response_as_act(cio_packet::act_dev_read, qi);
				if (v_tuple.empty()) {
					//현재 요청은 아직 결과를 몰라서 큐에서 삭제하면 안됌.
					b_complete = false;
					continue;//more processing
				}

				// 모든 session 에 대한 응답 생성.
				// 0 - request ptr, 1 - event ptr, 2 - event index, 3 - response ptr.

				// 동일한 결과를 broadcast 할 거라 첫 응답만 고려해서 전체에 broadcast.
				ptr_rsp = std::get<3>(v_tuple[0]);
				ptr_req = std::get<0>(v_tuple[0]);

				if (ptr_rsp->is_cancel()) {
					if (ptr_req->is_recover_reserved()) {
						b_complete = false;
						continue;// recover 후, response 할 것임.
					}
				}

				//
				type_v_buffer v_rsp;
				ptr_rsp->get_packet_by_json_format(v_rsp);

				// 응답을 broadcast
				for (auto tu : v_tuple) {
					if (!cserver::get_instance().send_data_to_client_by_ip4(v_rsp, std::get<3>(tu)->get_session_number())) {
						//p_obj->p_log->trace( L"_callback_request_result : error send data : session = %u : device_index = %u", n_session, n_device_index);
					}
				}//end for

			} while (false);

			return b_complete;
		}

		bool cdev_ctl_fn::_cb_dev_for_sync_req(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			cio_packet::type_ptr ptr_req, ptr_rsp;
			cwait::type_ptr ptr_evt;
			int n_evt_index(-1);

			do {
				std::tie(ptr_req, ptr_evt, n_evt_index, ptr_rsp) = p_obj->_pop_front_cur_reqeust_for_response(qi.get_session_number(), false);
				if (!ptr_req) {
					continue;// 요청한 session 의 응답이 아니면, 무시.
				}

				if (!p_obj->_set_response(*ptr_rsp, qi, *ptr_req)) {
					//현재 요청은 아직 결과를 몰라서 큐에서 삭제하면 안됌.
					b_complete = false;
					continue;//more processing
				}

				// 현재 요청 큐에서 삭제. and 모든 backup 은 recover!
				// 현재 shared open 이 아니면, recover 를 요청해도 backup 된 것이 없어서 recover 되지 않음.
				// 현재 shared open 이면, backup 된 모든 read request 는 recover 됨.
				auto set_recover = p_obj->_remove_front_for_response(qi.get_session_number(),true);
				if (!set_recover.empty()) {
					// 복구 된 것이 있으면 재실행 필요. 아마 read request 명령 일거임.
					// 다수의 세션에서 read 가 복구 되어도 아무거나 하나의 세션 read 만 재실행 하면, 응답 받을때,
					// 모든 대기 중인 read 세션에 대해 응답을 broadcast 할 거임.
					// 여기느 아래 event set 할 때 까지 세로운 명령 받는 루틴은 lock 되어 있어서 thread 안전함.
					// 따라서 그냥 실행 해도 안전.
					do {
						bool b_exist_not_read(false);

						for (auto re_item : set_recover) {
							if (re_item.second != cio_packet::act_dev_read) {
								b_exist_not_read = true;
								break;
							}
						}//end for

						if (b_exist_not_read) {
							// recover 된 것에 read 가 아니애가 있음. 설계상 오류로 검토가 필요함.
#if defined(_WIN32) && defined(_DEBUG)
							ATLTRACE(L"recover 된 것에 read 가 아니애가 있음. 설계상 오류로 검토가 필요함.\n");
#endif
							continue;
						}
						_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
						if (!lib_hid.is_ini()) {
#if defined(_WIN32) && defined(_DEBUG)
							ATLTRACE(L"recover 실패. : _mp::clibhid::get_instance() .\n");
#endif
							continue;
						}
						clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(p_obj->_get_device_path());
						if (wptr_dev.expired()) {
#if defined(_WIN32) && defined(_DEBUG)
							ATLTRACE(L"recover 실패. : wptr_dev.expired().\n");
#endif
							continue;
						}

#if defined(_WIN32) && defined(_DEBUG)
						ATLTRACE(L"++ %09u + _cb_dev_for_sync_req-start_read(_cb_dev_read_on_shared)\n", qi.get_session_number());
#endif
						wptr_dev.lock()->start_read(cdev_ctl_fn::_cb_dev_read_on_shared, p_obj, qi.get_session_number());

					} while (false);

				}
				//
				if (ptr_evt) {
					ptr_evt->set(n_evt_index);
				}

			} while (false);


			return b_complete;
		}

		/**
		* callback of clibhid_dev.start_cancel.
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		bool cdev_ctl_fn::_cb_dev_cancel(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			cio_packet::type_ptr ptr_req, ptr_rsp;
			cwait::type_ptr ptr_evt;
			int n_evt_index(-1);

			do {
				std::tie(ptr_req, ptr_evt, n_evt_index, ptr_rsp) = p_obj->_pop_front_cur_reqeust_for_response(qi.get_session_number(), false);
				if (!ptr_req) {
					continue;// 요청한 session 의 응답이 아니면, 무시.
				}

				if (!p_obj->_set_response(*ptr_rsp, qi, *ptr_req)) {
					b_complete = false;
					continue;//more processing
				}
				//

				// 현재 요청 큐에서 삭제. and 모든 backup 은 recover!
				// 현재 shared open 이 아니면, recover 를 요청해도 backup 된 것이 없어서 recover 되지 않음.
				// 현재 shared open 이면, backup 된 모든 read request 는 recover 됨.
				auto set_recover = p_obj->_remove_front_for_response(qi.get_session_number(), true);
				if (!set_recover.empty()) {
					// 복구 된 것이 있으면 재실행 필요. 아마 read request 명령 일거임.
					// 다수의 세션에서 read 가 복구 되어도 아무거나 하나의 세션 read 만 재실행 하면, 응답 받을때,
					// 모든 대기 중인 read 세션에 대해 응답을 broadcast 할 거임.
					// 여기느 아래 event set 할 때 까지 세로운 명령 받는 루틴은 lock 되어 있어서 thread 안전함.
					// 따라서 그냥 실행 해도 안전.
					do {
						bool b_exist_not_read(false);

						for (auto re_item : set_recover) {
							if (re_item.second != cio_packet::act_dev_read) {
								b_exist_not_read = true;
								break;
							}
						}//end for

						if (b_exist_not_read) {
							// recover 된 것에 read 가 아니애가 있음. 설계상 오류로 검토가 필요함.
#if defined(_WIN32) && defined(_DEBUG)
							ATLTRACE(L"recover 된 것에 read 가 아니애가 있음. 설계상 오류로 검토가 필요함.\n");
#endif
							continue;
						}
						_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
						if (!lib_hid.is_ini()) {
#if defined(_WIN32) && defined(_DEBUG)
							ATLTRACE(L"recover 실패. : _mp::clibhid::get_instance() .\n");
#endif
							continue;
						}
						clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(p_obj->_get_device_path());
						if (wptr_dev.expired()) {
#if defined(_WIN32) && defined(_DEBUG)
							ATLTRACE(L"recover 실패. : wptr_dev.expired().\n");
#endif
							continue;
						}

#if defined(_WIN32) && defined(_DEBUG)
						ATLTRACE(L"++ %09u + _cb_dev_for_sync_req-start_read(_cb_dev_read_on_shared)\n", qi.get_session_number());
#endif
						wptr_dev.lock()->start_read(cdev_ctl_fn::_cb_dev_read_on_shared, p_obj, qi.get_session_number());

					} while (false);

				}
				//
				if (ptr_evt) {
					ptr_evt->set(n_evt_index);
				}
			} while (false);

			return b_complete;
		}

		////////////////////////////////////////////////
		// inner queue section

		cdev_ctl_fn::_cq::_cq(unsigned long n_session_number) :
			m_n_session_number(n_session_number)
		{
		}

		cdev_ctl_fn::_cq::~_cq()
		{
		}

		std::tuple<cwait::type_ptr,int, cio_packet::type_ptr> cdev_ctl_fn::_cq::push_back(const cio_packet& req)
		{
			cwait::type_ptr ptr_evt;
			int n_evt(-1);
			cio_packet::type_ptr ptr_rsp = std::make_shared<cio_packet>();

			do {
				//std::lock_guard<std::mutex> lock(m_mutex);
				cio_packet::type_ptr ptr_req = std::make_shared<cio_packet>(req);
				ptr_evt = std::make_shared<cwait>();
				

				// 생성 후, 첫 event 생성이므로 생성된 event index 는 항상 0.
				n_evt = ptr_evt->generate_new_event(); // return waits 0( construct and the first generation)
				m_q.push_back(std::make_tuple(ptr_req, ptr_evt, n_evt, ptr_rsp));
			} while (false);
			return std::make_tuple(ptr_evt,n_evt, ptr_rsp);
		}

		cdev_ctl_fn::_cq::type_tuple_full cdev_ctl_fn::_cq::pop_front( bool b_remove )
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

		void cdev_ctl_fn::_cq::remove_front()
		{
			do {
				//std::lock_guard<std::mutex> lock(m_mutex);

				if (m_q.empty()) {
					continue;
				}

				m_q.pop_front();
				//
			} while (false);
		}

		bool cdev_ctl_fn::_cq::empty()
		{
			return m_q.empty();
		}

		bool cdev_ctl_fn::_cq::save_for_recover(const _cq::type_tuple_full& item)
		{
			bool b_result(false);

			do {
				if (std::get<0>(m_must_be_recoverd_read_item)) {
					continue;// already saved item
				}

				m_must_be_recoverd_read_item = item;

#if defined(_WIN32) && defined(_DEBUG)
				std::wostringstream ss_out;
				auto ptr_req_cio_packet = std::get<0>(m_must_be_recoverd_read_item);
				if (ptr_req_cio_packet) {
					ss_out << L" saved session : ";
					ss_out << std::dec << ptr_req_cio_packet->get_session_number()
						<< L":" << cio_packet::get_action_code_by_string(ptr_req_cio_packet->get_action())
						<< L", ";

					ATLTRACE(L"++ %ls\n", ss_out.str().c_str());
				}
#endif
				//
				b_result = true;
			} while (false);
			return b_result;
		}

		bool cdev_ctl_fn::_cq::save_for_recover(const cio_packet::type_ptr& ptr_req, const cwait::type_ptr& ptr_evt, int n_event, const cio_packet::type_ptr& ptr_rsp)
		{
			return save_for_recover(std::make_tuple(ptr_req, ptr_evt, n_event, ptr_rsp));
		}

		bool cdev_ctl_fn::_cq::recover_to_front()
		{
			bool b_result(false);

			do {
				cio_packet::type_ptr ptr_req_cio_packet = std::get<0>(m_must_be_recoverd_read_item);

				if (!ptr_req_cio_packet) {
					continue;// none saved item
				}

				ptr_req_cio_packet->set_recover_reserve(false); // recover 했음으로 삭제.
				m_q.push_front(m_must_be_recoverd_read_item);

#if defined(_WIN32) && defined(_DEBUG)
				std::wostringstream ss_out;

				if (ptr_req_cio_packet) {
					ss_out << L" recovered session : ";
					ss_out << std::dec << ptr_req_cio_packet->get_session_number()
						<< L":" << cio_packet::get_action_code_by_string(ptr_req_cio_packet->get_action())
						<< L", ";

					ATLTRACE(L"++ %ls\n", ss_out.str().c_str());
				}
#endif
				std::get<0>(m_must_be_recoverd_read_item).reset();
				std::get<1>(m_must_be_recoverd_read_item).reset();
				std::get<2>(m_must_be_recoverd_read_item) = -1;
				std::get<3>(m_must_be_recoverd_read_item).reset();
				//
				b_result = true;
			} while (false);
			return b_result;
		}



}//the end of _mp namespace

