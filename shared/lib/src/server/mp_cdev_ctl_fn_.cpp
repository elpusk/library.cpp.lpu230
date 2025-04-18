#include <websocket/mp_win_nt.h>

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <cassert>

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

		cdev_ctl_fn::cdev_ctl_fn(clog* p_log) : 
			cbase_ctl_fn(p_log),
			m_n_cnt_open(0),
			m_b_shared_mode(false)
		{
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

		cdev_ctl_fn::type_tuple_full cdev_ctl_fn::_cq_mgmt::sync_push_back(const cio_packet& request)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			auto r = m_q_sync_req_evt_rsp.push_back(request);

			//
			// m_map_ptr_q_ptr_cur_req_read 에 기존 read 명령은 device start_X 명령 실행으로
			// 하위 device io 에서 자동 취소 되서, 기존 read 명령 실행시 설정한 callback 이 cancel result 를 가지고 호출된다.
			// 따라서, m_map_ptr_q_ptr_cur_req_read 에 기존 명령이 있어도 자동으로 처리된다.

			return r;
		}

		cdev_ctl_fn::type_tuple_full cdev_ctl_fn::_cq_mgmt::sync_pop_front(bool b_remove)
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

		std::vector<cdev_ctl_fn::type_tuple_full> cdev_ctl_fn::_cq_mgmt::read_pop_front(unsigned long n_session_number,bool b_remove)
		{
			cwait::type_ptr ptr_evt;
			int n_evet(-1);
			cio_packet::type_ptr ptr_req, ptr_rsp;
			std::vector<cdev_ctl_fn::type_tuple_full> v_result;

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

		std::vector<cdev_ctl_fn::type_tuple_full> cdev_ctl_fn::_cq_mgmt::read_set_response_front(cqitem_dev& qi, bool b_also_cancel/*=false*/)
		{
			cwait::type_ptr ptr_evt;
			std::vector<cdev_ctl_fn::type_tuple_full> v_result;

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

		std::vector<cdev_ctl_fn::type_tuple_full> cdev_ctl_fn::_cq_mgmt::read_pop_front_remove_complete_of_all()
		{
			cwait::type_ptr ptr_evt;
			int n_evet(-1);
			cio_packet::type_ptr ptr_req, ptr_rsp;
			std::vector<cdev_ctl_fn::type_tuple_full> v_result;

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

		cdev_ctl_fn::type_tuple_full cdev_ctl_fn::_cq_mgmt::_cq::push_back(const cio_packet& req)
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

		cdev_ctl_fn::type_tuple_full cdev_ctl_fn::_cq_mgmt::_cq::pop_front( bool b_remove )
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

		//=======================================//=======================================
		//=======================================//=======================================
		//_cstatus_mgmt::_cstate member
		// m_trans_table_base 는 exclusive 또는 하나의 session 의 기본 state transaction table.
		const int cdev_ctl_fn::_cstatus_mgmt::_cstate::_m_trans_table_base[_cstatus_mgmt::_cstate::st_total][_cstatus_mgmt::_cstate::ev_total]
			= {
				{st_idl,st_not,st_not,st_not,st_not},
				{st_idl,st_not,st_idl,st_asy,st_idl},
				{st_asy,st_not,st_idl,st_asy,st_idl}
		};
		
		std::pair<bool, cdev_ctl_fn::_cstatus_mgmt::_cstate::type_state> cdev_ctl_fn::_cstatus_mgmt::_cstate::get_combination_state_in_map_except_selected_session
		(
			const _cstatus_mgmt::_cstate::type_map_ptr_state& map_ptr_state_cur,
			unsigned long n_selected_session_number
		)
		{
			bool b_exist(false);
			_cstatus_mgmt::_cstate::type_state st_result(_cstatus_mgmt::_cstate::st_not);

			do {
				if (map_ptr_state_cur.empty()) {
					continue;
				}

				_cstatus_mgmt::_cstate::type_map_ptr_state::const_iterator it_sel = map_ptr_state_cur.find(n_selected_session_number);
				if (it_sel != map_ptr_state_cur.cend()) {
					if (map_ptr_state_cur.size() == 1) {
						continue; // n_session 의 state 만 존재하는 경우. - another session is none.
					}
				}

				b_exist = true; //there are another session.
				st_result = _cstatus_mgmt::_cstate::st_not;

				_cstatus_mgmt::_cstate::type_map_ptr_state::const_iterator it = map_ptr_state_cur.begin();
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

		cdev_ctl_fn::_cstatus_mgmt::_cstate::type_evt cdev_ctl_fn::_cstatus_mgmt::_cstate::get_event_from_request_action(cio_packet::type_act act)
		{
			cdev_ctl_fn::_cstatus_mgmt::_cstate::type_evt evt(_cstatus_mgmt::_cstate::ev_none);

			switch (act)
			{
			case cio_packet::act_dev_open:
				evt = _cstatus_mgmt::_cstate::ev_open;
				break;
			case cio_packet::act_dev_close:
				evt = _cstatus_mgmt::_cstate::ev_close;
				break;
			case cio_packet::act_dev_transmit:
			case cio_packet::act_dev_cancel:
			case cio_packet::act_dev_write:
				evt = _cstatus_mgmt::_cstate::ev_sync;
				break;
			case cio_packet::act_dev_read:
				evt = _cstatus_mgmt::_cstate::ev_asy;
				break;
			case cio_packet::act_dev_sub_bootloader:
			default:
				break;
			}//end switch
			return evt;
		}

		int cdev_ctl_fn::_cstatus_mgmt::_cstate::get_mask_from_state(_cstate::type_state st)
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

		cdev_ctl_fn::_cstatus_mgmt::_cstate::type_state cdev_ctl_fn::_cstatus_mgmt::_cstate::get_state_from_mask(int n_mask)
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

		cdev_ctl_fn::_cstatus_mgmt::_cstate::_cstate()
		{
			this->reset();
		}

		cdev_ctl_fn::_cstatus_mgmt::_cstate::_cstate(const _cstatus_mgmt::_cstate& src)
		{
			*this = src;
		}

		cdev_ctl_fn::_cstatus_mgmt::_cstate& cdev_ctl_fn::_cstatus_mgmt::_cstate::operator=(const _cstatus_mgmt::_cstate& src)
		{
			m_st_cur = src.m_st_cur;
			m_ev_last = src.m_ev_last;
			return *this;
		}
		cdev_ctl_fn::_cstatus_mgmt::_cstate::~_cstate()
		{
		}

		void cdev_ctl_fn::_cstatus_mgmt::_cstate::reset()
		{
			m_st_cur = _cstate::st_not;
			m_ev_last = _cstate::ev_none;
		}

		cdev_ctl_fn::_cstatus_mgmt::_cstate::type_state cdev_ctl_fn::_cstatus_mgmt::_cstate::get() const
		{
			return m_st_cur;
		}

		cdev_ctl_fn::_cstatus_mgmt::_cstate::type_evt cdev_ctl_fn::_cstatus_mgmt::_cstate::get_last_event() const
		{
			return m_ev_last;
		}

		cdev_ctl_fn::_cstatus_mgmt::_cstate::type_result_evt   cdev_ctl_fn::_cstatus_mgmt::_cstate::set(_cstate::type_evt evt)
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
		////_cstatus_mgmt::_cresult  member
		cdev_ctl_fn::_cstatus_mgmt::_cresult::_cresult()
		{
			reset();
		}

		cdev_ctl_fn::_cstatus_mgmt::_cresult::_cresult(const cdev_ctl_fn::_cstatus_mgmt::_cresult& src)
		{
			*this = src;
		}

		cdev_ctl_fn::_cstatus_mgmt::_cresult::_cresult(const cio_packet& request)
		{
			reset();
			process_set_req_packet(request);
		}

		cdev_ctl_fn::_cstatus_mgmt::_cresult& cdev_ctl_fn::_cstatus_mgmt::_cresult::operator=(const cdev_ctl_fn::_cstatus_mgmt::_cresult& src)
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

		void cdev_ctl_fn::_cstatus_mgmt::_cresult::reset()
		{
			m_b_process_exist_session = false;

			m_st_session_old = _cstatus_mgmt::_cstate::st_not;
			m_st_session_new = _cstatus_mgmt::_cstate::st_not;

			m_st_combination_old = _cstatus_mgmt::_cstate::st_not;
			m_st_combination_new = _cstatus_mgmt::_cstate::st_not;

			m_b_process_result = false;
			m_b_process_complete = true;

			m_s_dev_path.clear();
		}

		const cio_packet::type_ptr& cdev_ctl_fn::_cstatus_mgmt::_cresult::get_req() const
		{
			return m_ptr_req;
		}

		unsigned long cdev_ctl_fn::_cstatus_mgmt::_cresult::get_session_number() const
		{
			unsigned long n_session(_MP_TOOLS_INVALID_SESSION_NUMBER);
			if (m_ptr_req) {
				n_session = m_ptr_req->get_session_number();
			}
			return n_session;
		}


		cdev_ctl_fn::_cstatus_mgmt::_cstate::type_evt cdev_ctl_fn::_cstatus_mgmt::_cresult::get_cur_event() const
		{
			_cstate::type_evt evt(_cstate::ev_none);

			do {
				if (!m_ptr_req) {
					continue;
				}
				evt = _cstatus_mgmt::_cstate::get_event_from_request_action(m_ptr_req->get_action());

			} while (false);
			return evt;
		}

		type_pair_bool_result_bool_complete cdev_ctl_fn::_cstatus_mgmt::_cresult::process_get_result() const
		{
			return std::make_pair(m_b_process_result, m_b_process_complete);
		}

		cio_packet::type_ptr cdev_ctl_fn::_cstatus_mgmt::_cresult::process_set_req_packet(const cio_packet& request)
		{
			m_ptr_req = std::make_shared<cio_packet>(request);
			m_ptr_rsp = std::make_shared<cio_packet>(request);

			m_ptr_rsp->set_cmd(cio_packet::cmd_response).set_data_error();

			return m_ptr_rsp;
		}

		void cdev_ctl_fn::_cstatus_mgmt::_cresult::process_set_rsp_with_error_complete
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

		void cdev_ctl_fn::_cstatus_mgmt::_cresult::process_set_rsp_with_succss_complete(const _mp::type_v_buffer& v_rx, const std::wstring& s_dev_path)
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
					m_ptr_rsp->set_data(v_rx,true);
				}
				//
			} while (false);

			m_b_process_result = true;
			m_b_process_complete = true;
		}

		void cdev_ctl_fn::_cstatus_mgmt::_cresult::process_set_rsp_with_succss_complete(const std::wstring& s_data, const std::wstring& s_dev_path)
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

		void cdev_ctl_fn::_cstatus_mgmt::_cresult::process_set_rsp_with_succss_ing
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

		bool cdev_ctl_fn::_cstatus_mgmt::_cresult::is_changed_state_of_selected_session() const
		{
			if (m_st_session_old == m_st_session_new) {
				return false;
			}
			return true;
		}

		bool cdev_ctl_fn::_cstatus_mgmt::_cresult::is_changed_combination_state_of_all_session() const
		{
			if (m_st_combination_old == m_st_combination_new) {
				return false;
			}
			return true;
		}

		void cdev_ctl_fn::_cstatus_mgmt::_cresult::set_selected_session_state
		(
			_cstatus_mgmt::_cstate::type_state st_new,
			_cstatus_mgmt::_cstate::type_state st_old
		)
		{
			m_st_session_old = st_old;
			m_st_session_new = st_new;
		}

		void cdev_ctl_fn::_cstatus_mgmt::_cresult::set_selected_session_state(_cstatus_mgmt::_cstate::type_result_evt st_result)
		{
			m_st_session_new = std::get<1>(st_result);
			m_st_session_old = std::get<2>(st_result);
		}

		void cdev_ctl_fn::_cstatus_mgmt::_cresult::set_combination_state(_cstatus_mgmt::_cstate::type_state st_new, _cstatus_mgmt::_cstate::type_state st_old)
		{
			m_st_combination_old = st_old;
			m_st_combination_new = st_new;
		}

		std::pair<cdev_ctl_fn::_cstatus_mgmt::_cstate::type_state, cdev_ctl_fn::_cstatus_mgmt::_cstate::type_state> cdev_ctl_fn::_cstatus_mgmt::_cresult::get_combination_state() const
		{
			return std::make_pair(m_st_combination_new, m_st_combination_old);
		}

		//=======================================//=======================================
		//=======================================//=======================================
		//_cstatus_mgmt member

		/**
		* callback 에서 서버에 바로 전송하지 말자.
		*/
		bool cdev_ctl_fn::_cstatus_mgmt::_cb_dev_read_on_exclusive(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			_cstatus_mgmt* p_obj((_cstatus_mgmt*)p_user);

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
		bool cdev_ctl_fn::_cstatus_mgmt::_cb_dev_read_on_shared(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			_cstatus_mgmt* p_obj((_cstatus_mgmt*)p_user);

			do {
				//응답 공유 상태에서는 모든 read 에 대해 응답을 동일하게 설정해주어야 함.
				std::vector<cdev_ctl_fn::type_tuple_full> v_tuple = p_obj->m_mgmt_q.read_set_response_front(qi);
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
		bool cdev_ctl_fn::_cstatus_mgmt::_cb_dev_for_sync_req(cqitem_dev& qi, void* p_user)
		{
			//callback 에서 서버에 바로 전송하지 말자.
			bool b_complete(true);
			_cstatus_mgmt* p_obj((_cstatus_mgmt*)p_user);

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


		cdev_ctl_fn::_cstatus_mgmt::_cstatus_mgmt() : m_b_cur_shared_mode(false)
		{
			m_st_combi = _cstatus_mgmt::_cstate::st_not;
		}

		cdev_ctl_fn::_cstatus_mgmt::_cstatus_mgmt(bool b_shared_mode) :
			m_b_cur_shared_mode(b_shared_mode)
		{
			m_st_combi = _cstatus_mgmt::_cstate::st_not;
		}

		cdev_ctl_fn::_cstatus_mgmt::~_cstatus_mgmt()
		{
		}

		void cdev_ctl_fn::_cstatus_mgmt::reset(bool b_shared_mode)
		{
			m_b_cur_shared_mode = b_shared_mode;
			m_st_combi = _cstatus_mgmt::_cstate::st_not;
			m_map_ptr_state_cur.clear();
		}

		std::tuple<bool,cdev_ctl_fn::_cstatus_mgmt::_cstate::type_state, cdev_ctl_fn::_cstatus_mgmt::_cstate::type_state> cdev_ctl_fn::_cstatus_mgmt::get_cur_state(unsigned long n_session) const
		{
			bool b_exist(false);
			_cstatus_mgmt::_cstate st_cur;
			do {
				auto it = m_map_ptr_state_cur.find(n_session);
				if (it == std::end(m_map_ptr_state_cur)) {
					continue;
				}

				b_exist = true;
				st_cur = *it->second;
			} while (false);
			return std::make_tuple(b_exist,st_cur.get(), m_st_combi);
		}

		/**
		* @brief get the current combination state of all session.
		* @return the current combination state of all session.
		*/
		cdev_ctl_fn::_cstatus_mgmt::_cstate::type_state cdev_ctl_fn::_cstatus_mgmt::get_combination_cur_state() const
		{
			return m_st_combi;
		}

		cdev_ctl_fn::_cstatus_mgmt::_cresult cdev_ctl_fn::_cstatus_mgmt::process_event(	const cio_packet& request )
		{
			// 논린적으로 각 session 의 장비는 독립된 것으로 보기 때문에
			// 이벤트가 발생한 session 이 외의 session 의 state 에 영향은 없어야 함. 
			// result.b_process_complete 이 true 면, result.ptr_rsp 에 결과가 설정되어야 함.

			_cstatus_mgmt::_cresult result;
			auto evt = _cstatus_mgmt::_cstate::get_event_from_request_action(request.get_action());

			do {
				if (evt == _cstatus_mgmt::_cstate::ev_none) {
					continue;
				}

				if (m_b_cur_shared_mode) {
					result = _process_event_on_shared_mode(request);
				}
				else {
					result = _process_event_on_exclusive_mode(request);
				}

			} while (false);
			return result;
		}

		cdev_ctl_fn::_cstatus_mgmt::_cresult cdev_ctl_fn::_cstatus_mgmt::_process_event_on_shared_mode(const cio_packet& request)
		{
			// 논린적으로 각 session 의 장비는 독립된 것으로 보기 때문에
			// 이벤트가 발생한 session 이 외의 session 의 state 에 영향은 없어야 함.
			// result.b_process_complete 이 true 면, result.ptr_rsp 에 결과가 설정되어야 함.

			_cstatus_mgmt::_cresult result(request);
			_cstatus_mgmt::_cstate::type_state st_cur(_cstatus_mgmt::_cstate::st_not), st_another(_cstatus_mgmt::_cstate::st_not);

			auto it_sel = m_map_ptr_state_cur.find(request.get_session_number());
			if (it_sel != std::end(m_map_ptr_state_cur)) {

				st_cur = it_sel->second->get();// 현재 상태를 얻는다.
			}

			// 선택된 session 을 제외한 나머지 session 의 상태의 combination state를 얻는다.
			st_another = ? ? ;

			do {
				switch (st_cur)
				{
				case _cstatus_mgmt::_cstate::st_not:
					//result 설정을 하위 함수에서 설정.
					_process_shared_selected_session_st_not(result);
					break;
				case _cstatus_mgmt::_cstate::st_idl:
					//result 설정을 하위 함수에서 설정.
					_process_shared_selected_session_st_idl(result);
					break;
				case _cstatus_mgmt::_cstate::st_asy:
					//result 설정을 하위 함수에서 설정.
					_process_shared_selected_session_st_asy(result);
					break;
				default:
					// 에러 complete.
					result.b_process_complete = true;
					result.b_process_result = false;
					result.ptr_rsp->set_data_error();
					continue;
				}

			} while (false);
			return result;
		}

		cdev_ctl_fn::_cstatus_mgmt::_cresult cdev_ctl_fn::_cstatus_mgmt::_process_event_on_exclusive_mode(const cio_packet& request)
		{
			// 논린적으로 각 session 의 장비는 독립된 것으로 보기 때문에
			// 이벤트가 발생한 session 이 외의 session 의 state 에 영향은 없어야 함.
			// result.b_process_complete 이 true 면, result.ptr_rsp 에 결과가 설정되어야 함.
			
			_cstatus_mgmt::_cresult result(request);
			_cstatus_mgmt::_cstate::type_state st_cur(_cstatus_mgmt::_cstate::st_not);

			auto it_sel = m_map_ptr_state_cur.find(request.get_session_number());
			if (it_sel != std::end(m_map_ptr_state_cur)) {

				st_cur = it_sel->second->get();// 현재 상태를 얻는다.
			}

			do {
				switch (st_cur)
				{
				case _cstatus_mgmt::_cstate::st_not:
					//result 설정을 하위 함수에서 설정.
					_process_exclusive_st_not(result);
					break;
				case _cstatus_mgmt::_cstate::st_idl:
					//result 설정을 하위 함수에서 설정.
					_process_exclusive_st_idl(result);
					break;
				case _cstatus_mgmt::_cstate::st_asy:
					//result 설정을 하위 함수에서 설정.
					_process_exclusive_st_asy(result);
					break;
				default:
					// 에러 complete.
					result.process_set_rsp_with_error_complete(cio_packet::error_reason_session);
					continue;
				}

			} while (false);
			return result;
		}

		void cdev_ctl_fn::_cstatus_mgmt::_process_exclusive_st_not(_cstatus_mgmt::_cresult& result)
		{
			// result.b_process_complete 이 true 면, result.ptr_rsp 에 결과가 설정되어야 함.
			//excluisive mode 에서 open 이 안된 상태.
			
			_cstatus_mgmt::_cstate::type_evt evt = result.get_cur_event();
			
			// state 가 st_not 인 경우는 모두 sync 타입.

			do {
				std::wstring s_dev_path;
				bool b_user_shared_mode(false);

				switch (evt)
				{
				case _cstatus_mgmt::_cstate::ev_open:
					std::tie(s_dev_path, b_user_shared_mode) = cbase_ctl_fn::_get_device_path_from_req(result.get_req());
					if (s_dev_path.empty()) {
						result.process_set_rsp_with_error_complete(cio_packet::error_reason_device_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | dev_open() : session = %u.\n", __WFUNCTION__, result.get_session_number());
						continue; //error 일때만 result 를 _get_device_path_from_open_req 에서 설정함.
					}

					
					if (!cbase_ctl_fn::_open_device_by_req(s_dev_path, b_user_shared_mode, m_map_ptr_state_cur.size())) {
						result.process_set_rsp_with_error_complete(cio_packet::error_reason_device_path, s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());

						continue;
					}

					result.process_set_rsp_with_succss_complete(std::wstring(), s_dev_path);

					//이 경우 에러 일겨우 상태변경은 없으므로,
					// 에러 일때, _transit_state_by_processing_result() 를 호출 할 필요 없음. 
					_transit_state_by_processing_result( result);
					break;
				case _cstatus_mgmt::_cstate::ev_close:
				case _cstatus_mgmt::_cstate::ev_sync:
				case _cstatus_mgmt::_cstate::ev_asy:
					result.process_set_rsp_with_error_complete(cio_packet::error_reason_device_not_open);
					break;
				case _cstatus_mgmt::_cstate::ev_rx:
					// 무시되는 event
					break;
				default:
					continue;
				}//end switch
			} while (false);
		}
		void cdev_ctl_fn::_cstatus_mgmt::_process_exclusive_st_idl(_cstatus_mgmt::_cresult& result)
		{
			// result.b_process_complete 이 true 면, result.ptr_rsp 에 결과가 설정되어야 함.
			_cstatus_mgmt::_cstate::type_evt evt = result.get_cur_event();

			// state 가 st_idl 인 경우는 asy 를 제외하고 모두 sync 타입.

			auto it_sel = m_map_ptr_state_cur.find(result.get_session_number());

			do {

				switch (evt)
				{
				case _cstatus_mgmt::_cstate::ev_open:
					result.process_set_rsp_with_error_complete(cio_packet::error_reason_device_open);
					break;
				case _cstatus_mgmt::_cstate::ev_close:
					// 현재 상태가 IDL 이므로 해당 session 이 있으면, 해당 session 의 키와 값만 map 에서 삭제 하면됨.
					if (it_sel == std::end(m_map_ptr_state_cur)) {
						result.process_set_rsp_with_error_complete(cio_packet::error_reason_device_not_open);
						continue;
					}

					result.process_set_rsp_with_succss_complete(std::wstring(), m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);
					break;
				case _cstatus_mgmt::_cstate::ev_sync:
					if (!_start_and_complete_by_sync_req(result)) {
						continue;
					}
					_transit_state_by_processing_result(result);

					result.ptr_rsp->set_data_sucesss();
					result.b_process_result = true;
					break;
				case _cstatus_mgmt::_cstate::ev_asy:
					if (!_start_by_async_req(p_log, result)) {
						continue;
					}
					_transit_state_by_processing_result(result);
					result.ptr_rsp->set_data_sucesss();
					result.b_process_result = true;
					break;
				case _cstatus_mgmt::_cstate::ev_rx:
					break;
				default:
					continue;
				}//end switch
			} while (false);
		}

		void cdev_ctl_fn::_cstatus_mgmt::_process_exclusive_st_asy(_cstatus_mgmt::_cresult& result)
		{
			// result.b_process_complete 이 true 면, result.ptr_rsp 에 결과가 설정되어야 함.
			_cstatus_mgmt::_cstate::type_evt evt = _cstatus_mgmt::_cstate::get_event_from_request_action(result.ptr_req->get_action());

			// state 가 st_idl 인 경우는 asy 를 제외하고 모두 sync 타입.
			result.b_process_complete = true;

			auto it_sel = m_map_ptr_state_cur.find(result.ptr_req->get_session_number());

			do {

				switch (evt)
				{
				case _cstatus_mgmt::_cstate::ev_open:
					result.ptr_rsp->set_data_error();
					result.b_process_result = false;
					break;
				case _cstatus_mgmt::_cstate::ev_close:
					// 현재 상태가 IDL 이므로 해당 session 이 있으면, 해당 session 의 키와 값만 map 에서 삭제 하면됨.
					if (it_sel == std::end(m_map_ptr_state_cur)) {
						result.ptr_rsp->set_data_error();
						result.b_process_result = false;
						continue;
					}

					// 상태변경과 result 를 success 로 설정.
					_transit_state_by_processing_result(result);

					result.ptr_rsp->set_data_sucesss();
					result.b_process_result = true;
					break;
				case _cstatus_mgmt::_cstate::ev_sync:
					// syc 명령을 실행하면 하단에서 현재 실행 중인 async 를 취소해서 결과가 callback 에서 설정됨.
					if (!_start_and_complete_by_sync_req(result)) {
						continue;
					}
					_transit_state_by_processing_result(result);

					result.ptr_rsp->set_data_sucesss();
					result.b_process_result = true;
					break;
				case _cstatus_mgmt::_cstate::ev_asy:
					// asyc 명령을 실행하면 하단에서 현재 실행 중인 async 를 취소해서 결과가 callback 에서 설정됨.
					if (!_start_by_async_req(p_log, result)) {
						continue;
					}
					_transit_state_by_processing_result(result);
					result.ptr_rsp->set_data_sucesss();
					result.b_process_result = true;
					break;
				case _cstatus_mgmt::_cstate::ev_rx:
					break;
				default:
					continue;
				}//end switch
			} while (false);
		}


		size_t cdev_ctl_fn::_cstatus_mgmt::get_the_number_of_session() const
		{
			return m_map_ptr_state_cur.size();
		}

		bool cdev_ctl_fn::_cstatus_mgmt::is_shared_mode() const
		{
			return m_b_cur_shared_mode;
		}

		void cdev_ctl_fn::_cstatus_mgmt::_transit_state_by_processing_result
		(
			cdev_ctl_fn::_cstatus_mgmt::_cresult &result
		)
		{
			_cstatus_mgmt::_cstate::type_result_evt result_state(false,_cstatus_mgmt::_cstate::st_not, _cstatus_mgmt::_cstate::st_not);

			//
			unsigned long n_session = result.get_session_number();
			_cstatus_mgmt::_cstate::type_map_ptr_state::iterator it_sel = m_map_ptr_state_cur.find(n_session);

			if (result.get_cur_event() == _cstatus_mgmt::_cstate::ev_open) {
				// 이 함수는 request 가 성공 일때만  호출되므로, session 이 map 에 없는 경우는 open request 일때 밖에 없음.
				if (it_sel == std::end(m_map_ptr_state_cur)) {
					// open request 이므로 세션에 대한 state 를 map 에 생성.
					std::tie(it_sel, std::ignore) = m_map_ptr_state_cur.emplace(n_session, std::make_shared<_cstatus_mgmt::_cstate>());
				}
				result_state = it_sel->second->set(result.get_cur_event());
			}
			else if (result.get_cur_event() == _cstatus_mgmt::_cstate::ev_close) {
				result_state = it_sel->second->set(result.get_cur_event());

				m_map_ptr_state_cur.erase(it_sel); //remove session
				//
				if (m_map_ptr_state_cur.empty()) {
					reset(false);// set default mode(exclusive mode)
					m_s_dev_path.clear();
				}
			}
			else {
				result_state = it_sel->second->set(result.get_cur_event());
			}

			result.set_selected_session_state(result_state);

			if (!m_b_cur_shared_mode) {
				// exclusive mode 에서는 session 이 하나 이므로 combination state 와 session state 가 항상 같다.
				result.set_combination_state(std::get<1>(result_state), std::get<2>(result_state));
				// the end of exclusive mode
			}
			else {
				// shared mode 
				// generate combinatin state.
				auto exist_st = _get_state_another(n_session);
				if (exist_st.first) {
					result.set_combination_state(exist_st.second, m_st_combi);
				}
				else {// session 이 selected session 만 있음.
					result.set_combination_state(std::get<1>(result_state), m_st_combi);
				}
			}
			std:tie(m_st_combi, std::ignore) = result.get_combination_state();
		}

		/**
		* callback of clibhid_dev.start_write, cancel, write_read.
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		bool cdev_ctl_fn::_cstatus_mgmt::_cb_dev_for_self_cancel(cqitem_dev& qi, void* p_user)
		{
			//callback 에서 서버에 바로 전송하지 말자.
			bool b_complete(true);
			cdev_ctl_fn::_cstatus_mgmt* p_obj((cdev_ctl_fn::_cstatus_mgmt*)p_user);

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

		bool cdev_ctl_fn::_cstatus_mgmt::_close_by_close_req(clog* p_log, _cstatus_mgmt::_cresult& result)
		{
			bool b_result(false);

			result.ptr_rsp->set_data_error();
			result.b_process_result = false;

			do {
				if (m_map_ptr_state_cur.empty()) {
					continue; // 열린 session 없음.
				}
				unsigned long n_session = result.ptr_req->get_session_number();
				_cstatus_mgmt::_cstate::type_map_ptr_state::iterator it_sel = m_map_ptr_state_cur.find(n_session);

				if (it_sel == m_map_ptr_state_cur.end()) {
					// 해당 session 없음.
					continue;
				}

				// 해당 session 이 있으나 _transit_state_by_processing_result() 에서 해당 session 의 key & value 를 map에서 삭제.

				_cstatus_mgmt::_cstate::type_ptr ptr_state = it_sel->second;// get current session state!
				// 선택된 session 만 ASY 상태면, client 에 응답을 하지 않는 내부 cancel request 만들어, 현재 ASY 를 cancel 로 client 에게 응답하도록 함.
				// 선택된 session 만 외에도 ASY 이 존재하면, 현재 ASY 를 cancel 로 client 에게 응답하도록 함.

				_cstatus_mgmt::_cstate::type_state sel_state = ptr_state->get();

				if (sel_state == _cstatus_mgmt::_cstate::st_asy) {
					
					std::vector<unsigned long> v_asy_state_sessions;

					// 선태된 session 이외에서 상태가 st_asy 인 session 찾기.
					_cstatus_mgmt::_cstate::type_map_ptr_state::iterator it = m_map_ptr_state_cur.begin();
					for (; it != std::end(m_map_ptr_state_cur); ++it) {
						if (it == it_sel) {
							continue;
						}
						if (it->second->get() != _cstatus_mgmt::_cstate::st_asy) {
							continue;
						}
						v_asy_state_sessions.push_back(it->first);
					}//end for

					if (v_asy_state_sessions.empty() ){
						//선택된 session 만 ASY 상태
						//client 에 응답을 하지 않는 내부 cancel request 만들어, 현재 ASY 를 cancel 로 client 에게 응답하도록 함.

						unsigned long dw_device_index = result.ptr_req->get_device_index();
						cio_packet::type_ptr ptr_req_inner_cancel = cio_packet::build_cancel(n_session, dw_device_index);
						//
						_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
						clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(m_s_dev_path);
						if (wptr_dev.expired()) {
							continue;//error
						}

						cwait::type_ptr ptr_evt_self_cancel;
						std::tie(std::ignore, ptr_evt_self_cancel, std::ignore, std::ignore) = m_mgmt_q.sync_push_back(*result.ptr_req);
						wptr_dev.lock()->start_cancel(_cstatus_mgmt::_cb_dev_for_self_cancel, this, result.ptr_req->get_session_number());
						if (ptr_evt_self_cancel) {
							// ccancel 되어진 명령에 대한 callback 이 result cancel 로 호출된 후,
							// 생성된 cancel 명령에 callback 이 호출되고, 그 callback 에서 event 를 trigger 한다.
							ptr_evt_self_cancel->wait_for_at_once();
							//응답을 client 에 전송 할 필요 없음.
						}
					}
				}
				
				b_result = true;
			} while (false);
			return b_result;
		}

		cdev_ctl_fn::_cstatus_mgmt::_cresult cdev_ctl_fn::_cstatus_mgmt::_execute_write(clog* p_log, const cio_packet& request)
		{
			_debug_win_enter_execute_x(request, std::wstring(__WFUNCTION__));

			_cstatus_mgmt::_cresult result;
			result.ev_cur = _cstatus_mgmt::_cstate::ev_sync;// 중간에 에러나서 나가는 것들때문에 미리 event 값은 설정.

			cio_packet::type_ptr ptr_rsp = result.process_set_req_packet(request);
			cwait::type_ptr ptr_evt;

			unsigned long n_session = request.get_session_number();
			_cstatus_mgmt::_cstate::type_map_ptr_state::iterator it_sel = m_map_ptr_state_cur.find(n_session);
			_cstatus_mgmt::_cstate::type_map_ptr_state::iterator it = m_map_ptr_state_cur.end();

			do {
				if (it_sel == std::end(m_map_ptr_state_cur)) {
					continue;// not open
				}

				// client 부터 받은 parameter 얻기.
				type_v_buffer v_tx;
				request.get_data_field(v_tx);


				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					continue;
				}

				clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(m_s_dev_path);
				if (wptr_dev.expired()) {
					continue;
				}

				// 하위 라이브러리로 IO 실행.
				std::tie(std::ignore, ptr_evt, std::ignore, ptr_rsp) = m_mgmt_q.sync_push_back(request);
				wptr_dev.lock()->start_write(v_tx, _cstatus_mgmt::_cb_dev_for_sync_req, this, request.get_session_number());
				if (!ptr_evt) {
					continue;
				}
				ptr_evt->wait_for_at_once();
				if (!ptr_rsp->is_success()) {
					continue;
				}

				// 상태변경과 result 를 success 로 설정.
				_set_state_success_result_on_open_req(p_log, request, result, s_dev_path);


			} while (false);

			if (result.b_process_result) {
				//push_info(_ns_tools::ct_color::COLOR_NORMAL, L"_execute_open : success : session = %u", request.get_session_number());
			}

			_debug_win_leave_execute_x(request, std::wstring(__WFUNCTION__));
			return result;
		}
		//cdev_ctl_fn::_cstatus_mgmt::_cresult cdev_ctl_fn::_cstatus_mgmt::_execute_read(clog* p_log, const cio_packet& request);
		//cdev_ctl_fn::_cstatus_mgmt::_cresult cdev_ctl_fn::_cstatus_mgmt::_execute_transmit(clog* p_log, const cio_packet& request);
		//cdev_ctl_fn::_cstatus_mgmt::_cresult cdev_ctl_fn::_cstatus_mgmt::_execute_cancel(clog* p_log, const cio_packet& request);


		std::pair<bool, cdev_ctl_fn::_cstatus_mgmt::_cstate::type_state> cdev_ctl_fn::_cstatus_mgmt::_get_state(unsigned long n_session)
		{
			bool b_exist(false);
			_cstatus_mgmt::_cstate::type_state st_result(_cstatus_mgmt::_cstate::st_not);

			do {
				_cstatus_mgmt::_cstate::type_map_ptr_state::iterator it_sel = m_map_ptr_state_cur.find(n_session);
				if (it_sel == m_map_ptr_state_cur.end()) {
					continue;
				}

				b_exist = true;
				st_result = it_sel->second->get();

			} while (false);
			return std::make_pair(b_exist, st_result);
		}

		std::pair<bool, cdev_ctl_fn::_cstatus_mgmt::_cstate::type_state> cdev_ctl_fn::_cstatus_mgmt::_get_state_another(unsigned long n_session)
		{
			return _cstatus_mgmt::_cstate::get_combination_state_in_map_except_selected_session(m_map_ptr_state_cur, n_session);
		}


}//the end of _mp namespace

