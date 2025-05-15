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
#include <hid/_vhid_info_lpu237.h>

#if defined(_WIN32) && defined(_DEBUG)
#include <atltrace.h>
#endif


namespace _mp {
		cdev_ctl_fn::~cdev_ctl_fn()
		{
		}

		cdev_ctl_fn::cdev_ctl_fn(clog* p_log) : 
			cbase_ctl_fn(p_log),
			m_b_cur_shared_mode(false)
		{
			m_st_combi = cbase_ctl_fn::_cstate::st_not;
		}

		cdev_ctl_fn::type_tuple_full cdev_ctl_fn::_cq_mgmt::qm_sync_push_back(const cio_packet::type_ptr& ptr_request)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			auto r = m_q_sync_req_evt_rsp.push_back(ptr_request);

			//
			// m_map_ptr_q_ptr_cur_req_read �� ���� read ����� device start_X ��� ��������
			// ���� device io ���� �ڵ� ��� �Ǽ�, ���� read ��� ����� ������ callback �� cancel result �� ������ ȣ��ȴ�.
			// ����, m_map_ptr_q_ptr_cur_req_read �� ���� ����� �־ �ڵ����� ó���ȴ�.

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

			// key �� ������, �׻� value ���� ptr_q �� �׻� �Ҵ�Ǿ�� �Ѵ�.

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
				// start_read �� �߻���Ű�� read request ptr �� m_ptr_cur_req_read �� ����.
				std::tie(m_ptr_cur_req_read, std::ignore, std::ignore, std::ignore) = ptr_q->push_back(ptr_request);
			}
			else {
				ptr_q->push_back(ptr_request);
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
						// q �� item�� ������ �׻� q ��ü�� �Ҹ� ��Ŵ. 
						m_map_ptr_q_ptr_cur_req_read.erase(n_session_number);
					}
					continue;
				}

				//all session case
				it = std::begin(m_map_ptr_q_ptr_cur_req_read);

				for (; it != std::end(m_map_ptr_q_ptr_cur_req_read); ++it) {
					v_result.push_back(it->second->pop_front(b_remove));
					if (it->second->empty()) {
						// q �� item�� ������ �׻� q ��ü�� �Ҹ� ��Ŵ. 
						it = m_map_ptr_q_ptr_cur_req_read.erase(it);
						if (it == std::end(m_map_ptr_q_ptr_cur_req_read)) {
							break; //exit for
						}
					}
				}//end for

			} while (false);
			return v_result;
		}

		std::vector<cdev_ctl_fn::type_tuple_full> cdev_ctl_fn::_cq_mgmt::qm_read_set_response_front(cqitem_dev& qi)
		{
			cwait::type_ptr ptr_evt;
			cio_packet::type_ptr ptr_req, ptr_rsp;
			int n_evt(-1);
			std::vector<cdev_ctl_fn::type_tuple_full> v_result;

			do {
				std::lock_guard<std::mutex> lock(m_mutex);

				_cq_mgmt::_type_map_ptr_q::iterator it;

				if (m_map_ptr_q_ptr_cur_req_read.empty()) {
					continue;
				}

				it = std::begin(m_map_ptr_q_ptr_cur_req_read);

				if (qi.get_request_type() != cqitem_dev::req_cancel) {
					// cancel �� ���� ����� �ƴϸ�

					for (; it != std::end(m_map_ptr_q_ptr_cur_req_read); ++it) {
						auto r = it->second->pop_front(false);
						std::tie(ptr_req, ptr_evt, n_evt, ptr_rsp) = r;
						if (ptr_req) {
							if (ptr_req->is_recover_reserved()) {
								ptr_req->set_recover_reserve(false);// recover flag ����.
								continue;// recover flag �ִ� ���� response �� �������� ����.
							}
						}

						if (!ptr_evt) {
							continue;
						}
						if (!cbase_ctl_fn::_set_response(*ptr_rsp, qi, *ptr_req)) {
							continue;
						}
						v_result.push_back(r);

						ptr_evt->set(n_evt);
					}//end for
					continue;
				}

				// qi.get_request_type() == cqitem_dev::req_cancel
				// ���� session �� �ִ� �͸� ��� setting.
				for (; it != std::end(m_map_ptr_q_ptr_cur_req_read); ++it) {
					auto r = it->second->pop_front(false);
					ptr_evt = std::get<1>(r);
					if (!ptr_evt) {
						continue;
					}
					if (std::get<0>(r)->get_session_number() != qi.get_session_number()) {
						continue;
					}
					if (!cbase_ctl_fn::_set_response(*std::get<3>(r), qi, *std::get<0>(r))) {
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
					continue;
				}
				_cq_mgmt::_cq::type_ptr ptr_q = item.second;
				std::tie(ptr_req,std::ignore, std::ignore, std::ignore) = ptr_q->pop_front(false);
				if (!ptr_req) {
					continue;
				}
				ptr_req->set_recover_reserve(true);
				++n_cnt;
#if defined(_WIN32) && defined(_DEBUG)
				ATLTRACE(L"-- [%09u] SET recover flag %09u + %s\n", n_session_number, ptr_req->get_session_number());
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
					auto r = ptr_q->pop_front(false);// event set �� �͸� �����ؾ� �ϱ� ������ ���� pop �ϸ鼭 ���� �ϸ� �ȉ�.
					ptr_evt = std::get<1>(r);
					if (!ptr_evt) {
						continue;
					}
					n_evet = std::get<2>(r);

					if (n_evet != ptr_evt->wait_for_one_at_time(0)) {
						// not triggered
						continue;
					}

					// event set��.
					ptr_q->remove_front();// queue ���� ����.
					if (ptr_q->empty()) {
						// �� queue �� map���� ����.
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
					auto r = ptr_q->pop_front(false);// event set �� �͸� �����ؾ� �ϱ� ������ ���� pop �ϸ鼭 ���� �ϸ� �ȉ�.
					ptr_evt = std::get<1>(r);
					if (!ptr_evt) {
						continue;
					}
					n_evet = std::get<2>(r);

					if (n_evet != ptr_evt->wait_for_one_at_time(0)) {
						// not triggered
						continue;
					}

					// event set��.

					if (!ptr_v_req_rsp) {// return vector ����.
						ptr_v_req_rsp = std::make_shared<std::vector<std::pair<cio_packet::type_ptr, cio_packet::type_ptr>>>();
					}
					
					ptr_q->remove_front();// queue ���� ����.
					if (ptr_q->empty()) {
						// �� queue �� map���� ����.
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
					// q �� item�� ������ �׻� q ��ü�� �Ҹ� ��Ŵ. 
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
						// q �� item�� ������ �׻� q ��ü�� �Ҹ� ��Ŵ. 
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

			// ���� ��, ù event �����̹Ƿ� ������ event index �� �׻� 0.
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
		* callback ���� ������ �ٷ� �������� ����.
		*/
		bool cdev_ctl_fn::_cb_dev_read_on_exclusive(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			cio_packet::type_ptr ptr_rsp;
			
			do {
				bool b_pass_this_response(false);
				do{
					// ��ȣȭ ������ �����ϱ� ����, response �� set �ϱ� ���� ���� �˻�.
					// himalia ���� �߰��� ��ȣȭ ��� ������ ���� protocol �� ȣȯ�� ������ ���� �ʿ��� �ڵ�.
					std::tuple<cqitem_dev::type_result, type_v_buffer, std::wstring> rqi = qi.get_result_all();
					if (std::get<0>(rqi) != cqitem_dev::result_success) {
						continue;
					}
					if (std::get<1>(rqi).size() < 3) {
						continue;
					}
					std::array<char, 3> ar_c_len{ std::get<1>(rqi)[0], std::get<1>(rqi)[1], std::get<1>(rqi)[2] };
					if (ar_c_len[0] >= 0) {
						continue;
					}
					if (ar_c_len[1] >= 0) {
						continue;
					}
					if (ar_c_len[2] >= 0) {
						continue;
					}

					if(_vhid_info_lpu237::is_rx_msr_extension(std::get<1>(rqi))) {
						// ���䰪�� �� track len �� ���� ������ ������ ǥ���ϴ� ��������,
						// himalia ���� �߰��� ��ȣȭ ���� ������ ���� ������ Ȯ�εǾ�,
						// �������� ���� ��� ó��.
						continue;
					}

					b_pass_this_response = true; // �̹� ���� ����.

				} while (false);

				if (b_pass_this_response) {
					b_complete = false;
					continue;//more processing
				}
				
				// ���� ������ �������� ����. 
				std::vector<cdev_ctl_fn::type_tuple_full> v_tuple = p_obj->m_mgmt_q.qm_read_set_response_front(qi);
				if (v_tuple.empty()) {
					b_complete = false;
					continue;//more processing
				}
				//////////////////////


			} while (false);

			return b_complete;
		}

		/**
		* callback ���� ������ �ٷ� �������� ����.
		*/
		bool cdev_ctl_fn::_cb_dev_read_on_shared(cqitem_dev& qi, void* p_user)
		{
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			cio_packet::type_ptr ptr_rsp;

			do {
				bool b_pass_this_response(false);
				do {
					// ��ȣȭ ������ �����ϱ� ����, response �� set �ϱ� ���� ���� �˻�.
					// himalia ���� �߰��� ��ȣȭ ��� ������ ���� protocol �� ȣȯ�� ������ ���� �ʿ��� �ڵ�.
					std::tuple<cqitem_dev::type_result, type_v_buffer, std::wstring> rqi = qi.get_result_all();
					if (std::get<0>(rqi) != cqitem_dev::result_success) {
						continue;
					}
					if (std::get<1>(rqi).size() < 3) {
						continue;
					}
					std::array<char, 3> ar_c_len{ std::get<1>(rqi)[0], std::get<1>(rqi)[1], std::get<1>(rqi)[2] };
					if (ar_c_len[0] >= 0) {
						continue;
					}
					if (ar_c_len[1] >= 0) {
						continue;
					}
					if (ar_c_len[2] >= 0) {
						continue;
					}

					if (_vhid_info_lpu237::is_rx_msr_extension(std::get<1>(rqi))) {
						// ���䰪�� �� track len �� ���� ������ ������ ǥ���ϴ� ��������,
						// himalia ���� �߰��� ��ȣȭ ���� ������ ���� ������ Ȯ�εǾ�,
						// �������� ���� ��� ó��.
						continue;
					}

					b_pass_this_response = true; // �̹� ���� ����.

				} while (false);

				if (b_pass_this_response) {
					b_complete = false;
					continue;//more processing
				}

				//���� ���� ���¿����� ��� read �� ���� ������ �����ϰ� �������־�� ��.
				std::vector<cdev_ctl_fn::type_tuple_full> v_tuple = p_obj->m_mgmt_q.qm_read_set_response_front(qi);
				if (v_tuple.empty()) {
					b_complete = false;
					continue;//more processing
				}
				//////////////////////

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
			//callback ���� ������ �ٷ� �������� ����.
			bool b_complete(true);
			cdev_ctl_fn* p_obj((cdev_ctl_fn*)p_user);

			cio_packet::type_ptr ptr_req, ptr_rsp;
			cwait::type_ptr ptr_evt;
			int n_evt_index(-1);

			do {
				// pop ���� request ��°� ��� setting �� �и��Ǿ, ��������� event set �ɶ� ���� ��ٸ��ϱ� ����.
				std::tie(ptr_req, ptr_evt, n_evt_index, ptr_rsp) = p_obj->m_mgmt_q.qm_sync_pop_front(false);
				if (!ptr_req) {
					continue;// ��û�� session �� ������ �ƴϸ�, ����.
				}

				if (!cbase_ctl_fn::_set_response(*ptr_rsp, qi, *ptr_req)) {
					//���� ��û�� ���� ����� ���� ť���� �����ϸ� �ȉ�.
					b_complete = false;
					continue;//more processing
				}
				//
				if (ptr_evt) {
					ptr_evt->set(n_evt_index); //����� ���� ��ٸ��� event ��ٸ� ����.
				}

			} while (false);
			return b_complete;
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
				// ��� session �� reading( asyn ) ���� complete �� req�� ���� ���, reading Q ���� ����.
				ptr_v_req_rsp = m_mgmt_q.qm_read_pop_front_remove_complete_of_all_by_rsp_ptr();

				// sync ���� ����.
				cio_packet::type_ptr ptr_rsp_sync, ptr_req_sync;
				std::tie(ptr_req_sync, std::ignore, std::ignore, ptr_rsp_sync) = m_mgmt_q.qm_sync_pop_front(true);// pop nad remove

				if (ptr_rsp_sync) {
					if (!ptr_v_req_rsp) {
						ptr_v_req_rsp = std::make_shared<std::vector<std::pair<cio_packet::type_ptr, cio_packet::type_ptr>>>();
					}

					// �񵿱� ó�� ����� ����.
					// �񵿱� ����� Ư�� �� �����ϸ�, 1�� �ۿ� ����. 
					ptr_v_req_rsp->emplace_back(ptr_req_sync,ptr_rsp_sync);
				}

				if (!pair_ptr_req_ptr_rsp_additional.first) {
					continue;
				}
				if (!pair_ptr_req_ptr_rsp_additional.second) {
					continue;
				}

				// �̹� vector �� �ִ��� ����.
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
							// ������ req ã�� ����.
							b_already_exsit = true;
							break;// exit for
						}
					}//end for
				}

				if (b_already_exsit) {
					continue; // �̹� �����ϸ�, �߰����� �ʴ´�.
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

		cbase_ctl_fn::cresult::type_ptr cdev_ctl_fn::process_event
		(
			const cio_packet::type_ptr& ptr_req_new,
			const cio_packet::type_ptr& ptr_req_cur
		)
		{
			// �������� �� session �� ���� ������ ������ ���� ������
			// �̺�Ʈ�� �߻��� session �� ���� session �� state �� ������ ����� ��. 
			// result.b_process_complete �� true ��, result.ptr_rsp �� ����� �����Ǿ�� ��.
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
			// �������� �� session �� ���� ������ ������ ���� ������
			// �̺�Ʈ�� �߻��� session �� ���� session �� state �� ������ ����� ��.
			cbase_ctl_fn::_cstate::type_state st_cur(cbase_ctl_fn::_cstate::st_not), st_another(cbase_ctl_fn::_cstate::st_not);

			std::tie(std::ignore, st_cur, std::ignore ) = _get_state_cur_(ptr_req_new->get_session_number());

			// ���õ� session �� ������ ������ session �� ������ combination state�� ��´�.
			bool b_exist_another(false);
			std::tie(b_exist_another, st_another) = _get_state_another_(ptr_req_new->get_session_number());

			// ���µ� session state �� ������ session �� ���¸� ���� ���¸� ��´�.
			_cstate::type_state_sel_another st_sel_another = (_cstate::type_state_sel_another)((int)(st_cur * 100 + st_another));

			cbase_ctl_fn::cresult::type_ptr ptr_result_new = std::make_shared<cbase_ctl_fn::cresult>(ptr_req_new); //contructure, only increase reference request, isn't create response.

			do {// ���ο� req ���� ��, ���� ���� ����, req �� �ִ� ��찡 ���� ���.
				switch (st_sel_another)
				{
				case cbase_ctl_fn::_cstate::st_snot_anot://result ������ ���� �Լ����� ����.
					_process_shared_selected_session_st_not_another_session_st_not(*ptr_result_new);
					break;
				case cbase_ctl_fn::_cstate::st_snot_aidl://result ������ ���� �Լ����� ����.
					_process_shared_selected_session_st_not_another_session_st_idl(*ptr_result_new);
					break;
				case cbase_ctl_fn::_cstate::st_snot_aasy://result ������ ���� �Լ����� ����.
					_process_shared_selected_session_st_not_another_session_st_asy(*ptr_result_new, ptr_req_cur); //<< �����ϴ� ���� ����.
					break;

				case cbase_ctl_fn::_cstate::st_sidl_anot://result ������ ���� �Լ����� ����.
					_process_shared_selected_session_st_idl_another_session_st_not(*ptr_result_new);
					break;
				case cbase_ctl_fn::_cstate::st_sidl_aidl://result ������ ���� �Լ����� ����.
					_process_shared_selected_session_st_idl_another_session_st_idl(*ptr_result_new);
					break;
				case cbase_ctl_fn::_cstate::st_sidl_aasy://result ������ ���� �Լ����� ����.
					_process_shared_selected_session_st_idl_another_session_st_asy(*ptr_result_new, ptr_req_cur); //<< �����ϴ� ���� ����.
					break;

				case cbase_ctl_fn::_cstate::st_sasy_anot://result ������ ���� �Լ����� ����.
					_process_shared_selected_session_st_asy_another_session_st_not(*ptr_result_new, ptr_req_cur); //<< �����ϴ� ���� ����.
					break;
				case cbase_ctl_fn::_cstate::st_sasy_aidl://result ������ ���� �Լ����� ����.
					_process_shared_selected_session_st_asy_another_session_st_idl(*ptr_result_new, ptr_req_cur); //<< �����ϴ� ���� ����.
					break;
				case cbase_ctl_fn::_cstate::st_sasy_aasy://result ������ ���� �Լ����� ����.
					_process_shared_selected_session_st_asy_another_session_st_asy(*ptr_result_new, ptr_req_cur); //<< �����ϴ� ���� ����.
					break;
				default:
					// ���� complete.
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
			//excluisive mode ���� open �� �ȵ� ���¿� �����ϰ� ó��.
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
					// ���õǴ� event
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
					// ���õǴ� event
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
					// ���� ���°� IDL �̹Ƿ� �ش� session �� ������, �ش� session �� Ű�� ���� map ���� ���� �ϸ��.
					result.after_processing_set_rsp_with_succss_complete(std::wstring(), m_s_dev_path);

					// ���º���
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

					// ���º���
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

					// ���º���
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_rx:
					// ���õǴ� event
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
					// ���� ���°� IDL �̹Ƿ� �ش� session �� ������, �ش� session �� Ű�� ���� map ���� ���� �ϸ��.
					result.after_processing_set_rsp_with_succss_complete(std::wstring(), m_s_dev_path);

					// ���º���
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_sync:
					// ���ο� request �� ������, another �� �����ܿ��� �ڵ� cancel �� �߻��ϹǷ� ��Ű�Ƿ�
					// ������ ���� , ���� ���� another request�� ���� ���� �ؾ���.
					m_mgmt_q.qm_read_set_request_front_to_recover_of_another_session(result.get_session_number());
					ptr_rsp = _start_and_complete_by_sync_req(m_s_dev_path, result.get_req());
					if (!ptr_rsp) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_processing_set_rsp_with_succss_complete(ptr_rsp, m_s_dev_path);

					// ���º���
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_asy:
					// ���ο� request �� ������, another �� �����ܿ��� �ڵ� cancel �� �߻��ϹǷ� ��Ű�Ƿ�
					// ������ ���� , ���� ���� another request�� ���� ���� �ؾ���.

					// another session �� �̹� asyn �̹Ƿ�, asy queue �� ���� session �߰��� �ϸ�. OK.
					m_mgmt_q.qm_read_push_back(result.get_req());;

					result.after_starting_process_set_rsp_with_succss_ing(m_s_dev_path);

					// ���º���
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_rx:
					// ���õǴ� event
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
					//another session �� asy �����̹Ƿ�, �ش� session �� Ű�� ���� map ���� ���� �ϸ��.
					result.after_processing_set_rsp_with_succss_complete(std::wstring(), m_s_dev_path);

					// ���º���� result �� success �� ����.
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_sync:
					// ���ο� request �� ������, another �� �����ܿ��� �ڵ� cancel �� �߻��ϹǷ� ��Ű�Ƿ�
					// ������ ���� , ���� ���� another request�� ���� ���� �ؾ���.
					m_mgmt_q.qm_read_set_request_front_to_recover_of_another_session(result.get_session_number());
					ptr_rsp = _start_and_complete_by_sync_req(m_s_dev_path, result.get_req());
					if (!ptr_rsp) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_processing_set_rsp_with_succss_complete(ptr_rsp, m_s_dev_path);

					// ���º���
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_asy:
					// ���ο� request �� ������, another �� �����ܿ��� �ڵ� cancel �� �߻��ϹǷ� ��Ű�Ƿ�
					// ������ ���� , ���� ���� another request�� ���� ���� �ؾ���.
					m_mgmt_q.qm_read_set_request_front_to_recover_of_another_session(result.get_session_number());

					if (!_start_by_async_req(m_s_dev_path, result.get_req())) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_starting_process_set_rsp_with_succss_ing(m_s_dev_path);

					// ���º���
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_rx:
					// ���õǴ� event
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
					continue; //error �϶��� result �� _get_device_path_from_open_req ���� ������.
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

				//�� ��� ���� �ϰܿ� ���º����� �����Ƿ�, _transit_state_by_processing_result() �� ȣ�� �� �ʿ� ����. 
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
			// �������� �� session �� ���� ������ ������ ���� ������
			// �̺�Ʈ�� �߻��� session �� ���� session �� state �� ������ ����� ��.
			// result.b_process_complete �� true ��, result.ptr_rsp �� ����� �����Ǿ�� ��.

			//contructure, only increase reference request, isn't create response.
			cbase_ctl_fn::cresult::type_ptr ptr_result_new = std::make_shared<cbase_ctl_fn::cresult>(ptr_req_new);
			cbase_ctl_fn::_cstate::type_state st_cur(cbase_ctl_fn::_cstate::st_not);

			std::tie(std::ignore, st_cur, std::ignore) = _get_state_cur_(ptr_result_new->get_session_number());

			do {
				switch (st_cur)
				{
				case cbase_ctl_fn::_cstate::st_not:
					//result ������ ���� �Լ����� ����. rsp ptr �� ���� �Լ����� ����.
					_process_exclusive_st_not(*ptr_result_new);
					break;
				case cbase_ctl_fn::_cstate::st_idl:
					//result ������ ���� �Լ����� ����. rsp ptr �� ���� �Լ����� ����..
					_process_exclusive_st_idl(*ptr_result_new);
					break;
				case cbase_ctl_fn::_cstate::st_asy:
					//result ������ ���� �Լ����� ����.. rsp ptr �� ���� �Լ����� ����.
					_process_exclusive_st_asy(*ptr_result_new);
					break;
				default:
					// ���� complete. rsp ptr ����.
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
			//excluisive mode ���� open �� �ȵ� ����.
			
			cbase_ctl_fn::_cstate::type_evt evt = result.get_cur_event();
			
			// state �� st_not �� ���� ��� sync Ÿ��.
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
					// ���õǴ� event
					break;
				default:
					continue;
				}//end switch
			} while (false);
		}

		void cdev_ctl_fn::_process_exclusive_st_idl(cbase_ctl_fn::cresult& result)
		{
			// result.b_process_complete �� true ��, result.ptr_rsp �� ����� �����Ǿ�� ��.
			cbase_ctl_fn::_cstate::type_evt evt = result.get_cur_event();

			// state �� st_idl �� ���� asy �� �����ϰ� ��� sync Ÿ��.
			cio_packet::type_ptr ptr_rsp;

			do {

				switch (evt)
				{
				case cbase_ctl_fn::_cstate::ev_open:
					result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_open);
					break;
				case cbase_ctl_fn::_cstate::ev_close:
					// ���� ���°� IDL �̹Ƿ� �ش� session �� ������, �ش� session �� Ű�� ���� map ���� ���� �ϸ��.
					result.after_processing_set_rsp_with_succss_complete(std::wstring(), m_s_dev_path);

					// ���º���
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

					// ���º���
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

					// ���º���
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
			// result.b_process_complete �� true ��, result.ptr_rsp �� ����� �����Ǿ�� ��.
			cbase_ctl_fn::_cstate::type_evt evt = result.get_cur_event();

			// state �� st_asy �� ���� asy �� �����ϰ� ��� sync Ÿ��.

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

					// ���º���� result �� success �� ����.
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_sync:
					// syc ����� �����ϸ� �ϴܿ��� ���� ���� ���� async �� ����ؼ� ����� callback ���� ������.
					ptr_rsp = _start_and_complete_by_sync_req(m_s_dev_path, result.get_req());
					if (!ptr_rsp) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_processing_set_rsp_with_succss_complete(ptr_rsp, m_s_dev_path);

					// ���º���
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_asy:
					// asyc ����� �����ϸ� �ϴܿ��� ���� ���� ���� async �� ����ؼ� ����� callback ���� ������.
					if (!_start_by_async_req(m_s_dev_path, result.get_req())) {
						result.after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_operation, m_s_dev_path);

						m_p_ctl_fun_log->log_fmt(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						m_p_ctl_fun_log->trace(L"[E] - %ls | open counter = %u : session = %u.\n", __WFUNCTION__, m_map_ptr_state_cur.size(), result.get_session_number());
						continue;
					}

					result.after_starting_process_set_rsp_with_succss_ing(m_s_dev_path);

					// ���º���
					_transit_state_by_processing_result(result);
					break;
				case cbase_ctl_fn::_cstate::ev_rx:
					break;
				default:
					continue;
				}//end switch
			} while (false);
		}

		void cdev_ctl_fn::_transit_state_by_processing_result
		(
			cbase_ctl_fn::cresult &result,
			bool b_user_shared_mode_on_open_request /*=false*/
		)
		{
			cbase_ctl_fn::_cstate::type_result_evt result_state(false,cbase_ctl_fn::_cstate::st_not, cbase_ctl_fn::_cstate::st_not);

			//
			unsigned long n_session = result.get_session_number();
			cbase_ctl_fn::_cstate::type_map_ptr_state::iterator it_sel = m_map_ptr_state_cur.find(n_session);

			if (result.get_cur_event() == cbase_ctl_fn::_cstate::ev_open) {
				// �� �Լ��� request �� ���� �϶���  ȣ��ǹǷ�, session �� map �� ���� ���� open request �϶� �ۿ� ����.
				if (m_map_ptr_state_cur.empty()) {
					// open �� �� ���� ���� open ����̹Ƿ� shared �Ǵ� exclusive mode ����.
					_reset_(b_user_shared_mode_on_open_request);
				}

				if (it_sel == std::end(m_map_ptr_state_cur)) {
					// open request �̹Ƿ� ���ǿ� ���� state �� map �� ����.
					std::tie(it_sel, std::ignore) = m_map_ptr_state_cur.emplace(n_session, std::make_shared<cbase_ctl_fn::_cstate>());
				}
				result_state = it_sel->second->set(result.get_cur_event());
			}
			else if (result.get_cur_event() == cbase_ctl_fn::_cstate::ev_close) {
				result_state = it_sel->second->set(result.get_cur_event());

				m_map_ptr_state_cur.erase(it_sel); //remove session
				//
				if (m_map_ptr_state_cur.empty()) {
					_reset_(false);// set default mode(exclusive mode)
					m_s_dev_path.clear();
				}
			}
			else {
				result_state = it_sel->second->set(result.get_cur_event());
			}

			result.set_selected_session_state(result_state);

			if (!m_b_cur_shared_mode) {
				// exclusive mode ������ session �� �ϳ� �̹Ƿ� combination state �� session state �� �׻� ����.
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
				else {// session �� selected session �� ����.
					result.set_combination_state(std::get<1>(result_state), m_st_combi);
				}
			}
			std::tie(m_st_combi, std::ignore) = result.get_combination_state();
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
					wptr_dev.lock()->start_write_read(v_tx, cdev_ctl_fn::_cb_dev_for_sync_req, this, n_session);
					if (ptr_evt) {
						// ���� �ڵ� cancel �Ǵ� request �� ������, ���� request ���� q ���ʿ��� �ֱ� ������, cdev_ctl::_execute() ���� �ڵ� ���� client �� ���۵�.
						ptr_evt->wait_for_at_once();
					}
					break;
				case cio_packet::act_dev_cancel:
					std::tie(std::ignore, ptr_evt, std::ignore, ptr_rsp) = m_mgmt_q.qm_sync_push_back(ptr_req);
					wptr_dev.lock()->start_cancel(cdev_ctl_fn::_cb_dev_for_sync_req, this, n_session);
					if (ptr_evt) {
						// ���� �ڵ� cancel �Ǵ� request �� ������, ���� request ���� q ���ʿ��� �ֱ� ������, cdev_ctl::_execute() ���� �ڵ� ���� client �� ���۵�.
						ptr_evt->wait_for_at_once();
					}
					break;
				case cio_packet::act_dev_write:
					std::tie(std::ignore, ptr_evt, std::ignore, ptr_rsp) = m_mgmt_q.qm_sync_push_back(ptr_req);
					wptr_dev.lock()->start_write(v_tx, cdev_ctl_fn::_cb_dev_for_sync_req, this, n_session);
					if (ptr_evt) {
						// ���� �ڵ� cancel �Ǵ� request �� ������, ���� request ���� q ���ʿ��� �ֱ� ������, cdev_ctl::_execute() ���� �ڵ� ���� client �� ���۵�.
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
				wptr_dev.lock()->start_cancel(cdev_ctl_fn::_cb_dev_for_sync_req, this, n_session);
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
						if (m_b_cur_shared_mode) {
							wptr_dev.lock()->start_read(cdev_ctl_fn::_cb_dev_read_on_shared, this, n_session);
						}
						else {
							wptr_dev.lock()->start_read(cdev_ctl_fn::_cb_dev_read_on_exclusive, this, n_session);
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

