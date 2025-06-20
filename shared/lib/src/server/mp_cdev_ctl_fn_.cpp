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
#include <mp_cstring.h>

#include <server/mp_cserver_.h>
#include <server/mp_cdev_ctl_fn_.h>

#include <hid/mp_clibhid.h>
#include <hid/_vhid_info_lpu237.h>

#if defined(_WIN32) && defined(_DEBUG)
//#undef __THIS_FILE_ONLY__
#define __THIS_FILE_ONLY__
#define __THIS_FILE_ONLY_STATE__

#include <atltrace.h>
#endif


namespace _mp {
		cdev_ctl_fn::~cdev_ctl_fn()
		{
		}

		void cdev_ctl_fn::_logging_if_state_is_changed()
		{
			std::vector<std::wstring> v_s_info;

			std::wstring s_out;

			if (m_st_combi != m_st_combi_old_for_debug) {
				_mp::cstring::format_stl_style(s_out, L"[I][STATE] CHANGE - all session state = ( %ls -> %ls ).\n",
					cbase_ctl_fn::_cstate::get_string_from_state(m_st_combi_old_for_debug).c_str(),
					cbase_ctl_fn::_cstate::get_string_from_state(m_st_combi).c_str()
				);
				if (!s_out.empty()) {
					v_s_info.push_back(s_out);
				}
			}
			if (m_b_cur_shared_mode != m_b_cur_shared_mode_old_for_debug) {
				std::wstring s_cur_shared_mode = L"OFF";
				std::wstring s_cur_shared_mode_old_for_debug = L"OFF";

				if (m_b_cur_shared_mode) {
					s_cur_shared_mode = L"ON";
				}
				if (m_b_cur_shared_mode_old_for_debug) {
					s_cur_shared_mode_old_for_debug = L"ON";
				}

				_mp::cstring::format_stl_style(s_out, L"[I][STATE] CHANGE - share mode = ( %ls -> %ls ).\n",
					s_cur_shared_mode_old_for_debug.c_str(),
					s_cur_shared_mode.c_str()
				);
				if (!s_out.empty()) {
					v_s_info.push_back(s_out);
				}
			}

			//
			// old map - new map 
			//cbase_ctl_fn::_cstate::type_map_ptr_state union_map;   // 합집합
			cbase_ctl_fn::_cstate::type_map_ptr_state intersection_map; // 교집합
			cbase_ctl_fn::_cstate::type_map_ptr_state diff_new_old; // map_new - map_old
			cbase_ctl_fn::_cstate::type_map_ptr_state diff_old_new; // map_old - map_new

			// 차집합, 합집합 및 교집합 계산
			for (const auto& item : m_map_ptr_state_cur) {
				//union_map[item.first] = item.second; // 먼저 map_new의 모든 요소 추가
				if (m_map_ptr_state_cur_old_for_debug.count(item.first)) {
					intersection_map[item.first] = item.second; // 공통 키인 경우 교집합에 추가
				}
				else {
					diff_new_old[item.first] = item.second; // map_old에 없는 경우 차집합 (map_new - map_old) 추가
				}
			}//end for

			for (const auto& item : m_map_ptr_state_cur_old_for_debug) {
				//union_map[item.first] = item.second; // 기존 합집합에 map_old의 요소 추가 (중복된 키는 덮어씀)
				if (!m_map_ptr_state_cur.count(item.first)) {
					diff_old_new[item.first] = item.second; // map_new에 없는 경우 차집합 (map_old - map_new) 추가
				}
			}//end for

				
			// 새로운 session
			for (const auto& item : diff_new_old) {
				if (!item.second) {
					continue;
				}
				_mp::cstring::format_stl_style(s_out, L"[I][STATE] NEW    - (session , state) = ( %u , %ls ).\n", item.first, item.second->get_by_wstring().c_str());
				if (!s_out.empty()) {
					v_s_info.push_back(s_out);
				}
			}//end for

			// 제거된 session
			for (const auto& item : diff_old_new) {
				if (!item.second) {
					continue;
				}
				_mp::cstring::format_stl_style(s_out, L"[I][STATE] REMOVE - (session , state) = ( %u , %ls ).\n", item.first, item.second->get_by_wstring().c_str());
				if (!s_out.empty()) {
					v_s_info.push_back(s_out);
				}
			}//end for

			// 유지되는 session 는 state 변화 검사
			for (const auto& item : intersection_map) {
				/*
				if (m_map_ptr_state_cur[item.first]->get() == m_map_ptr_state_cur_old_for_debug[item.first]->get()) {
					continue; // 상태변화 없으면 pass
				}
				*/
				//
				_mp::cstring::format_stl_style(s_out, L"[I][STATE] CHANGE - (session , state) = ( %u , %ls -> %ls ).\n",
					item.first,
					m_map_ptr_state_cur_old_for_debug[item.first]->get_by_wstring().c_str(),
					m_map_ptr_state_cur[item.first]->get_by_wstring().c_str()
				);
				if (!s_out.empty()) {
					v_s_info.push_back(s_out);
				}
			}//end for

			//////////////////////////////
			if (!v_s_info.empty()) {
				// 상태 표시.
				for (auto item : v_s_info) {
					m_p_ctl_fun_log->log_fmt(L"%ls", item.c_str());
					m_p_ctl_fun_log->trace(L"%ls", item.c_str());

#if defined(_WIN32) && defined(_DEBUG) && (defined(__THIS_FILE_ONLY__) || defined(__THIS_FILE_ONLY_STATE__))
					ATLTRACE(L"~~~~~ %ls", item.c_str());
#endif
				}

				///////////////////////////////
				// 새로운 상태 저장.
				m_st_combi_old_for_debug = m_st_combi;
				m_b_cur_shared_mode_old_for_debug = m_b_cur_shared_mode;
				// 
				// m_map_ptr_state_cur_old_for_debug = m_map_ptr_state_cur; 이와 같아 하면
				// m_map_ptr_state_cur_old_for_debug 과 m_map_ptr_state_cur 의 shared_ptr 이 같은 pointer 가리키게 되서,
				// m_map_ptr_state_cur 의 상태가 변경되면, m_map_ptr_state_cur_old_for_debug 도 변경되므로 쓸없게 되므로,
				// m_map_ptr_state_cur 를 m_map_ptr_state_cur_old_for_debug 에 deep copy 해야한다. overhead.......
				m_map_ptr_state_cur_old_for_debug.clear();
				for (const auto& item : m_map_ptr_state_cur) {
					if (item.second) {
						m_map_ptr_state_cur_old_for_debug[item.first] = std::make_shared<cbase_ctl_fn::_cstate>(*item.second);
					}
				}// end for

			}
		}

		cdev_ctl_fn::cdev_ctl_fn(clog* p_log) :
			cbase_ctl_fn(p_log),
			m_b_cur_shared_mode(false), m_b_cur_shared_mode_old_for_debug(false)
		{
			m_st_combi = cbase_ctl_fn::_cstate::st_not;
			m_st_combi_old_for_debug = m_st_combi;
		}

		cdev_ctl_fn::type_tuple_full cdev_ctl_fn::_cq_mgmt::qm_sync_push_back(const cio_packet::type_ptr& ptr_request)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			auto r = m_q_sync_req_evt_rsp.push_back(ptr_request);

			//
			// m_map_ptr_q_ptr_cur_req_read 에 기존 read 명령은 device start_X 명령 실행으로
			// 하위 device io 에서 자동 취소 되서, 기존 read 명령 실행시 설정한 callback 이 cancel result 를 가지고 호출된다.
			// 따라서, m_map_ptr_q_ptr_cur_req_read 에 기존 명령이 있어도 자동으로 처리된다.

			return r;
		}

		cdev_ctl_fn::type_tuple_full cdev_ctl_fn::_cq_mgmt::qm_sync_pop_front(bool b_remove)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			auto r = m_q_sync_req_evt_rsp.pop_front(b_remove);
			return r;
		}

		void cdev_ctl_fn::_cq_mgmt::qm_sync_remove_front()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_q_sync_req_evt_rsp.remove_front();
		}

		bool cdev_ctl_fn::_cq_mgmt::qm_read_is_empty()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			bool b_empty(true);
			b_empty = m_map_ptr_q_ptr_cur_req_read.empty();
			return b_empty;
		}

		bool cdev_ctl_fn::_cq_mgmt::qm_read_push_back(const cio_packet::type_ptr& ptr_request)
		{
			bool b_first(false);

			// key 가 있으면, 항상 value 값인 ptr_q 는 항상 할당되어야 한다.

			unsigned long n_session = ptr_request->get_session_number();
			cdev_ctl_fn::_cq_mgmt::_cq::type_ptr ptr_q;

			std::lock_guard<std::mutex> lock(m_mutex);

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
				std::tie(m_ptr_cur_req_read, std::ignore, std::ignore, std::ignore) = ptr_q->push_back(ptr_request);
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
				ATLTRACE(L"~~~~~ [%u] First push cur req(%ls) read Q.\n", n_session, ptr_request->get_action_by_string().c_str());
#endif
			}
			else {
				ptr_q->push_back(ptr_request);
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
				ATLTRACE(L"~~~~~ [%u] Push cur req(%ls) read Q.\n", n_session, ptr_request->get_action_by_string().c_str());
#endif
			}
			return b_first;
		}

		std::vector<cdev_ctl_fn::type_tuple_full> cdev_ctl_fn::_cq_mgmt::qm_read_pop_front(unsigned long n_session_number,bool b_remove)
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
						if (it == std::end(m_map_ptr_q_ptr_cur_req_read)) {
							break; //exit for
						}
					}
				}//end for

			} while (false);
			return v_result;
		}

		std::vector<cdev_ctl_fn::type_tuple_full> cdev_ctl_fn::_cq_mgmt::qm_read_set_response_front(
			cqitem_dev& qi,
			std::vector<size_t>& v_req_uid_need_more_reading
		)
		{
			cwait::type_ptr ptr_evt;
			cio_packet::type_ptr ptr_req, ptr_rsp;
			int n_evt(-1);
			std::vector<cdev_ctl_fn::type_tuple_full> v_result;

			v_req_uid_need_more_reading.clear(); // 계속 읽기가 필요한 req 의 uid vector 초기화..

			do {
				std::lock_guard<std::mutex> lock(m_mutex);

				_cq_mgmt::_type_map_ptr_q::iterator it;

				if (m_map_ptr_q_ptr_cur_req_read.empty()) {
					continue;
				}

				it = std::begin(m_map_ptr_q_ptr_cur_req_read);

				if (qi.get_request_type() != cqitem_dev::req_cancel) {
					// cancel 에 대한 결과가 아니면

					for (; it != std::end(m_map_ptr_q_ptr_cur_req_read); ++it) {
						auto r = it->second->pop_front(false);
						std::tie(ptr_req, ptr_evt, n_evt, ptr_rsp) = r;
						if (ptr_req) {
							if (ptr_req->is_recover_reserved()) {
								ptr_req->set_recover_reserve(false);// recover flag 삭제.
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
								ATLTRACE(L"~~~~~ [%u:%u] RESET recover flag.\n", ptr_req->get_session_number(), ptr_req->get_uid());
#endif
								v_req_uid_need_more_reading.push_back(ptr_req->get_uid());
								continue;// recover flag 있는 것은 response 를 설정하지 않음.
							}
						}

						if (!ptr_evt) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
							ATLTRACE(L"~~~~~ [%u:%u] ERROR ptr_evt is nullptr %u.\n", ptr_req->get_session_number(), ptr_req->get_uid());
#endif
							continue;
						}
						if (!cbase_ctl_fn::_set_response(*ptr_rsp, qi, *ptr_req)) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
							ATLTRACE(L"~~~~~ [%u:%u] ERROR fail set rsp %u.\n", ptr_req->get_session_number(), ptr_req->get_uid());
#endif
							continue;
						}
						v_result.push_back(r);

#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L"~~~~~ [%u:%u] Set response of  req read Q.\n", ptr_req->get_session_number(), ptr_req->get_uid());
#endif
						ptr_evt->set(n_evt);
					}//end for
					continue;
				}

				// qi.get_request_type() == cqitem_dev::req_cancel
				// 같은 session 에 있는 것만 결과 setting.
				for (; it != std::end(m_map_ptr_q_ptr_cur_req_read); ++it) {
					auto r = it->second->pop_front(false);
					std::tie(ptr_req, ptr_evt, n_evt, ptr_rsp) = r;
					if (!ptr_evt) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L"~~~~~ [%u:%u] ERROR ptr_evt is nullptr %09u.\n", ptr_req->get_session_number(), ptr_req->get_uid());
#endif
						continue;
					}
					if (ptr_req->get_session_number() != qi.get_session_number()) {
						v_req_uid_need_more_reading.push_back(ptr_req->get_uid());
						continue;
					}
					if (!cbase_ctl_fn::_set_response(*ptr_rsp, qi, *ptr_req)) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L"~~~~~ [%u:%u] Set response of  req read Q.\n", ptr_req->get_session_number(), ptr_req->get_uid());
#endif
						continue;
					}
					v_result.push_back(r);

					ptr_evt->set(std::get<2>(r));
				}//end for

			} while (false);
			return v_result;
		}

		size_t cdev_ctl_fn::_cq_mgmt::qm_read_set_request_front_to_recover_of_another_session(unsigned long n_session_number)
		{
			size_t n_cnt(0);

			std::lock_guard<std::mutex> lock(m_mutex);

			cio_packet::type_ptr ptr_req;

			for (auto item : m_map_ptr_q_ptr_cur_req_read) {
				if (item.first == n_session_number) {
					// n_session_number 를 제외하고 복구 설정.
					continue;
				}
				_cq_mgmt::_cq::type_ptr ptr_q = item.second;
				std::tie(ptr_req,std::ignore, std::ignore, std::ignore) = ptr_q->pop_front(false);
				if (!ptr_req) {
					continue;
				}
				ptr_req->set_recover_reserve(true);
				++n_cnt;
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
				ATLTRACE(L"~~~~~ [%u:%u] SET recover flag.\n", ptr_req->get_session_number(), ptr_req->get_uid() );
#endif
			}//end for
			return n_cnt;
		}

		std::vector<cdev_ctl_fn::type_tuple_full> cdev_ctl_fn::_cq_mgmt::qm_read_pop_front_remove_complete_of_all()
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

					if (it == std::end(m_map_ptr_q_ptr_cur_req_read)) {
						break;//exit for
					}
				}//end for

			} while (false);
			return v_result;
		}

		std::shared_ptr< std::vector< std::pair<cio_packet::type_ptr, cio_packet::type_ptr>> >
			cdev_ctl_fn::_cq_mgmt::qm_read_pop_front_remove_complete_of_all_by_rsp_ptr()
		{
			cwait::type_ptr ptr_evt;
			int n_evet(-1);
			cio_packet::type_ptr ptr_req, ptr_rsp;
			std::shared_ptr< std::vector< std::pair<cio_packet::type_ptr, cio_packet::type_ptr>> > ptr_v_req_rsp;

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

					if (!ptr_v_req_rsp) {// return vector 생성.
						ptr_v_req_rsp = std::make_shared<std::vector<std::pair<cio_packet::type_ptr, cio_packet::type_ptr>>>();
					}
					
					ptr_q->remove_front();// queue 에서 삭제.
					if (ptr_q->empty()) {
						// 빈 queue 는 map에서 삭제.
						it = m_map_ptr_q_ptr_cur_req_read.erase(it);
					}

					ptr_v_req_rsp->emplace_back(std::get<0>(r),std::get<3>(r));

					if (it == std::end(m_map_ptr_q_ptr_cur_req_read)) {
						break;//exit for
					}
				}//end for

			} while (false);
			return ptr_v_req_rsp;
		}


		void cdev_ctl_fn::_cq_mgmt::qm_read_remove_front(unsigned long n_session_number)
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

		void cdev_ctl_fn::_cq_mgmt::qm_read_remove_front_all()
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
						if (it == std::end(m_map_ptr_q_ptr_cur_req_read)) {
							break; // exit for
						}
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

		cdev_ctl_fn::type_tuple_full cdev_ctl_fn::_cq_mgmt::_cq::push_back(const cio_packet::type_ptr& ptr_request)
		{
			cwait::type_ptr ptr_evt;
			int n_evt(-1);
			
			cio_packet::type_ptr ptr_req = ptr_request;
			cio_packet::type_ptr ptr_rsp = std::make_shared<cio_packet>(*ptr_req);

			ptr_rsp->set_cmd(cio_packet::cmd_response);

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
		//_cstatus_mgmt member

		/**
		* callback 에서 서버에 바로 전송하지 말자.
		*/
		std::pair<bool, std::vector<size_t>> cdev_ctl_fn::_cb_dev_read_on_exclusive(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			cio_packet::type_ptr ptr_rsp;
			
			do {
				bool b_pass_this_response(false);
				do{
					// 암호화 응답을 구별하기 위해, response 를 set 하기 전에 응답 검사.
					// himalia 에서 추가된 암호화 기능 지원과 기존 protocol 과 호환성 유지를 위해 필요한 코드.
					std::tuple<cqitem_dev::type_result, type_v_buffer, std::wstring> rqi = qi.get_result_all();
					if (std::get<0>(rqi) != cqitem_dev::result_success) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L">>>>> [%ls] result !=cqitem_dev::result_success.\n", __WFUNCTION__);
#endif
						continue;
					}
					if (std::get<1>(rqi).size() < 3) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L">>>>> [%ls] rx-size < 3.\n", __WFUNCTION__);
#endif
						continue;
					}
					std::array<char, 3> ar_c_len{ std::get<1>(rqi)[0], std::get<1>(rqi)[1], std::get<1>(rqi)[2] };
					if (ar_c_len[0] >= 0) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L">>>>> [%ls] ar_c_len[0] >= 0.\n", __WFUNCTION__);
#endif
						continue;
					}
					if (ar_c_len[1] >= 0) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L">>>>> [%ls] ar_c_len[1] >= 0.\n", __WFUNCTION__);
#endif
						continue;
					}
					if (ar_c_len[2] >= 0) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L">>>>> [%ls] ar_c_len[2] >= 0.\n", __WFUNCTION__);
#endif
						continue;
					}

					if(_vhid_info_lpu237::is_rx_msr_extension(std::get<1>(rqi))) {
						// 응답값의 각 track len 의 값이 음수로 에러를 표시하는 듯하지만,
						// himalia 에서 추가된 암호화 응답 구조를 갖는 것으로 확인되어,
						// 무시하지 말고 계속 처리.
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L">>>>> [%ls] rx.is_rx_msr_extension = true .\n", __WFUNCTION__);
#endif
						continue; 
					}

					// 정상 응답 수신했으나, 응답 형식이 잘못 되어서
					b_pass_this_response = true; // 이번 응답 무시.

				} while (false);

				if (b_pass_this_response) {
					b_complete = false;
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
					ATLTRACE(L">>>>> [%ls] ignore. more processing .\n", __WFUNCTION__);
#endif
					continue;//more processing
				}
				
				// 현재 수신을 응답으로 설정. 
				std::vector<size_t> v_dummy_of_req_uid_need_more_reading;
				std::vector<cdev_ctl_fn::type_tuple_full> v_tuple = p_obj->m_mgmt_q.qm_read_set_response_front(qi, v_dummy_of_req_uid_need_more_reading);
				if (v_tuple.empty()) {
					b_complete = false;
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
					ATLTRACE(L">>>>> [%ls] more processing .\n", __WFUNCTION__);
#endif
					continue;//more processing
				}

				p_obj->_transit_state_for_only_all_async_by_rx_event(v_tuple); // 상태를 변화

				//////////////////////
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
				ATLTRACE(L">>>>> [%ls] normal complete with state change(exclusive).\n", __WFUNCTION__);
#endif

			} while (false);

			return std::make_pair(b_complete, std::vector<size_t>());
		}

		/**
		* callback 에서 서버에 바로 전송하지 말자.
		*/
		std::pair<bool, std::vector<size_t>> cdev_ctl_fn::_cb_dev_read_on_shared(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			cio_packet::type_ptr ptr_rsp;
			size_t n_qitem_uid(qi.get_uid());
			std::vector<size_t> v_req_uid_need_more_reading;

			do {
				bool b_pass_this_response(false);
				do {
					// 암호화 응답을 구별하기 위해, response 를 set 하기 전에 응답 검사.
					// himalia 에서 추가된 암호화 기능 지원과 기존 protocol 과 호환성 유지를 위해 필요한 코드.
					std::tuple<cqitem_dev::type_result, type_v_buffer, std::wstring> rqi = qi.get_result_all();
					if (std::get<0>(rqi) != cqitem_dev::result_success) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L">>>>> [%ls] result !=cqitem_dev::result_success.\n", __WFUNCTION__);
#endif
						continue;
					}
					if (std::get<1>(rqi).size() < 3) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L">>>>> [%ls] rx-size < 3.\n", __WFUNCTION__);
#endif
						continue;
					}
					std::array<char, 3> ar_c_len{ std::get<1>(rqi)[0], std::get<1>(rqi)[1], std::get<1>(rqi)[2] };
					if (ar_c_len[0] >= 0) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L">>>>> [%ls] ar_c_len[0] >= 0.\n", __WFUNCTION__);
#endif
						continue;
					}
					if (ar_c_len[1] >= 0) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L">>>>> [%ls] ar_c_len[1] >= 0.\n", __WFUNCTION__);
#endif
						continue;
					}
					if (ar_c_len[2] >= 0) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L">>>>> [%ls] ar_c_len[2] >= 0.\n", __WFUNCTION__);
#endif
						continue;
					}

					if (_vhid_info_lpu237::is_rx_msr_extension(std::get<1>(rqi))) {
						// 응답값의 각 track len 의 값이 음수로 에러를 표시하는 듯하지만,
						// himalia 에서 추가된 암호화 응답 구조를 갖는 것으로 확인되어,
						// 무시하지 말고 계속 처리.
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L">>>>> [%ls] rx.is_rx_msr_extension = true .\n", __WFUNCTION__);
#endif
						continue;
					}

					// 정상 응답 수신했으나, 응답 형식이 잘못 되어서
					b_pass_this_response = true; // 이번 응답 무시.

				} while (false);

				if (b_pass_this_response) {
					b_complete = false;
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
					ATLTRACE(L">>>>> [%ls] ignore. more processing .\n", __WFUNCTION__);
#endif
					continue;//more processing
				}

				//응답 공유 상태에서는 모든 read 에 대해 응답을 동일하게 설정해주어야 함.
				
				std::vector<cdev_ctl_fn::type_tuple_full> v_tuple = p_obj->m_mgmt_q.qm_read_set_response_front(qi, v_req_uid_need_more_reading);
				if (v_tuple.empty()) {
					b_complete = false;
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
					ATLTRACE(L">>>>> [%ls] more processing .\n", __WFUNCTION__);
#endif
					continue;//more processing
				}

				p_obj->_transit_state_for_only_all_async_by_rx_event(v_tuple); // 상태를 변화

				//////////////////////
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
				ATLTRACE(L">>>>> [%ls] normal complete with state change(shared).\n", __WFUNCTION__);
				for (auto item : v_req_uid_need_more_reading) {
					ATLTRACE(L">>>>> more reading uid - %u.\n", item);
				}
#endif
			} while (false);

			return std::make_pair(b_complete, v_req_uid_need_more_reading);
		}

		/**
		* callback of clibhid_dev.start_write, cancel, write_read.
		* @param first cqitem_dev reference
		* @param second user data
		* @return true -> complete, false -> read more
		*/
		std::pair<bool, std::vector<size_t>> cdev_ctl_fn::_cb_dev_for_sync_req(cqitem_dev& qi, void* p_user)
		{
			//callback 에서 서버에 바로 전송하지 말자.
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			cio_packet::type_ptr ptr_req, ptr_rsp;
			cwait::type_ptr ptr_evt;
			int n_evt_index(-1);

			do {
				// pop 에서 request 얻는과 결과 setting 이 분리되어도, 동기식으로 event set 될때 까지 기다리니까 안전.
				std::tie(ptr_req, ptr_evt, n_evt_index, ptr_rsp) = p_obj->m_mgmt_q.qm_sync_pop_front(false);
				if (!ptr_req) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
					ATLTRACE(L">>>>> [%ls] qm_sync_pop_front.false.\n", __WFUNCTION__);
#endif
					continue;// 요청한 session 의 응답이 아니면, 무시.
				}

				if (!cbase_ctl_fn::_set_response(*ptr_rsp, qi, *ptr_req)) {
					//현재 요청은 아직 결과를 몰라서 큐에서 삭제하면 안됌.
					b_complete = false;
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
					ATLTRACE(L">>>>> [%ls] _set_response.false.\n", __WFUNCTION__);
#endif
					continue;//more processing
				}
				//
				if (ptr_evt) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
					ATLTRACE(L">>>>> [%ls] set.before\n", __WFUNCTION__);
#endif
					ptr_evt->set(n_evt_index); //동기식 응답 기다리는 event 기다림 해제.

#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
					ATLTRACE(L">>>>> [%ls] set.after\n", __WFUNCTION__);
#endif
				}

			} while (false);
			return std::make_pair(b_complete, std::vector<size_t>());
		}

		void cdev_ctl_fn::_reset_(bool b_shared_mode)
		{
			m_b_cur_shared_mode = b_shared_mode;
			m_st_combi = cbase_ctl_fn::_cstate::st_not;
			m_map_ptr_state_cur.clear();
		}

		std::tuple<bool,cbase_ctl_fn::_cstate::type_state, cbase_ctl_fn::_cstate::type_state> cdev_ctl_fn::_get_state_cur_(unsigned long n_session) const
		{
			bool b_exist(false);
			cbase_ctl_fn::_cstate st_cur;
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

		cdev_ctl_fn::type_ptr_v_pair_ptr_req_ptr_rsp cdev_ctl_fn::get_all_complete_response
		(
			const cdev_ctl_fn::type_pair_ptr_req_ptr_rsp& pair_ptr_req_ptr_rsp_additional 
			/*= std::make_pair(cio_packet::type_ptr(),cio_packet::type_ptr())*/
		)
		{
			cdev_ctl_fn::type_ptr_v_pair_ptr_req_ptr_rsp ptr_v_req_rsp;

			do {
				// 모든 session 의 reading( asyn ) 에서 complete 된 req를 먼저 얻고, reading Q 에서 제거.
				ptr_v_req_rsp = m_mgmt_q.qm_read_pop_front_remove_complete_of_all_by_rsp_ptr();

				// sync 에서 얻음.
				cio_packet::type_ptr ptr_rsp_sync, ptr_req_sync;
				std::tie(ptr_req_sync, std::ignore, std::ignore, ptr_rsp_sync) = m_mgmt_q.qm_sync_pop_front(true);// pop nad remove

				if (ptr_rsp_sync) {
					if (!ptr_v_req_rsp) {
						ptr_v_req_rsp = std::make_shared<std::vector<std::pair<cio_packet::type_ptr, cio_packet::type_ptr>>>();
					}

					// 비동기 처리 결과를 얻음.
					// 비동기 결과는 특성 상 존재하면, 1개 밖에 없음. 
					ptr_v_req_rsp->emplace_back(ptr_req_sync,ptr_rsp_sync);
				}

				if (!pair_ptr_req_ptr_rsp_additional.first) {
					continue;
				}
				if (!pair_ptr_req_ptr_rsp_additional.second) {
					continue;
				}

				// 이미 vector 에 있는지 조사.
				bool b_already_exsit(false);
				if (ptr_v_req_rsp) {
					for (auto item : *ptr_v_req_rsp) {
						if (!item.first) {
							continue;
						}
						if (!item.second) {
							continue;
						}
						if (item.first.get() == pair_ptr_req_ptr_rsp_additional.first.get()) {
							// 동일한 req 찾기 성공.
							b_already_exsit = true;
							break;// exit for
						}
					}//end for
				}

				if (b_already_exsit) {
					continue; // 이미 존재하면, 추가하지 않는다.
				}
				if (!ptr_v_req_rsp) {
					ptr_v_req_rsp = std::make_shared<std::vector<std::pair<cio_packet::type_ptr, cio_packet::type_ptr>>>();
				}
				ptr_v_req_rsp->emplace_back(pair_ptr_req_ptr_rsp_additional);

			} while (false);

			

			return ptr_v_req_rsp;
		}

		std::vector<cio_packet::type_ptr> cdev_ctl_fn::get_front_of_read_queue()
		{
			std::vector<cio_packet::type_ptr> v_ptr_req;

			auto v = m_mgmt_q.qm_read_pop_front(_MP_TOOLS_INVALID_SESSION_NUMBER,false);

			for (auto item : v) {
				if (std::get<0>(item)) {
					v_ptr_req.push_back(std::get<0>(item));
				}
			}//end for
			return v_ptr_req;
		}

		std::wstring cdev_ctl_fn::get_dev_path() const
		{
			return m_s_dev_path;
		}

		cbase_ctl_fn::cresult::type_ptr cdev_ctl_fn::process_event
		(
			const cio_packet::type_ptr& ptr_req_new,
			const cio_packet::type_ptr& ptr_req_cur
		)
		{
			// 논리적으로 각 session 의 장비는 독립된 것으로 보기 때문에
			// 이벤트가 발생한 session 이 외의 session 의 state 에 영향은 없어야 함. 
			// result.b_process_complete 이 true 면, result.ptr_rsp 에 결과가 설정되어야 함.
			// cresult.ptr_rsp is must be created in this function.

			cbase_ctl_fn::cresult::type_ptr ptr_result; // default contructure, not yet ptr response.
			auto evt = cbase_ctl_fn::_cstate::get_event_from_request_action(ptr_req_new->get_action());

			do {
				if (evt == cbase_ctl_fn::_cstate::ev_none) {
					continue;
				}

				if (m_b_cur_shared_mode) {
					ptr_result = _process_event_on_shared_mode(ptr_req_new, ptr_req_cur);
				}
				else {
					ptr_result = _process_event_on_exclusive_mode(ptr_req_new, ptr_req_cur);
				}

			} while (false);
			return ptr_result;
		}

		cbase_ctl_fn::cresult::type_ptr cdev_ctl_fn::_process_event_on_shared_mode
		(
			const cio_packet::type_ptr& ptr_req_new,
			const cio_packet::type_ptr& ptr_req_cur
		)
		{
			// 논린적으로 각 session 의 장비는 독립된 것으로 보기 때문에
			// 이벤트가 발생한 session 이 외의 session 의 state 에 영향은 없어야 함.
			cbase_ctl_fn::_cstate::type_state st_cur(cbase_ctl_fn::_cstate::st_not), st_another(cbase_ctl_fn::_cstate::st_not);
			cbase_ctl_fn::_cstate::type_state st_combination(cbase_ctl_fn::_cstate::st_not);

			bool b_exist_sel(false);
			std::tie(b_exist_sel, st_cur, st_combination) = _get_state_cur_(ptr_req_new->get_session_number());

			// 선택된 session 을 제외한 나머지 session 의 상태의 combination state를 얻는다.
			bool b_exist_another(false);
			std::tie(b_exist_another, st_another) = _get_state_another_(ptr_req_new->get_session_number());

			// 선태된 session state 와 나머지 session 의 상태를 연결 상태를 얻는다.
			_cstate::type_state_sel_another st_sel_another = (_cstate::type_state_sel_another)((int)(st_cur * 100 + st_another));

#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
			if (b_exist_sel) {
				ATLTRACE(L"~~~~~ %u, %ls (sel state, sel state, combi state ) = (exist, %ls, %ls).\n",
					ptr_req_new->get_session_number(),
					ptr_req_new->get_action_by_string().c_str(),
					cbase_ctl_fn::_cstate::get_string_from_state(st_cur).c_str(),
					cbase_ctl_fn::_cstate::get_string_from_state(st_combination).c_str()
				);
			}
			else {
				ATLTRACE(L"~~~~~ %u, %ls (sel state, sel state, combi state ) = (none, %ls, %ls).\n",
					ptr_req_new->get_session_number(),
					ptr_req_new->get_action_by_string().c_str(),
					cbase_ctl_fn::_cstate::get_string_from_state(st_cur).c_str(),
					cbase_ctl_fn::_cstate::get_string_from_state(st_combination).c_str()
				);
			}
			if (b_exist_another) {
				ATLTRACE(L"~~~~~ (another state, another state, sel_another state ) = (exist, %ls, %ls).\n",
					cbase_ctl_fn::_cstate::get_string_from_state(st_another).c_str(),
					cbase_ctl_fn::_cstate::get_string_from_sel_another_state(st_sel_another).c_str()
				);
			}
			else {
				ATLTRACE(L"~~~~~ (another state, another state, sel_another state ) = (none, %ls, %ls).\n",
					cbase_ctl_fn::_cstate::get_string_from_state(st_another).c_str(),
					cbase_ctl_fn::_cstate::get_string_from_sel_another_state(st_sel_another).c_str()
				);
			}
#endif

			cbase_ctl_fn::cresult::type_ptr ptr_result_new = std::make_shared<cbase_ctl_fn::cresult>(ptr_req_new, m_s_dev_path); //contructure, only increase reference request, isn't create response.

			do {// 새로운 req 실행 전, 현재 실행 중인, req 가 있는 경우가 주의 대상.
				switch (st_sel_another)
				{
				case cbase_ctl_fn::_cstate::st_snot_anot://result 설정을 하위 함수에서 설정.
					_process_shared_selected_session_st_not_another_session_st_not(*ptr_result_new);
					break;
				case cbase_ctl_fn::_cstate::st_snot_aidl://result 설정을 하위 함수에서 설정.
					_process_shared_selected_session_st_not_another_session_st_idl(*ptr_result_new);
					break;
				case cbase_ctl_fn::_cstate::st_snot_aasy://result 설정을 하위 함수에서 설정.
					_process_shared_selected_session_st_not_another_session_st_asy(*ptr_result_new, ptr_req_cur); //<< 현재하는 것이 있음.
					break;

				case cbase_ctl_fn::_cstate::st_sidl_anot://result 설정을 하위 함수에서 설정.
					_process_shared_selected_session_st_idl_another_session_st_not(*ptr_result_new);
					break;
				case cbase_ctl_fn::_cstate::st_sidl_aidl://result 설정을 하위 함수에서 설정.
					_process_shared_selected_session_st_idl_another_session_st_idl(*ptr_result_new);
					break;
				case cbase_ctl_fn::_cstate::st_sidl_aasy://result 설정을 하위 함수에서 설정.
					_process_shared_selected_session_st_idl_another_session_st_asy(*ptr_result_new, ptr_req_cur); //<< 현재하는 것이 있음.
					break;

				case cbase_ctl_fn::_cstate::st_sasy_anot://result 설정을 하위 함수에서 설정.
					_process_shared_selected_session_st_asy_another_session_st_not(*ptr_result_new, ptr_req_cur); //<< 현재하는 것이 있음.
					break;
				case cbase_ctl_fn::_cstate::st_sasy_aidl://result 설정을 하위 함수에서 설정.
					_process_shared_selected_session_st_asy_another_session_st_idl(*ptr_result_new, ptr_req_cur); //<< 현재하는 것이 있음.
					break;
				case cbase_ctl_fn::_cstate::st_sasy_aasy://result 설정을 하위 함수에서 설정.
					_process_shared_selected_session_st_asy_another_session_st_asy(*ptr_result_new, ptr_req_cur); //<< 현재하는 것이 있음.
					break;
				default:
					// 에러 complete.
					ptr_result_new->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_session);
					continue;
				}

			} while (false);
			return ptr_result_new;
		}

		///////////////////////////////////////////////
		// shared mode sub processing
		///////////////////////////////////////////////
		void cdev_ctl_fn::_process_shared_selected_session_st_not_another_session_st_not(cbase_ctl_fn::cresult& result)
		{	
			//excluisive mode 에서 open 이 안된 상태와 동일하게 처리.
			_process_exclusive_st_not(result);
		}

		void cdev_ctl_fn::_process_shared_selected_session_st_not_another_session_st_idl(cbase_ctl_fn::cresult& result)
		{
			cbase_ctl_fn::_cstate::type_evt evt = result.get_cur_event();

			do {
				std::wstring s_dev_path;
				bool b_user_shared_mode(false);

				switch (evt)
				{
				case cbase_ctl_fn::_cstate::ev_open:
					_proc_event_open(result,false);
					break;
				case cbase_ctl_fn::_cstate::ev_close:
				case cbase_ctl_fn::_cstate::ev_sync:
				case cbase_ctl_fn::_cstate::ev_asy:
					result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_not_open);

					m_p_ctl_fun_log->log_fmt(L"[E] - %ls | error_reason_device_not_open : session = %u.\n", __WFUNCTION__, result.get_session_number());
					m_p_ctl_fun_log->trace(L"[E] - %ls | error_reason_device_not_open : session = %u.\n", __WFUNCTION__, result.get_session_number());
					break;
				case cbase_ctl_fn::_cstate::ev_rx:
					// 무시되는 event
					break;
				default:
					continue;
				}//end switch
			} while (false);

		}

		void cdev_ctl_fn::_process_shared_selected_session_st_not_another_session_st_asy
		(
			cbase_ctl_fn::cresult& result_new,
			const cio_packet::type_ptr& ptr_req_cur
		)
		{
			cbase_ctl_fn::_cstate::type_evt evt = result_new.get_cur_event();

			do {
				std::wstring s_dev_path;
				bool b_user_shared_mode(false);

				switch (evt)
				{
				case cbase_ctl_fn::_cstate::ev_open:
					_proc_event_open(result_new, false);
					break;
				case cbase_ctl_fn::_cstate::ev_close:
				case cbase_ctl_fn::_cstate::ev_sync:
				case cbase_ctl_fn::_cstate::ev_asy:
					result_new.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_not_open);

					m_p_ctl_fun_log->log_fmt(L"[E] - %ls | error_reason_device_not_open : session = %u.\n", __WFUNCTION__, result_new.get_session_number());
					m_p_ctl_fun_log->trace(L"[E] - %ls | error_reason_device_not_open : session = %u.\n", __WFUNCTION__, result_new.get_session_number());
					break;
				case cbase_ctl_fn::_cstate::ev_rx:
					// 무시되는 event
					break;
				default:
					continue;
				}//end switch
			} while (false);
		}

		void cdev_ctl_fn::_process_shared_selected_session_st_idl_another_session_st_not(cbase_ctl_fn::cresult& result)
		{
			_process_exclusive_st_idl(result);
		}

		void cdev_ctl_fn::_process_shared_selected_session_st_idl_another_session_st_idl(cbase_ctl_fn::cresult& result)
		{
			cbase_ctl_fn::_cstate::type_evt evt = result.get_cur_event();

			cio_packet::type_ptr ptr_rsp;

			do {
				std::wstring s_dev_path;
				bool b_user_shared_mode(false);

				switch (evt)
				{
				case cbase_ctl_fn::_cstate::ev_open:
					result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_open);
					break;
				case cbase_ctl_fn::_cstate::ev_close:
					// 현재 상태가 IDL 이므로 해당 session 이 있으면, 해당 session 의 키와 값만 map 에서 삭제 하면됨.
					result.after_processing_set_rsp_with_succss_complete(std::wstring(), m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_sync:
					ptr_rsp = _start_and_complete_by_sync_req(m_s_dev_path, result.get_req());
					if (!ptr_rsp) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_processing_set_rsp_with_succss_complete(ptr_rsp, m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_asy:
					if (!_start_by_async_req(m_s_dev_path, result.get_req())) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_starting_process_set_rsp_with_succss_ing(m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);//<<
					break;
				case cbase_ctl_fn::_cstate::ev_rx:
					// 무시되는 event
					break;
				default:
					continue;
				}//end switch
			} while (false);
		}

		void cdev_ctl_fn::_process_shared_selected_session_st_idl_another_session_st_asy
		(
			cbase_ctl_fn::cresult& result,
			const cio_packet::type_ptr& ptr_req_cur
		)
		{
			cbase_ctl_fn::_cstate::type_evt evt = result.get_cur_event();

			cio_packet::type_ptr ptr_rsp;

			do {
				std::wstring s_dev_path;
				bool b_user_shared_mode(false);

				switch (evt)
				{
				case cbase_ctl_fn::_cstate::ev_open:
					result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_open);
					break;
				case cbase_ctl_fn::_cstate::ev_close:
					// 현재 상태가 IDL 이므로 해당 session 이 있으면, 해당 session 의 키와 값만 map 에서 삭제 하면됨.
					result.after_processing_set_rsp_with_succss_complete(std::wstring(), m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_sync:
					// 새로운 request 를 내리면, another 는 하위단에서 자동 cancel 이 발생하므로 시키므로
					// 내리기 전에 , 실행 중인 another request에 복구 설정 해야함.
					m_mgmt_q.qm_read_set_request_front_to_recover_of_another_session(result.get_session_number());
					ptr_rsp = _start_and_complete_by_sync_req(m_s_dev_path, result.get_req());
					if (!ptr_rsp) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_processing_set_rsp_with_succss_complete(ptr_rsp, m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_asy:
					// 새로운 request 를 내리면, another 는 하위단에서 자동 cancel 이 발생하므로 시키므로
					// 내리기 전에 , 실행 중인 another request에 복구 설정 해야함.
					//>>m_mgmt_q.qm_read_set_request_front_to_recover_of_another_session(result.get_session_number());

					if (!_start_by_async_req(m_s_dev_path, result.get_req())) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_starting_process_set_rsp_with_succss_ing(m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);//<<
					break;
				case cbase_ctl_fn::_cstate::ev_rx:
					// 무시되는 event
					break;
				default:
					continue;
				}//end switch
			} while (false);
		}

		void cdev_ctl_fn::_process_shared_selected_session_st_asy_another_session_st_not
		(
			cbase_ctl_fn::cresult& result,
			const cio_packet::type_ptr& ptr_req_cur
		)
		{
			_process_exclusive_st_asy(result);
		}

		void cdev_ctl_fn::_process_shared_selected_session_st_asy_another_session_st_idl
		(
			cbase_ctl_fn::cresult& result,
			const cio_packet::type_ptr& ptr_req_cur
		)
		{
			_process_exclusive_st_asy(result);
		}

		void cdev_ctl_fn::_process_shared_selected_session_st_asy_another_session_st_asy
		(
			cbase_ctl_fn::cresult& result,
			const cio_packet::type_ptr& ptr_req_cur
		)
		{
			cbase_ctl_fn::_cstate::type_evt evt = result.get_cur_event();

			cio_packet::type_ptr ptr_rsp;

			do {
				std::wstring s_dev_path;
				bool b_user_shared_mode(false);

				switch (evt)
				{
				case cbase_ctl_fn::_cstate::ev_open:
					result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_open);
					break;
				case cbase_ctl_fn::_cstate::ev_close:
					//another session 이 asy 상태이므로, 해당 session 의 키와 값만 map 에서 삭제 하면됨.
					result.after_processing_set_rsp_with_succss_complete(std::wstring(), m_s_dev_path);

					// 상태변경과 result 를 success 로 설정.
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_sync:
					// 새로운 request 를 내리면, another 는 하위단에서 자동 cancel 이 발생하므로 시키므로
					// 내리기 전에 , 실행 중인 another request에 복구 설정 해야함.
					m_mgmt_q.qm_read_set_request_front_to_recover_of_another_session(result.get_session_number());
					ptr_rsp = _start_and_complete_by_sync_req(m_s_dev_path, result.get_req());
					if (!ptr_rsp) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_processing_set_rsp_with_succss_complete(ptr_rsp, m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_asy:
					// 새로운 request 를 내리면, another 는 하위단에서 자동 cancel 이 발생하므로 시키므로
					// 내리기 전에 , 실행 중인 another request에 복구 설정 해야함.
					//>>m_mgmt_q.qm_read_set_request_front_to_recover_of_another_session(result.get_session_number());

					if (!_start_by_async_req(m_s_dev_path, result.get_req())) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_starting_process_set_rsp_with_succss_ing(m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);//<<
					break;
				case cbase_ctl_fn::_cstate::ev_rx:
					// 무시되는 event
					break;
				default:
					continue;
				}//end switch
			} while (false);
		}

		bool cdev_ctl_fn::_proc_event_open(cbase_ctl_fn::cresult& result, bool b_need_low_level)
		{
			bool b_result(false);
			std::wstring s_dev_path;
			bool b_user_shared_mode(false);

			do {
				std::tie(s_dev_path, b_user_shared_mode) = cbase_ctl_fn::_get_device_path_from_req(result.get_req());
				if (s_dev_path.empty()) {
					result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_path);

					m_p_ctl_fun_log->log_fmt(L"[E] - %ls | _get_device_path_from_req() : session = %u.\n", __WFUNCTION__, result.get_session_number());
					m_p_ctl_fun_log->trace(L"[E] - %ls | _get_device_path_from_req() : session = %u.\n", __WFUNCTION__, result.get_session_number());
					continue; //error 일때만 result 를 _get_device_path_from_open_req 에서 설정함.
				}

				if (b_need_low_level) {
					if (!cbase_ctl_fn::_open_device_by_req(s_dev_path, b_user_shared_mode, m_map_ptr_state_cur.size())) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_path, s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}
				}

				m_s_dev_path = s_dev_path;

				result.after_processing_set_rsp_with_succss_complete(std::wstring(), s_dev_path);

				//이 경우 에러 일겨우 상태변경은 없으므로, _transit_state_by_processing_result() 를 호출 할 필요 없음. 
				_transit_state_by_processing_result(result, b_user_shared_mode);

				b_result = true;
			} while (false);

			return b_result;
		}

		cbase_ctl_fn::cresult::type_ptr cdev_ctl_fn::_process_event_on_exclusive_mode
		(
			const cio_packet::type_ptr& ptr_req_new,
			const cio_packet::type_ptr& ptr_req_cur
		)
		{
			// 논린적으로 각 session 의 장비는 독립된 것으로 보기 때문에
			// 이벤트가 발생한 session 이 외의 session 의 state 에 영향은 없어야 함.
			// result.b_process_complete 이 true 면, result.ptr_rsp 에 결과가 설정되어야 함.

			//contructure, only increase reference request, isn't create response.
			cbase_ctl_fn::cresult::type_ptr ptr_result_new = std::make_shared<cbase_ctl_fn::cresult>(ptr_req_new, m_s_dev_path);
			cbase_ctl_fn::_cstate::type_state st_cur(cbase_ctl_fn::_cstate::st_not);

			std::tie(std::ignore, st_cur, std::ignore) = _get_state_cur_(ptr_result_new->get_session_number());

			do {
				switch (st_cur)
				{
				case cbase_ctl_fn::_cstate::st_not:
					//result 설정을 하위 함수에서 설정. rsp ptr 은 하위 함수에서 생성.
					_process_exclusive_st_not(*ptr_result_new);
					break;
				case cbase_ctl_fn::_cstate::st_idl:
					//result 설정을 하위 함수에서 설정. rsp ptr 은 하위 함수에서 생성..
					_process_exclusive_st_idl(*ptr_result_new);
					break;
				case cbase_ctl_fn::_cstate::st_asy:
					//result 설정을 하위 함수에서 설정.. rsp ptr 은 하위 함수에서 생성.
					_process_exclusive_st_asy(*ptr_result_new);
					break;
				default:
					// 에러 complete. rsp ptr 생성.
					ptr_result_new->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_session);
					continue;
				}

			} while (false);
			return ptr_result_new;
		}

		///////////////////////////////////////////////
		// exclusive mode sub processing
		///////////////////////////////////////////////
		void cdev_ctl_fn::_process_exclusive_st_not(cbase_ctl_fn::cresult& result)
		{
			//excluisive mode 에서 open 이 안된 상태.
			
			cbase_ctl_fn::_cstate::type_evt evt = result.get_cur_event();
			
			// state 가 st_not 인 경우는 모두 sync 타입.
			do {
				std::wstring s_dev_path;
				bool b_user_shared_mode(false);

				switch (evt)
				{
				case cbase_ctl_fn::_cstate::ev_open:
					_proc_event_open(result, true);
					break;
				case cbase_ctl_fn::_cstate::ev_close:
				case cbase_ctl_fn::_cstate::ev_sync:
				case cbase_ctl_fn::_cstate::ev_asy:
					result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_not_open);

					m_p_ctl_fun_log->log_fmt(L"[E] - %ls | error_reason_device_not_open : session = %u.\n", __WFUNCTION__, result.get_session_number());
					m_p_ctl_fun_log->trace(L"[E] - %ls | error_reason_device_not_open : session = %u.\n", __WFUNCTION__, result.get_session_number());
					break;
				case cbase_ctl_fn::_cstate::ev_rx:
					// 무시되는 event
					break;
				default:
					continue;
				}//end switch
			} while (false);
		}

		void cdev_ctl_fn::_process_exclusive_st_idl(cbase_ctl_fn::cresult& result)
		{
			// result.b_process_complete 이 true 면, result.ptr_rsp 에 결과가 설정되어야 함.
			cbase_ctl_fn::_cstate::type_evt evt = result.get_cur_event();

			// state 가 st_idl 인 경우는 asy 를 제외하고 모두 sync 타입.
			cio_packet::type_ptr ptr_rsp;

			do {

				switch (evt)
				{
				case cbase_ctl_fn::_cstate::ev_open:
					result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_open);
					break;
				case cbase_ctl_fn::_cstate::ev_close:
					// 현재 상태가 IDL 이므로 해당 session 이 있으면, 해당 session 의 키와 값만 map 에서 삭제 하면됨.
					result.after_processing_set_rsp_with_succss_complete(std::wstring(), m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_sync:
					ptr_rsp = _start_and_complete_by_sync_req(m_s_dev_path, result.get_req());
					if (!ptr_rsp) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_processing_set_rsp_with_succss_complete(ptr_rsp,m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_asy:
					if (!_start_by_async_req(m_s_dev_path, result.get_req())) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_starting_process_set_rsp_with_succss_ing(m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_rx:
					break;
				default:
					continue;
				}//end switch
			} while (false);
		}

		void cdev_ctl_fn::_process_exclusive_st_asy(cbase_ctl_fn::cresult& result)
		{
			// result.b_process_complete 이 true 면, result.ptr_rsp 에 결과가 설정되어야 함.
			cbase_ctl_fn::_cstate::type_evt evt = result.get_cur_event();

			// state 가 st_asy 인 경우는 asy 를 제외하고 모두 sync 타입.

			cio_packet::type_ptr ptr_rsp;

			do {

				switch (evt)
				{
				case cbase_ctl_fn::_cstate::ev_open:
					result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_open);
					break;
				case cbase_ctl_fn::_cstate::ev_close:
					if (!_start_and_complete_by_cancel_self_req(result.get_dev_path(), result.get_req())) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_processing_set_rsp_with_succss_complete(std::wstring(), m_s_dev_path);

					// 상태변경과 result 를 success 로 설정.
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_sync:
					// syc 명령을 실행하면 하단에서 현재 실행 중인 async 를 취소해서 결과가 callback 에서 설정됨.
					ptr_rsp = _start_and_complete_by_sync_req(m_s_dev_path, result.get_req());
					if (!ptr_rsp) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_processing_set_rsp_with_succss_complete(ptr_rsp, m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_asy:
					// asyc 명령을 실행하면 하단에서 현재 실행 중인 async 를 취소해서 결과가 callback 에서 설정됨.
					if (!_start_by_async_req(m_s_dev_path, result.get_req())) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_starting_process_set_rsp_with_succss_ing(m_s_dev_path);

					// 상태변경
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_rx:
					break;
				default:
					continue;
				}//end switch
			} while (false);
		}

		void cdev_ctl_fn::_transit_state_for_only_all_async_by_rx_event(	const cdev_ctl_fn::type_v_tuple_full& v_tuple_full	)
		{
			cbase_ctl_fn::_cstate::type_result_evt result_state(false, cbase_ctl_fn::_cstate::st_not, cbase_ctl_fn::_cstate::st_not);

			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_state);
				if (v_tuple_full.empty()) {
					continue;
				}

				for (auto item : v_tuple_full) {
					cio_packet::type_ptr ptr_req = std::get<0>(item);
					cio_packet::type_ptr ptr_rsp = std::get<3>(item);

					if (!ptr_req) {
						continue;
					}
					if (!ptr_rsp) {
						continue;
					}
					//
					unsigned long n_session = ptr_req->get_session_number();
					cbase_ctl_fn::_cstate::type_map_ptr_state::iterator it_sel = m_map_ptr_state_cur.find(n_session);
					if (it_sel == std::end(m_map_ptr_state_cur)) {
						// complete 된 것이라고 session 을 받았는데, 그 session 이 state map 에 없는 경우!
						// 뭔가 이상.
						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | session(%u) isn't in state map.\n", __WFUNCTION__, n_session);
						m_p_ctl_fun_log->trace(L"[E] - %ls | session(%u) isn't in state map.\n", __WFUNCTION__, n_session);

						continue;
					}

					if (ptr_req->get_action() == cio_packet::act_dev_open) {
						continue;
					}
					if (ptr_req->get_action() == cio_packet::act_dev_close) {
						continue;
					}

					auto evt = cbase_ctl_fn::_cstate::ev_rx;
					result_state = it_sel->second->set(evt);

					if (!std::get<0>(result_state)) {
						continue; // 상태 변화 없음.
					}

					cbase_ctl_fn::_cstate::type_state st_new(cbase_ctl_fn::_cstate::st_undefined);
					cbase_ctl_fn::_cstate::type_state st_old(cbase_ctl_fn::_cstate::st_undefined);

					st_new = std::get<1>(result_state);
					st_old = std::get<2>(result_state);

					if (!m_b_cur_shared_mode) {
						// exclusive mode 에서는 session 이 하나 이므로 combination state 와 session state 가 항상 같다.
						m_st_combi = st_new;
						// the end of exclusive mode
					}
					else {
						// shared mode 
						// generate combinatin state.
						auto exist_st = _get_state_another_(n_session);
						if (exist_st.first) {
							m_st_combi = exist_st.second;
						}
						else {// session 이 selected session 만 있음.
							m_st_combi = st_new;
						}
					}

				}//end for

#if defined(_WIN32) && defined(_DEBUG) && (defined(__THIS_FILE_ONLY__) || defined(__THIS_FILE_ONLY_STATE__))
				ATLTRACE(L"~~~~~ state info(%ls).\n", __WFUNCTION__);
#endif
				_logging_if_state_is_changed();

			} while (false);
		}

		void cdev_ctl_fn::_transit_state_by_processing_result
		(
			cbase_ctl_fn::cresult &result,
			bool b_user_shared_mode_on_open_request /*=false*/
		)
		{
			do {
				std::lock_guard<std::mutex> lock(m_mutex_for_state);

				cbase_ctl_fn::_cstate::type_result_evt result_state(false, cbase_ctl_fn::_cstate::st_not, cbase_ctl_fn::_cstate::st_not);

				//
				unsigned long n_session = result.get_session_number();
				cbase_ctl_fn::_cstate::type_map_ptr_state::iterator it_sel = m_map_ptr_state_cur.find(n_session);

				if (result.get_cur_event() == cbase_ctl_fn::_cstate::ev_open) {
					// 이 함수는 request 가 성공 일때만  호출되므로, session 이 map 에 없는 경우는 open request 일때 밖에 없음.
					if (m_map_ptr_state_cur.empty()) {
						// open 된 적 없을 때의 open 명령이므로 shared 또는 exclusive mode 변경.
						_reset_(b_user_shared_mode_on_open_request);
					}

					if (it_sel == std::end(m_map_ptr_state_cur)) {
						// open request 이므로 세션에 대한 state 를 map 에 생성.
						std::tie(it_sel, std::ignore) = m_map_ptr_state_cur.emplace(n_session, std::make_shared<cbase_ctl_fn::_cstate>());
					}
					result_state = it_sel->second->set(result.get_cur_event());
				}
				else if (result.get_cur_event() == cbase_ctl_fn::_cstate::ev_close) {
					if (it_sel == std::end(m_map_ptr_state_cur)) {
						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | session(%u) isn't in state map.\n", __WFUNCTION__, n_session);
						m_p_ctl_fun_log->trace(L"[E] - %ls | session(%u) isn't in state map.\n", __WFUNCTION__, n_session);
						continue;// error
					}
					result_state = it_sel->second->set(result.get_cur_event());

#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
					ATLTRACE(L"~~~~~ removed state(%u).\n",	it_sel->first);
#endif
					m_map_ptr_state_cur.erase(it_sel); //remove session
					//
					if (m_map_ptr_state_cur.empty()) {
						_reset_(false);// set default mode(exclusive mode)
						m_s_dev_path.clear();
					}
				}
				else {
					if (it_sel == std::end(m_map_ptr_state_cur)) {
						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | session(%u) isn't in state map.\n", __WFUNCTION__, n_session);
						m_p_ctl_fun_log->trace(L"[E] - %ls | session(%u) isn't in state map.\n", __WFUNCTION__, n_session);
						continue;// error
					}
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
					auto exist_st = _get_state_another_(n_session);
					if (exist_st.first) {
						result.set_combination_state(exist_st.second, m_st_combi);
					}
					else {// session 이 selected session 만 있음.
						result.set_combination_state(std::get<1>(result_state), m_st_combi);
					}
				}
				std::tie(m_st_combi, std::ignore) = result.get_combination_state();

#if defined(_WIN32) && defined(_DEBUG) && (defined(__THIS_FILE_ONLY__) || defined(__THIS_FILE_ONLY_STATE__))
				ATLTRACE(L"~~~~~ state info(%ls).\n", __WFUNCTION__);
#endif
				_logging_if_state_is_changed();
			} while (false);

		}

		cio_packet::type_ptr cdev_ctl_fn::_start_and_complete_by_sync_req
		(
			const std::wstring& s_dev_path,
			const cio_packet::type_ptr& ptr_req
		)
		{
			cio_packet::type_ptr ptr_rsp;

			do {
				if (!ptr_req) {
					continue;
				}

				cwait::type_ptr ptr_evt;
				unsigned long n_session = ptr_req->get_session_number();

				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					continue;
				}

				clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(s_dev_path);
				if (wptr_dev.expired()) {
					continue;
				}

				type_v_buffer v_tx;
				ptr_req->get_data_field(v_tx);

				switch (ptr_req->get_action())
				{
				case cio_packet::act_dev_transmit:
					std::tie(std::ignore, ptr_evt, std::ignore, ptr_rsp) = m_mgmt_q.qm_sync_push_back(ptr_req);
					wptr_dev.lock()->start_write_read(ptr_req->get_uid(),v_tx, cdev_ctl_fn::_cb_dev_for_sync_req, this, n_session);
					if (ptr_evt) {
						// 만약 자동 cancel 되는 request 가 있으면, 현재 request 보다 q 앞쪽에서 있기 때문에, cdev_ctl::_execute() 에서 자동 응답 client 에 전송됨.
						ptr_evt->wait_for_at_once();
					}
					break;
				case cio_packet::act_dev_cancel:
					std::tie(std::ignore, ptr_evt, std::ignore, ptr_rsp) = m_mgmt_q.qm_sync_push_back(ptr_req);
					wptr_dev.lock()->start_cancel(ptr_req->get_uid(), cdev_ctl_fn::_cb_dev_for_sync_req, this, n_session);
					if (ptr_evt) {
						// 만약 자동 cancel 되는 request 가 있으면, 현재 request 보다 q 앞쪽에서 있기 때문에, cdev_ctl::_execute() 에서 자동 응답 client 에 전송됨.
						ptr_evt->wait_for_at_once();
					}
					break;
				case cio_packet::act_dev_write:
					std::tie(std::ignore, ptr_evt, std::ignore, ptr_rsp) = m_mgmt_q.qm_sync_push_back(ptr_req);
					wptr_dev.lock()->start_write(ptr_req->get_uid(), v_tx, cdev_ctl_fn::_cb_dev_for_sync_req, this, n_session);
					if (ptr_evt) {
						// 만약 자동 cancel 되는 request 가 있으면, 현재 request 보다 q 앞쪽에서 있기 때문에, cdev_ctl::_execute() 에서 자동 응답 client 에 전송됨.
						ptr_evt->wait_for_at_once();
					}
					break;
				case cio_packet::act_dev_read:
				default:
					//error complete.
					continue;
				}//end switch

			} while (false);
			return ptr_rsp;
		}

		bool cdev_ctl_fn::_start_and_complete_by_cancel_self_req
		(
			const std::wstring& s_dev_path,
			const cio_packet::type_ptr& ptr_req

		)
		{
			bool b_result(false);

			do {
				if (!ptr_req) {
					continue;
				}

				cwait::type_ptr ptr_evt;
				unsigned long n_session = ptr_req->get_session_number();

				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					continue;
				}

				clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(s_dev_path);
				if (wptr_dev.expired()) {
					continue;
				}

				// create cancel request
				cio_packet req(*ptr_req);

				req.set_action(cio_packet::act_dev_cancel);
				req.set_cmd(cio_packet::cmd_self_request);
				req.set_data(type_v_buffer());

				std::tie(std::ignore, ptr_evt, std::ignore, std::ignore) = m_mgmt_q.qm_sync_push_back(ptr_req);
				wptr_dev.lock()->start_cancel(ptr_req->get_uid(), cdev_ctl_fn::_cb_dev_for_sync_req, this, n_session);
				if (ptr_evt) {
					ptr_evt->wait_for_at_once();
				}

				b_result = true;
			} while (false);
			return b_result;
		}
		bool cdev_ctl_fn::_start_by_async_req
		(
			const std::wstring& s_dev_path,
			const cio_packet::type_ptr& ptr_req
		)
		{
			bool b_result(false);
			cio_packet::type_ptr ptr_rsp;

			do {
				if (!ptr_req) {
					continue;
				}

				cwait::type_ptr ptr_evt;
				unsigned long n_session = ptr_req->get_session_number();

				_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
				if (!lib_hid.is_ini()) {
					continue;
				}

				clibhid_dev::type_wptr wptr_dev = lib_hid.get_device(s_dev_path);
				if (wptr_dev.expired()) {
					continue;
				}

				type_v_buffer v_tx;
				ptr_req->get_data_field(v_tx);

				switch (ptr_req->get_action())
				{
				case cio_packet::act_dev_read:
					if (m_mgmt_q.qm_read_push_back(ptr_req)) {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L"~~~~~ 1'st push req(%u : %ls).\n",
							ptr_req->get_session_number(),
							ptr_req->get_action_by_string().c_str()
						);
#endif
						if (m_b_cur_shared_mode) {
							wptr_dev.lock()->start_read(ptr_req->get_uid(), cdev_ctl_fn::_cb_dev_read_on_shared, this, n_session);
						}
						else {
							wptr_dev.lock()->start_read(ptr_req->get_uid(), cdev_ctl_fn::_cb_dev_read_on_exclusive, this, n_session);
						}
					}
					else {
#if defined(_WIN32) && defined(_DEBUG) && defined(__THIS_FILE_ONLY__)
						ATLTRACE(L"~~~~~ 2'nd... push req(%ls).\n",
							ptr_req->get_action_by_string().c_str()
						);
#endif
						if (m_b_cur_shared_mode) {
							wptr_dev.lock()->start_read(ptr_req->get_uid(), cdev_ctl_fn::_cb_dev_read_on_shared, this, n_session);
						}
						else {
							wptr_dev.lock()->start_read(ptr_req->get_uid(), cdev_ctl_fn::_cb_dev_read_on_exclusive, this, n_session);
						}

					}

					break;
				default:
					//error complete.
					continue;
				}//end switch

				b_result = true;
			} while (false);

			return b_result;
		}

		std::pair<bool, cdev_ctl_fn::cbase_ctl_fn::_cstate::type_state> cdev_ctl_fn::_get_state_another_(unsigned long n_session)
		{
			return cbase_ctl_fn::_cstate::get_combination_state_in_map_except_selected_session(m_map_ptr_state_cur, n_session);
		}


}//the end of _mp namespace

