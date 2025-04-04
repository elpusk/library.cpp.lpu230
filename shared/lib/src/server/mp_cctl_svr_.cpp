#include <websocket/mp_win_nt.h>

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include <mp_type.h>
#include <server/mp_cserver_.h>
#include <server/mp_cctl_svr_.h>
#include <server/mp_cmain_ctl_.h>
#include <server/mp_ckernel_ctl_.h>
#include <server/mp_cdev_ctl_.h>
#include <hid/mp_clibhid.h>

namespace _mp {

	/**
	 * the saved information in this class.
	 * - the relation between session number and session name.
	 * - main (worker) control.
	 * - the relation between device index and device (worker) control.
	 * - the relation betwwen session number and the set of device index.
	 */

		cctl_svr::~cctl_svr()
		{
		}

		cctl_svr& cctl_svr::create_kernel_ctl(clog* p_log, unsigned long n_session)
		{
			std::lock_guard<std::mutex> lock(m_mutex_map_session_to_kernel);
			if (m_map_session_to_ptr_kernel_ctl.find(n_session) == std::end(m_map_session_to_ptr_kernel_ctl)) {
				m_map_session_to_ptr_kernel_ctl[n_session] = cworker_ctl::type_ptr(new ckernel_ctl(p_log));
			}
			return *this;
		}

		void cctl_svr::remove_kernel_ctl(unsigned long n_session)
		{
			std::lock_guard<std::mutex> lock(m_mutex_map_session_to_kernel);
			auto it = m_map_session_to_ptr_kernel_ctl.find(n_session);
			if ( it != std::end(m_map_session_to_ptr_kernel_ctl)) {
				m_map_session_to_ptr_kernel_ctl.erase(it);
			}
		}

		cctl_svr& cctl_svr::create_main_ctl_and_set_callack(clog* p_log)
		{
			m_ptr_main_ctl = cworker_ctl::type_ptr(new cmain_ctl(p_log));
			

			clibhid::get_instance().set_callback_pluginout(cctl_svr::_cb_dev_pluginout, this);
			return *this;
		}

		void cctl_svr::_cb_dev_pluginout(
			const clibhid_dev_info::type_set& set_inserted,
			const clibhid_dev_info::type_set& set_removed,
			const clibhid_dev_info::type_set& set_current,
			void* p_usr
		)
		{
			cctl_svr* p_obj = (cctl_svr*)p_usr;

			std::set<unsigned short> set_removed_dev_index;

			unsigned long n_session(_MP_TOOLS_INVALID_SESSION_NUMBER);

			do {
				//build set of device index.
				for (const clibhid_dev_info& item : set_removed) {
					std::wstring s = item.get_path_by_wstring();
					unsigned short w = p_obj->_get_device_index_from_device_path(s);
					if (w == _MP_TOOLS_INVALID_DEVICE_INDEX) {
						continue;
					}
					set_removed_dev_index.insert(w);
					n_session = p_obj->get_session_number_from_device_index(w);
					cserver::get_instance().send_data_to_client_by_ip4_device_plug_out(
						n_session,
						w,
						s
					);
				}//end for

				for (const clibhid_dev_info& item : set_inserted) {
					std::wstring s = item.get_path_by_wstring();
					cserver::get_instance().broadcast_by_ip4_device_plug_in(s);
				}//end for

				if (!set_removed.empty()) {
					//update m_map_session_to_set_of_device_index
					do {
						std::lock_guard<std::mutex> lock(p_obj->m_mutex_map_session_to_device_index);
						for (auto item : p_obj->m_map_session_to_set_of_device_index) {
							for (auto index : set_removed_dev_index) {
								item.second.erase(index);
							}//end for set
						}//end for map

					} while (false);
					
					//update m_map_device_index_to_ptr_dev_ctl
					do {
						std::lock_guard<std::mutex> lock(p_obj->m_mutex_device_map);
						for (auto index : set_removed_dev_index) {
							p_obj->m_map_device_index_to_ptr_dev_ctl.erase(index);
						}//end for set

					} while (false);
				}//the end of removed.

				if (!set_inserted.empty()) {
					//update m_map_device_index_to_ptr_dev_ctl
					do {
						for (auto item : set_inserted) {
							unsigned short w = p_obj->create_new_dev_ctl(&clog::get_instance(), item);
							if (w == _MP_TOOLS_INVALID_DEVICE_INDEX) {
								continue;
							}
						}//end for set
					} while (false);
				}

				// send to clients !!!
				//TODO.

			} while (false);
		}

		// >> m_map_device_index_to_wptr_dev_ctl
		unsigned short cctl_svr::create_new_dev_ctl(clog* p_log, const clibhid_dev_info &item)
		{
			unsigned short w_result(0);
			do {
				if (!p_log)
					continue;

				cworker_ctl::type_ptr ptr_dev_ctl(new cdev_ctl(p_log));
				ptr_dev_ctl->set_dev_path(item.get_path_by_wstring());
				//
				std::lock_guard<std::mutex> lock(m_mutex_device_map);
				ptr_dev_ctl->set_device_index(m_w_new_device_index);
				w_result = m_w_new_device_index;

				cctl_svr::_type_map_device_index_to_ptr_dev_ctl::iterator it = m_map_device_index_to_ptr_dev_ctl.find(m_w_new_device_index);
				if (it != std::end(m_map_device_index_to_ptr_dev_ctl)) {
					m_map_device_index_to_ptr_dev_ctl.erase(it);
				}
				m_map_device_index_to_ptr_dev_ctl.emplace(m_w_new_device_index, ptr_dev_ctl);
				++m_w_new_device_index;
				if (_MP_TOOLS_INVALID_DEVICE_INDEX == m_w_new_device_index) {
					++m_w_new_device_index;
				}

				p_log->log_fmt(L"[I] - %ls | inserted(%u) : %ls.\n", __WFUNCTION__, w_result,item.get_path_by_wstring().c_str());
				p_log->trace(L"[I] - %ls | inserted(%u) : %ls.\n", __WFUNCTION__, w_result,item.get_path_by_wstring().c_str());
			} while (false);
			return w_result;
		}

		unsigned short cctl_svr::remove_dev_ctl(const std::wstring& s_device_path)
		{
			unsigned short w_device_index(_MP_TOOLS_INVALID_DEVICE_INDEX);
			bool b_found(false);

			do {
				if (s_device_path.empty())
					continue;

				std::lock_guard<std::mutex> lock(m_mutex_device_map);
				cctl_svr::_type_map_device_index_to_ptr_dev_ctl::iterator it = std::begin(m_map_device_index_to_ptr_dev_ctl);
				for (; it != std::end(m_map_device_index_to_ptr_dev_ctl); ++it) {

					do {
						if (!it->second)
							continue;
						std::wstring s_device_path_of_ctl = it->second->get_dev_path();

						if (s_device_path_of_ctl.compare(s_device_path) != 0)
							continue;
						//
						w_device_index = it->first;
						m_map_device_index_to_ptr_dev_ctl.erase(it);

						clog::get_instance().log_fmt(L"[I] - %ls | removed : %ls.\n", __WFUNCTION__, s_device_path.c_str());
						clog::get_instance().trace(L"[I] - %ls | removed : %ls.\n", __WFUNCTION__, s_device_path.c_str());

						b_found = true;
					} while (false);

					if (b_found)
						break;//exit for
				}//end for
			} while (false);
			return w_device_index;
		}

		// >> for m_map_session_to_set_of_device_index
		cctl_svr::type_set_device_index cctl_svr::get_device_index_set_of_session(unsigned long n_session)
		{
			unsigned short w_device_index(0);
			cctl_svr::type_set_device_index set_index;

			do {
				std::lock_guard<std::mutex> lock(m_mutex_map_session_to_device_index);
				cctl_svr::_type_map_session_to_set_of_device_index::iterator it = m_map_session_to_set_of_device_index.find(n_session);
				if (it == std::end(m_map_session_to_set_of_device_index))
					continue;
				set_index = it->second;
			} while (false);
			return set_index;
		}

		void cctl_svr::insert_device_index_to_device_index_set_of_session(unsigned long n_session, unsigned short w_device_index)
		{
			do {
				if (w_device_index == 0)
					continue;
				std::lock_guard<std::mutex> lock(m_mutex_map_session_to_device_index);
				cctl_svr::_type_map_session_to_set_of_device_index::iterator it = m_map_session_to_set_of_device_index.find(n_session);
				if (it == std::end(m_map_session_to_set_of_device_index)) {
					//insert new session
					cctl_svr::type_set_device_index set_device_index;
					set_device_index.insert(w_device_index);
					m_map_session_to_set_of_device_index[n_session] = set_device_index;
					continue;
				}
				cctl_svr::type_set_device_index::iterator it_index = it->second.find(w_device_index);
				if (it_index != std::end(it->second))
					continue;//already exist
				//add new device index
				it->second.insert(w_device_index);
			} while (false);
		}

		void cctl_svr::erase_device_index_from_device_index_set_of_session(unsigned long n_session, unsigned short w_device_index)
		{
			do {
				std::lock_guard<std::mutex> lock(m_mutex_map_session_to_device_index);
				cctl_svr::_type_map_session_to_set_of_device_index::iterator it = m_map_session_to_set_of_device_index.find(n_session);
				if (it == std::end(m_map_session_to_set_of_device_index))
					continue;
				cctl_svr::type_set_device_index::iterator it_index = it->second.find(w_device_index);
				if (it_index == std::end(it->second))
					continue;
				//
				it->second.erase(it_index);

			} while (false);
		}
		// <<

		// >> for m_map_session_to_name
		/**
		 * set_session_name.
		 * if the session have the name, this method will be failed.
		 *
		 * \param n_session - target session number
		 * \param s_session_name - the name of session
		 * \return boolean
		 */
		bool cctl_svr::cctl_svr::set_session_name(unsigned long n_session, const std::wstring& s_session_name)
		{
			bool b_result(false);

			do {
				if (s_session_name.empty())
					continue;
				cctl_svr::_type_map_session_to_name::iterator it = m_map_session_to_name.find(n_session);
				if (it != std::end(m_map_session_to_name))
					continue;
				//
				m_map_session_to_name[n_session] = s_session_name;
				b_result = true;
			} while (false);

			return b_result;
		}

		std::wstring cctl_svr::get_session_name(unsigned long n_session)
		{
			std::wstring s_name;

			do {
				cctl_svr::_type_map_session_to_name::iterator it = m_map_session_to_name.find(n_session);
				if (it == std::end(m_map_session_to_name))
					continue;
				s_name = it->second;
			} while (false);

			return s_name;
		}

		bool cctl_svr::is_exist_session_name(const std::wstring& s_session_name)
		{
			bool b_found(false);
			do {
				if (s_session_name.empty())
					continue;
				cctl_svr::_type_map_session_to_name::iterator it = std::begin(m_map_session_to_name);
				for (; it != std::end(m_map_session_to_name); ++it) {
					if (it->second.compare(s_session_name) == 0) {
						//found
						b_found = true;
						break;
					}
				}//end for
			} while (false);
			return b_found;
		}

		void cctl_svr::remove_session_name(unsigned long n_session)
		{
			do {
				cctl_svr::_type_map_session_to_name::iterator it = m_map_session_to_name.find(n_session);
				if (it == std::end(m_map_session_to_name))
					continue;
				m_map_session_to_name.erase(it);
			} while (false);
		}
		unsigned long cctl_svr::get_session_number_from_session_name(const std::wstring& s_session_name)
		{
			unsigned long n_session(_MP_TOOLS_INVALID_SESSION_NUMBER);

			do {
				if (s_session_name.empty())
					continue;
				cctl_svr::_type_map_session_to_name::iterator it = std::begin(m_map_session_to_name);
				for (; it != std::end(m_map_session_to_name); ++it) {
					if (it->second.compare(s_session_name) == 0) {
						//found
						n_session = it->first;
						break;
					}
				}//end for
			} while (false);
			return n_session;
		}
		std::wstring cctl_svr::generate_session_name(unsigned long n_session)
		{
			std::wstring s_name(std::to_wstring(n_session));
			unsigned long n_count(0);
			std::wstring s_full_name;
			s_name += L"_auto_by_cm_";

			do {
				s_full_name = s_name + std::to_wstring(n_count);
				++n_count;
			} while (is_exist_session_name(s_full_name));
			return s_full_name;
		}
		// <<

		void cctl_svr::display(const std::wstring& s_info) {}
		void cctl_svr::display(unsigned long  dw_color, const std::wstring& s_info) {}
		void cctl_svr::display(unsigned long  dw_color, const type_v_buffer& v_data) {}

		bool cctl_svr::push_request_to_worker_ctl(cio_packet::type_ptr& ptr_request, std::wstring& s_error_reason)
		{
			bool b_result(false);

			do {
				type_set_wstring set_dev_path;
				size_t n_dev(0);

				if (ptr_request->is_response()) {
					//this section for error response
					_push_back_request_to_main_ctl(ptr_request);
					b_result = true;
					continue;
				}

				if (ptr_request->get_action() == cio_packet::act_mgmt_dev_kernel_operation) {
					_push_back_request_to_kernel_ctl(ptr_request);
					b_result = true;
					continue;
				}


				if (ptr_request->is_owner_manager()) {
					switch (ptr_request->get_action())
					{
					case cio_packet::act_mgmt_get_echo:
					case cio_packet::act_mgmt_get_device_list:
					case cio_packet::act_mgmt_ctl_show:
					case cio_packet::act_mgmt_file_operation:
					case cio_packet::act_mgmt_advance_operation:
						_push_back_request_to_main_ctl(ptr_request);
						break;
					default:
						s_error_reason = cio_packet::get_error_message(cio_packet::error_reason_action_code);
						clog::get_instance().log_fmt(L"[E] - %ls | unknown action_code of manager.\n", __WFUNCTION__);
						clog::get_instance().trace(L"[E] - %ls | unknown action_code of manager.\n", __WFUNCTION__);
						continue;
					}//end switch

					b_result = true;
					continue;
				}

				// request to device.
				std::wstring s_path, s_target;
				type_list_wstring list_wstring;
				type_list_wstring list_wstring_token;
				type_list_wstring::iterator it_token;

				switch (ptr_request->get_action())
				{
				case cio_packet::act_dev_open:
					if (ptr_request->get_data_field(s_path) == 0) {
						s_error_reason = cio_packet::get_error_message(cio_packet::error_reason_device_path);
						clog::get_instance().log_fmt(L"[E] - %ls | no device path.\n", __WFUNCTION__);
						clog::get_instance().trace(L"[E] - %ls | no device path.\n", __WFUNCTION__);
						continue;
					}


					if (!_push_back_request_to_dev_ctl(ptr_request, s_path)) {
						s_error_reason = cio_packet::get_error_message(cio_packet::error_reason_device_object);
						clog::get_instance().log_fmt(L"[E] - %ls | %ls | no device page object | %ls.\n", __WFUNCTION__, cio_packet::get_action_code_by_string(ptr_request->get_action()).c_str(),s_path.c_str());
						clog::get_instance().trace(L"[E] - %ls | %ls |no device page object | %ls.\n", __WFUNCTION__, cio_packet::get_action_code_by_string(ptr_request->get_action()).c_str(),s_path.c_str());
						continue;
					}
					break;
				case cio_packet::act_dev_close:
				case cio_packet::act_dev_write:
				case cio_packet::act_dev_read:
				case cio_packet::act_dev_transmit:
				case cio_packet::act_dev_cancel:
				case cio_packet::act_dev_sub_bootloader:
					if (!_push_back_request_to_dev_ctl(ptr_request)) {
						s_error_reason = cio_packet::get_error_message(cio_packet::error_reason_device_object);
						clog::get_instance().log_fmt(L"[E] - %ls | %ls | no device page object : device index = %u.\n", __WFUNCTION__, cio_packet::get_action_code_by_string(ptr_request->get_action()).c_str(), ptr_request->get_device_index());
						clog::get_instance().trace(L"[E] - %ls | %ls | no device page object : device index = %u.\n", __WFUNCTION__, cio_packet::get_action_code_by_string(ptr_request->get_action()).c_str(),ptr_request->get_device_index());
						continue;
					}
					break;
				case cio_packet::act_mgmt_dev_kernel_operation:
					if (ptr_request->get_data_field(list_wstring) > 1) {
						//
						if (cconvert::tokenizer(list_wstring_token, list_wstring.front(), L" ") == 2) {
							it_token = std::begin(list_wstring_token);
							s_target = list_wstring_token.back();
							if ((list_wstring_token.front().compare(L"open") == 0) && (s_target.compare(L"device") == 0)) {
								s_path = *(++std::begin(list_wstring));
								if (!_push_back_request_to_dev_ctl(ptr_request, s_path)) {
									s_error_reason = cio_packet::get_error_message(cio_packet::error_reason_device_object);
									clog::get_instance().log_fmt(L"[E] - %ls | no device page object | %ls.\n", __WFUNCTION__, s_path.c_str());
									clog::get_instance().trace(L"[E] - %ls | no device page object | %ls.\n", __WFUNCTION__, s_path.c_str());
									continue;
								}
								break;
							}
						}
					}

					if (!_push_back_request_to_dev_ctl(ptr_request)) {
						s_error_reason = cio_packet::get_error_message(cio_packet::error_reason_device_object);
						clog::get_instance().log_fmt(L"[E] - %ls | %ls | no device page object : device index = %u.\n", __WFUNCTION__, cio_packet::get_action_code_by_string(ptr_request->get_action()).c_str(),ptr_request->get_device_index());
						clog::get_instance().trace(L"[E] - %ls | %ls | no device page object : device index = %u.\n", __WFUNCTION__, cio_packet::get_action_code_by_string(ptr_request->get_action()).c_str(), ptr_request->get_device_index());
						continue;
					}
					break;
				default:
					s_error_reason = cio_packet::get_error_message(cio_packet::error_reason_action_code);
					clog::get_instance().log_fmt(L"[E] - %ls | unknown action_code of device.\n", __WFUNCTION__);
					clog::get_instance().trace(L"[E] - %ls | unknown action_code of device.\n", __WFUNCTION__);
					continue;
				}//end switch

				b_result = true;
			} while (false);
			return b_result;
		}

		bool cctl_svr::close_all_device_of_session(unsigned long n_session)
		{
			bool b_result(false);
			do {
				std::wstring s_error_reason;

				cctl_svr::type_set_device_index set_device_index(get_device_index_set_of_session(n_session));
				if (set_device_index.empty()) {
					clog::get_instance().log_fmt(L"[E] - %ls | none device in session %u.\n", __WFUNCTION__, n_session);
					clog::get_instance().trace(L"[E] - %ls | none device in session %u.\n", __WFUNCTION__, n_session);
					continue;
				}
				_erase_device_index_set_of_session(n_session);

				std::wstring s_path;

				for (auto w_device_index : set_device_index) {
					if (w_device_index == 0)
						continue;
					s_path = _get_device_path_from_device_index(w_device_index);
					if (s_path.empty())
						continue;//already removed device data.

					//cancel current processing
					cio_packet::type_ptr ptr_request_cancel = cio_packet::build_cancel(n_session, w_device_index);
					if (!push_request_to_worker_ctl(ptr_request_cancel, s_error_reason)) {//automatic cancel device
						clog::get_instance().log_fmt(L"[E] - %ls | _push_request_of_client() of cancel.\n", __WFUNCTION__);
						clog::get_instance().trace(L"[E] - %ls | _push_request_of_client() of cancel.\n", __WFUNCTION__);
					}

					//close device.
					cio_packet::type_ptr ptr_request_close = cio_packet::build_close(n_session, w_device_index);
					if (!push_request_to_worker_ctl(ptr_request_close, s_error_reason)) {//automatic close device
						clog::get_instance().log_fmt(L"[E] - %ls | _push_request_of_client() of close.\n", __WFUNCTION__);
						clog::get_instance().trace(L"[E] - %ls | _push_request_of_client() of close.\n", __WFUNCTION__);
					}
				}//end for

				b_result = true;
			} while (false);
			return b_result;
		}
		//
		cctl_svr::cctl_svr() : m_w_new_device_index(1)
		{
		}

		bool cctl_svr::_push_back_request_to_kernel_ctl(cio_packet::type_ptr& ptr_cio_packet)
		{
			bool b_result(false);

			do {
				auto n_session_number = ptr_cio_packet->get_session_number();
				std::lock_guard<std::mutex> lock(m_mutex_map_session_to_kernel);
				auto iter = m_map_session_to_ptr_kernel_ctl.find(n_session_number);
				if (iter == std::end(m_map_session_to_ptr_kernel_ctl))
					continue;
				if (!iter->second)
					continue;
				//
				iter->second->push_back_request(ptr_cio_packet);
				b_result = true;
			} while (false);
			return b_result;
		}

		bool cctl_svr::_push_back_request_to_main_ctl(cio_packet::type_ptr& ptr_cio_packet)
		{
			bool b_result(false);

			do {
				if (!m_ptr_main_ctl)
					continue;
				m_ptr_main_ctl->push_back_request(ptr_cio_packet);
				b_result = true;
			} while (false);
			return b_result;
		}
		bool cctl_svr::_push_back_request_to_dev_ctl(cio_packet::type_ptr& ptr_cio_packet)
		{
			bool b_result(false);
			do {
				if (!ptr_cio_packet)
					continue;
				cworker_ctl::type_ptr ptr_dev_ctl = _get_dev_ctl((unsigned short)ptr_cio_packet->get_device_index());
				if (!ptr_dev_ctl)
					continue;
				ptr_dev_ctl->push_back_request(ptr_cio_packet);
				b_result = true;
			} while (false);
			return b_result;
		}
		bool cctl_svr::_push_back_request_to_dev_ctl(cio_packet::type_ptr& ptr_cio_packet, const std::wstring& s_device_path)
		{
			bool b_result(false);
			do {
				if (!ptr_cio_packet) {
					clog::get_instance().log_fmt(L"[E] - %ls | ptr_cio_packet is nulllptr.\n", __WFUNCTION__);
					clog::get_instance().trace(L"[E] - %ls | ptr_cio_packet is nulllptr.\n", __WFUNCTION__);
					continue;
				}
				cworker_ctl::type_ptr ptr_dev_ctl = _get_dev_ctl(s_device_path);
				if (!ptr_dev_ctl) {
					clog::get_instance().log_fmt(L"[E] - %ls | _get_dev_ctl() none | %ls.\n", __WFUNCTION__, s_device_path.c_str());
					clog::get_instance().trace(L"[E] - %ls | _get_dev_ctl() none | %ls.\n", __WFUNCTION__, s_device_path.c_str());
					continue;
				}
				ptr_cio_packet->set_device_index(ptr_dev_ctl->get_device_index());
				ptr_dev_ctl->push_back_request(ptr_cio_packet);
				b_result = true;
			} while (false);
			return b_result;
		}

		void cctl_svr::_erase_device_index_set_of_session(unsigned long n_session)
		{
			std::lock_guard<std::mutex> lock(m_mutex_map_session_to_device_index);
			cctl_svr::_type_map_session_to_set_of_device_index::iterator it = m_map_session_to_set_of_device_index.find(n_session);
			if (it != std::end(m_map_session_to_set_of_device_index))
				m_map_session_to_set_of_device_index.erase(it);
		}

		/**
		* m_mutex_device_map is used!
		*/
		cworker_ctl::type_ptr cctl_svr::_get_dev_ctl(unsigned short w_device_index)
		{
			cworker_ctl::type_ptr ptr_dev_ctl;
			do {
				std::lock_guard<std::mutex> lock(m_mutex_device_map);
				cctl_svr::_type_map_device_index_to_ptr_dev_ctl::iterator it = m_map_device_index_to_ptr_dev_ctl.find(w_device_index);
				if (it == std::end(m_map_device_index_to_ptr_dev_ctl))
					continue;
				ptr_dev_ctl = it->second;
			} while (false);

			return ptr_dev_ctl;
		}

		/**
		* m_mutex_device_map is used!
		*/
		cworker_ctl::type_ptr cctl_svr::_get_dev_ctl(const std::wstring& s_device_path)
		{
			cworker_ctl::type_ptr ptr_dev_ctl;
			unsigned short w_index(0);

			do {
				if (s_device_path.empty())
					continue;

				std::lock_guard<std::mutex> lock(m_mutex_device_map);
				//
				cctl_svr::_type_map_device_index_to_ptr_dev_ctl::iterator it = std::begin(m_map_device_index_to_ptr_dev_ctl);
				for (; it != std::end(m_map_device_index_to_ptr_dev_ctl); ++it) {
					if (it->second) {
						if (s_device_path.compare(it->second->get_dev_path()) == 0) {
							w_index = it->first;
							ptr_dev_ctl = it->second;
							break;//found
						}
					}
				}//end for

			} while (false);

			return ptr_dev_ctl;
		}

		/**
		* m_mutex_device_map is used in _get_dev_ctl()
		*/
		std::wstring cctl_svr::_get_device_path_from_device_index(unsigned short w_device_index)
		{
			std::wstring s_device_path;
			do {
				cworker_ctl::type_ptr ptr_dev_ctl = _get_dev_ctl(w_device_index);
				if (!ptr_dev_ctl)
					continue;//already removed device data.
				s_device_path = ptr_dev_ctl->get_dev_path();
			} while (false);
			return s_device_path;
		}

		/**
		* m_mutex_device_map is used in _get_dev_ctl()
		*/
		unsigned short cctl_svr::_get_device_index_from_device_path(const std::wstring& s_path)
		{
			unsigned short w = _MP_TOOLS_INVALID_DEVICE_INDEX;

			do {
				cworker_ctl::type_ptr ptr_dev_ctl = _get_dev_ctl(s_path);
				if (!ptr_dev_ctl)
					continue;//already removed device data.
				w = ptr_dev_ctl->get_device_index();

			} while (false);

			return w;
		}

		unsigned long cctl_svr::get_session_number_from_device_index(unsigned short w_device_index)
		{
			unsigned long n_s = _MP_TOOLS_INVALID_SESSION_NUMBER;

			std::lock_guard<std::mutex> lock(m_mutex_map_session_to_device_index);

			for (auto item : m_map_session_to_set_of_device_index) {
				for (auto n : item.second) {
					if (n == w_device_index) {
						return item.first;
					}
				}//end for set
			}//end for map
			return n_s;
		}

}//the end of _mp namespace

