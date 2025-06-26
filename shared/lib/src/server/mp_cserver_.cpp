
#include <websocket/mp_win_nt.h>

#include <memory>

#include <mp_coffee_path.h>
#include <mp_cio_packet.h>
#include <server/mp_cserver_.h>
#include <server/mp_cctl_svr_.h>

#include <hid/mp_clibhid.h>

namespace _mp{

	/**
	 * send data to client.
	 * receive data from client.
	 * start/stop server
	 */
		cserver& cserver::get_instance(clog* p_log /*= nullptr*/)
		{
			static bool b_first = true;
			static std::shared_ptr<cserver> ptr_obj(new cserver());

			if (b_first) {
				b_first = false;
				ptr_obj->m_p_log = p_log;
				cctl_svr::get_instance().create_main_ctl_and_set_callack(p_log);
			}

			return *ptr_obj;
		}

		cserver::~cserver()
		{
			stop();
		}
		bool cserver::start(int n_thread_for_server, const std::wstring &s_root_folder_except_backslash)
		{
			bool b_result(false);
			//static boost::asio::ip::address ip4_address = _ws_tools::get_local_ip6();
			static boost::asio::ip::address ip4_address = boost::asio::ip::address::from_string("127.0.0.1");

			do {
				if (s_root_folder_except_backslash.empty()) {
					continue;
				}
				if (n_thread_for_server < 1)
					continue;
				if (m_ptr_server_for_ip4)
					continue;
				//create hid device library instance.
				clibhid& lib_hid(_mp::clibhid::get_instance());
				if (lib_hid.is_ini()) {
					clibhid_dev_info::type_set st_dev = lib_hid.get_cur_device_set();
					unsigned short w_device_index = 0;

					for (const _mp::clibhid_dev_info& item : st_dev) {
						cctl_svr::get_instance().create_new_dev_ctl(m_p_log, item);
					}//end for
				}
				
				// create server
				if (m_b_ssl) {
					std::string s_cert_file;
					std::string s_key_file;

					if (!m_s_server_cert_file.empty())
						s_cert_file = _mp::cstring::get_mcsc_from_unicode(m_s_server_cert_file);
					if (!m_s_server_private_key_file.empty())
						s_key_file = _mp::cstring::get_mcsc_from_unicode(m_s_server_private_key_file);
					//
					m_ptr_server_for_ip4 = std::make_shared<_mp::cws_server>
						(
							ip4_address, m_w_port, n_thread_for_server, s_cert_file, s_key_file, s_root_folder_except_backslash,
							m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client,
							m_ll_msec_timeout_ws_server_wait_for_idle,
							m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete
						);
				}
				else
					m_ptr_server_for_ip4 = std::make_shared<_mp::cws_server>
					(
						ip4_address, m_w_port, n_thread_for_server, s_root_folder_except_backslash,
						m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client,
						m_ll_msec_timeout_ws_server_wait_for_idle,
						m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete
					);
				//
				if (!m_ptr_server_for_ip4->is_ini()) {
					m_p_log->log_fmt(L"[E] - %ls - create server : fail.\n", __WFUNCTION__);
					m_p_log->trace(L"[E] - %ls - create server : fail.\n", __WFUNCTION__);
					m_ptr_server_for_ip4.reset();
					continue;
				}

				m_ptr_server_for_ip4->set_callback(m_callack).start();

				// Run the I/O service on the requested number of threads
				m_v_thread_server_ip4.reserve(n_thread_for_server);
				for (int i = 0; i < n_thread_for_server; i++) {
					m_v_thread_server_ip4.emplace_back(std::thread(cserver::_server_worker, m_ptr_server_for_ip4));
				}

				std::string s_ip = ip4_address.to_string();
				std::wstring sw_ip;
				std::transform(std::begin(s_ip), std::end(s_ip), std::back_inserter(sw_ip), [](char c)->wchar_t {
					return (wchar_t)c;
					});
				m_p_log->log_fmt(L"[I] - %ls - started server : %ls.\n", __WFUNCTION__, sw_ip.c_str());
				m_p_log->trace(L"[I] - %ls - started server : %ls.\n", __WFUNCTION__, sw_ip.c_str());

				b_result = true;
			} while (false);
			
			return b_result;
		}
		void cserver::stop()
		{
			do {
				if (!m_ptr_server_for_ip4)
					continue;
				if (!m_ptr_server_for_ip4->stopped()) {
					m_ptr_server_for_ip4->stop();
					while (!m_ptr_server_for_ip4->stopped()) {
#ifdef _WIN32
						Sleep(20);
#else
						usleep(20 * 1000);
#endif
					}
				}

				for (auto& th : m_v_thread_server_ip4) {
					th.join();
				}

				m_ptr_server_for_ip4.reset();
			} while (false);
		}

		bool cserver::is_ssl()
		{
			return m_b_ssl;
		}

		cserver& cserver::set_cert_file(const std::wstring& s_server_cert_file)
		{
			m_s_server_cert_file = s_server_cert_file;
			return *this;
		}
		cserver& cserver::set_private_key_file(const std::wstring& s_server_private_key_file)
		{
			m_s_server_private_key_file = s_server_private_key_file;
			return *this;
		}
		cserver& cserver::set_ssl(bool b_ssl)
		{
			m_b_ssl = b_ssl;
			return *this;
		}
		cserver& cserver::set_callback(const _mp::cws_server::ccallback& callack)
		{
			m_callack = callack;
			return *this;
		}

		cserver& cserver::set_port(unsigned short w_port)
		{
			m_w_port = w_port;
			return *this;
		}

		cserver& cserver::set_timeout_websocket_upgrade_req(long long ll_timeout)
		{
			m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client = ll_timeout;
			return *this;
		}
		cserver& cserver::set_timeout_idle(long long ll_timeout)
		{
			m_ll_msec_timeout_ws_server_wait_for_idle = ll_timeout;
			return *this;
		}
		cserver& cserver::set_timeout_ssl_handshake_complete(long long ll_timeout)
		{
			m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete = ll_timeout;
			return *this;
		}

		bool cserver::send_data_to_client_by_ip4_device_plug_out(unsigned long n_session, unsigned short w_device_index, const std::wstring& s_device_path)
		{
			bool b_result(false);

			do {
				if (n_session == _MP_TOOLS_INVALID_SESSION_NUMBER)
					continue;
				//send system event message for removed device.
				cio_packet::type_ptr ptr_rsp = cio_packet::build(
					cio_packet::cmd_system_request
					, n_session
					, cio_packet::owner_device
					, w_device_index
					, cio_packet::act_dev_close
					, 0
					, 0
					, cio_packet::data_field_string_utf8
					, 0
					, nullptr
				);

				ptr_rsp->set_data_by_utf8(s_device_path);

				_mp::type_v_buffer v_rsp;
				if (ptr_rsp->get_packet_by_json_format(v_rsp) == 0)
					continue;
				send_data_to_client_by_ip4(v_rsp, n_session);
				b_result = true;
			} while (false);

			return b_result;
		}

		bool cserver::broadcast_by_ip4_device_plug_in(const std::wstring& s_device_path)
		{
			bool b_result(false);

			do {
				if (s_device_path.empty())
					continue;

				//notificate plugged in device.
				//session number will be changed in server by each connected session number.
				cio_packet::type_ptr ptr_rsp = cio_packet::build(
					cio_packet::cmd_system_request
					, _MP_TOOLS_INVALID_SESSION_NUMBER
					, cio_packet::owner_manager
					, 0
					, cio_packet::act_mgmt_dev_plug_in
					, 0
					, 0
					, cio_packet::data_field_string_utf8
					, 0
					, nullptr
				);

				ptr_rsp->set_data_by_utf8(s_device_path);

				_mp::type_v_buffer v_rsp;
				if (ptr_rsp->get_packet_by_json_format(v_rsp) == 0)
					continue;

				b_result = broadcast_by_ip4(_MP_TOOLS_INVALID_SESSION_NUMBER, v_rsp);//send all session.
			} while (false);
			return b_result;
		}

		bool cserver::send_data_to_client_by_ip4(const _mp::type_v_buffer& v_data, unsigned long n_session)
		{
			bool b_result(false);

			do {
				if (v_data.empty()) {
					_dp_e(L"send_data_to_client_by_ip4 : no data");
					continue;
				}
				if (!m_ptr_server_for_ip4) {
					_dp_e(L"send_data_to_client_by_ip4 : no server");
					continue;
				}

				_mp::cws_server::csession::type_ptr_session ptr_session(m_ptr_server_for_ip4->get_session(n_session));
				if (!ptr_session) {
					_dp_e(L"send_data_to_client_by_ip4 : no session");
					continue;
				}

				b_result = ptr_session->do_send(std::make_shared<_mp::type_v_buffer>(v_data));
			} while (false);
			return b_result;
		}

		bool cserver::broadcast_by_ip4(unsigned long n_owner_session, const _mp::type_v_buffer& v_data)
		{
			bool b_result(false);

			do {
				if (v_data.empty()) {
					_dp_e(L"broadcast_by_ip4 : no data");
					continue;
				}
				if (!m_ptr_server_for_ip4) {
					_dp_e(L"broadcast_by_ip4 : no server");
					continue;
				}

				m_ptr_server_for_ip4->broadcast(n_owner_session, v_data);
				b_result = true;
			} while (false);
			return b_result;
		}

		_mp::cws_server::csession::type_ptr_session cserver::get_session(unsigned long n_session)
		{
			_mp::cws_server::csession::type_ptr_session ptr_session;
			if (m_ptr_server_for_ip4) {
				ptr_session = m_ptr_server_for_ip4->get_session(n_session);
			}
			return ptr_session;
		}
	
		void cserver::_server_worker(const _mp::cws_server::type_ptr ptr_server)//thread for websocket server.
		{
			do {
				if (!ptr_server)
					continue;
				ptr_server->run();
				OPENSSL_thread_stop();
			} while (false);
		}


		//callback for server events.
		void cserver::_cb_svr_handshake(unsigned long n_session, boost::beast::error_code& ec, void* p_parameter)
		{
			std::wstring s_info;

			do {
				cserver* p_obj = (cserver*)p_parameter;

				if (ec) {
					//std::wstring s_error = cserver::_system_category_message_win32_by_english(ec.value());//ec.message();
					std::wstring ws_error = _mp::cstring::get_unicode_english_error_message(ec);
					p_obj->_dp_e(L"_cb_svr_handshake : error : " + ws_error);
					//m_p_log->log_fmt(L"[E] - %ls : %ls.\n", __WFUNCTION__, ws_error.c_str());
					//m_p_log->trace(L"[E] - %ls : %ls.\n", __WFUNCTION__, ws_error.c_str());
					continue;
				}
				
				s_info = L"_cb_svr_handshake : session = " + std::to_wstring(n_session);
				p_obj->_dp_n(s_info);
			} while (false);
		}

		void cserver::_cb_svr_accept(unsigned long n_session, boost::beast::error_code& ec, void* p_parameter)
		{
			std::wstring s_info;
			std::wstring s_error_reason;
			do {
				cserver* p_obj = (cserver*)p_parameter;
				if (ec) {
					std::wstring s_error = _mp::cstring::get_unicode_english_error_message(ec);//ec.message();
					p_obj->_dp_e(L"_cb_svr_accept : error : " + s_error);
					//m_p_log->log_fmt(L"[E] - %ls : %ls.\n", __WFUNCTION__, s_error.c_str());
					//m_p_log->trace(L"[E] - %ls : %ls.\n", __WFUNCTION__, s_error.c_str());
					continue;
				}

				p_obj->get_session(n_session)->set_virtual_full_path_of_temp_file(ccoffee_path::get_virtual_path_of_temp_rom_file_of_session(n_session));
				s_info = L"_cb_svr_accept : accepted : session = " + std::to_wstring(n_session);
				p_obj->_dp_n(s_info);

				//send session number
				cio_packet::type_ptr ptr_request_echo = cio_packet::build(
					cio_packet::cmd_request
					, n_session
					, cio_packet::owner_manager
					, 0
					, cio_packet::act_mgmt_get_echo
					, 0
					, 0
					, cio_packet::data_field_binary
					, 0
					, nullptr
				);

				cctl_svr::get_instance().create_kernel_ctl(p_obj->m_p_log, n_session);

				if (!cctl_svr::get_instance().push_request_to_worker_ctl(ptr_request_echo, s_error_reason)) {//automatic cancel device
					p_obj->m_p_log->log_fmt(L"[E] - %ls | _push_request_of_client() of echo[for sending session number].\n", __WFUNCTION__);
					p_obj->m_p_log->trace(L"[E] - %ls | _push_request_of_client() of echo[for sending session number].\n", __WFUNCTION__);
				}

			} while (false);
		}

		void cserver::_cb_svr_close(unsigned long n_session, boost::beast::error_code& ec, void* p_parameter)
		{
			do {
				cserver* p_obj = (cserver*)p_parameter;

				if (ec) {//this case this connection is closed by accident error.
					std::wstring s_info = L"_cb_svr_close : error(" + _mp::cstring::get_unicode_english_error_message(ec) + L") : session = " + std::to_wstring(n_session);
					p_obj->_dp_e(s_info);
					p_obj->m_p_log->log_fmt(L"[E] - %ls(%ls) | session = %u.\n", __WFUNCTION__, _mp::cstring::get_unicode_english_error_message(ec).c_str(), n_session);
					p_obj->m_p_log->trace(L"[E] - %ls(%ls) | session = %u.\n", __WFUNCTION__, _mp::cstring::get_unicode_english_error_message(ec).c_str(), n_session);
				}
				else {//this case this connection is closed by user request,
					std::wstring s_info = L"_cb_svr_close : success : session = " + std::to_wstring(n_session);
					p_obj->_dp_n(s_info);
				}

				cctl_svr::get_instance().close_all_device_of_session(n_session);
				cctl_svr::get_instance().remove_session_name(n_session);
				cctl_svr::get_instance().remove_kernel_ctl(n_session);
			} while (false);
		}

		void cserver::_cb_svr_read(unsigned long n_session, boost::beast::error_code& ec, const _mp::type_v_buffer& v_rx, void* p_parameter)
		{
			//don't call cws_server method. 
			// if call these, deadlock is generated.
			cio_packet::type_ptr ptr_error_response(nullptr);
			cserver* p_obj = nullptr;
			std::wstring s_info;
			std::wstring s_error_reason;

			do {
				p_obj = (cserver*)p_parameter;
				//
				if (ec)
					continue;
				//
				s_info = L"_cb_svr_read : success : session = " + std::to_wstring(n_session);
				s_info += L" : size = ";
				s_info += std::to_wstring(v_rx.size());
				p_obj->_dp_n(s_info);

				std::string s_json_error;
				cio_packet::type_ptr ptr_request(cio_packet::build_from_json(v_rx, s_json_error));
				if (!ptr_request) {
					//this case client request has a missing format.
					ptr_error_response = cio_packet::build_common_error_response(ptr_request, cio_packet::get_error_message(cio_packet::error_reason_json));
					p_obj->m_p_log->log_fmt(L"[E] - %ls | build_from_json().\n", __WFUNCTION__);
					p_obj->m_p_log->trace(L"[E] - %ls | build_from_json().\n", __WFUNCTION__);
					s_info = L"_cb_svr_read : request has a missing format.";
					p_obj->_dp_e(s_info);
					continue;
				}
				if (ptr_request->empty()) {
					ptr_error_response = cio_packet::build_common_error_response(ptr_request, cio_packet::get_error_message(cio_packet::error_reason_json));
					p_obj->m_p_log->log_fmt(L"[E] - %ls | ptr_request->empty().\n", __WFUNCTION__);
					p_obj->m_p_log->trace(L"[E] - %ls | ptr_request->empty().\n", __WFUNCTION__);
					s_info = L"_cb_svr_read : no request";
					p_obj->_dp_e(s_info);
					continue;
				}
				if (ptr_request->get_session_number() != n_session) {
					//invalied session number
					ptr_error_response = cio_packet::build_common_error_response(ptr_request, cio_packet::get_error_message(cio_packet::error_reason_session));
					p_obj->m_p_log->log_fmt(L"[E] - %ls | mismatch session number | request %u , current session %u.\n", __WFUNCTION__, ptr_request->get_session_number(), n_session);
					p_obj->m_p_log->trace(L"[E] - %ls | mismatch session number | request %u , current session %u.\n", __WFUNCTION__, ptr_request->get_session_number(), n_session);
					s_info = L"_cb_svr_read : mismatch session number[req : saved]" + std::to_wstring(ptr_request->get_session_number()) + L" : " + std::to_wstring(n_session);
					p_obj->_dp_e(s_info);
					continue;
				}
				if (!ptr_request->is_request()) {
					ptr_error_response = cio_packet::build_common_error_response(ptr_request, cio_packet::get_error_message(cio_packet::error_reason_request_type));
					p_obj->m_p_log->log_fmt(L"[E] - %ls | !is_request().\n", __WFUNCTION__);
					p_obj->m_p_log->trace(L"[E] - %ls | !is_request().\n", __WFUNCTION__);
					s_info = L"_cb_svr_read : invalied request";
					p_obj->_dp_e(s_info);
					continue;
				}

				if (ptr_request->get_action() == cio_packet::act_dev_open) {
					p_obj->_dp_n(L" -_-");
				}

				_mp::type_v_buffer v_data;
				ptr_request->get_data_field(v_data);

				//display raw message
				s_info.clear();
				std::transform(std::begin(v_rx), std::end(v_rx), std::back_inserter(s_info), [](unsigned char  c)->wchar_t {
					return (wchar_t)c;
					});

				size_t n_maxx_size(512);
				if (s_info.size() > n_maxx_size) {
					//ATLTRACE(L"I :: cserver :: _cb_svr_read :: v_rx.size() > %u bytes.\n", n_maxx_size);
					s_info.resize(n_maxx_size);
					s_info += L".......";
				}

				p_obj->_dp_n(s_info);

				if (!cctl_svr::get_instance().push_request_to_worker_ctl(ptr_request, s_error_reason)) {
					ptr_error_response = cio_packet::build_common_error_response(ptr_request, s_error_reason);

					p_obj->m_p_log->log_fmt(L"[E] - %ls | _push_request_of_client()\n", __WFUNCTION__);
					p_obj->m_p_log->trace(L"[E] - %ls | _push_request_of_client()\n", __WFUNCTION__);
					s_info = L"_cb_svr_read : fail _push_request_of_client()";
					p_obj->_dp_e(s_info);
					continue;
				}
			} while (false);

			if (ptr_error_response && p_obj != nullptr) {
				//push error response
				if (!cctl_svr::get_instance().push_request_to_worker_ctl(ptr_error_response, s_error_reason)) {
					p_obj->m_p_log->log_fmt(L"[E] - %ls | _push_request_of_client()\n", __WFUNCTION__);
					p_obj->m_p_log->trace(L"[E] - %ls | _push_request_of_client()\n", __WFUNCTION__);
					s_info = L"_cb_svr_read : fail error response _push_request_of_client()";
					p_obj->_dp_e(s_info);
				}
			}
		}
		void cserver::_cb_svr_write(unsigned long n_session, boost::beast::error_code& ec, const _mp::type_v_buffer& v_data, void* p_parameter)
		{
			do {
				if (ec)
					continue;
				//
				cserver* p_obj = (cserver*)p_parameter;

				std::wstring s_info = L"_cb_svr_write : success : session = " + std::to_wstring(n_session);
				s_info += L" : size = ";
				s_info += std::to_wstring(v_data.size());
				p_obj->_dp_n(s_info);

				s_info.clear();
				std::transform(std::begin(v_data), std::end(v_data), std::back_inserter(s_info), [](unsigned char  c)->wchar_t {
					return (wchar_t)c;
					});

				size_t n_maxx_size(512);
				if (s_info.size() > n_maxx_size) {
					//ATLTRACE(L"I :: cserver :: _cb_svr_write :: v_data.size() > %u bytes.\n", n_maxx_size);
					s_info.resize(n_maxx_size);
					s_info += L".......";
				}

				p_obj->_dp_n(s_info);

				//save device index for using session to device index.
				std::string s_json_error;
				cio_packet::type_ptr ptr_response(cio_packet::build_from_json(v_data, s_json_error));
				if (!ptr_response) {
					p_obj->m_p_log->log_fmt(L"[E] - %ls | build_from_json().\n", __WFUNCTION__);
					p_obj->m_p_log->trace(L"[E] - %ls | build_from_json().\n", __WFUNCTION__);
					s_info = L"_cb_svr_write : invalied json.";
					p_obj->_dp_e(s_info);
					continue;
				}
				if (ptr_response->get_action() == cio_packet::act_dev_open) {
					if (!ptr_response->is_success()) {
						p_obj->m_p_log->log_fmt(L"[E] - %ls | open !is_success().\n", __WFUNCTION__);
						p_obj->m_p_log->trace(L"[E] - %ls | open !is_success().\n", __WFUNCTION__);
						continue;
					}
					cctl_svr::get_instance().insert_device_index_to_device_index_set_of_session(n_session, (unsigned short)ptr_response->get_device_index());
					continue;
				}
				if (ptr_response->get_action() == cio_packet::act_dev_close) {
					if (!ptr_response->is_success()) {
						p_obj->m_p_log->trace(L"[E] - %ls | close !is_success().\n", __WFUNCTION__);
						continue;
					}
					cctl_svr::get_instance().erase_device_index_from_device_index_set_of_session(n_session, (unsigned short)ptr_response->get_device_index());
					continue;
				}
				if (ptr_response->get_action() == cio_packet::act_mgmt_dev_kernel_operation) {
					if (!ptr_response->is_success()) {
						p_obj->m_p_log->log_fmt(L"[E] - %ls | kernel_operation !is_success().\n", __WFUNCTION__);
						p_obj->m_p_log->trace(L"[E] - %ls | kernel_operation !is_success().\n", __WFUNCTION__);
						continue;
					}
					if (ptr_response->is_kernel_device_open()) {
						cctl_svr::get_instance().insert_device_index_to_device_index_set_of_session(n_session, (unsigned short)ptr_response->get_device_index());
						continue;
					}
					if (ptr_response->is_kernel_device_close()) {
						cctl_svr::get_instance().erase_device_index_from_device_index_set_of_session(n_session, (unsigned short)ptr_response->get_device_index());
						continue;
					}
				}

			} while (false);
		}

		cserver::cserver() : 
			m_w_port(_ws_tools::WEBSOCKET_SERVER_PORT_COFFEE_MANAGER)
			, m_b_ssl(true)
			, m_p_log(nullptr)
			, m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client(CONST_DEFAULT_WS_SERVER_WAIIT_TIMEOUT_FOR_WEBSOCKET_UPGRADE_REQ_OF_CLIENT_MSEC)
			, m_ll_msec_timeout_ws_server_wait_for_idle(CONST_DEFAULT_WS_SERVER_WAIIT_TIMEOUT_FOR_IDLE_MSEC)
			, m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete(CONST_DEFAULT_WS_SERVER_WAIIT_TIMEOUT_FOR_SSL_HANSHAKE_COMPLETE_MSEC)
		{
			//set default callback
			_mp::cws_server::ccallback cb;
			cb.set_handshake(cserver::_cb_svr_handshake, this)
				.set_accept(cserver::_cb_svr_accept, this)
				.set_close(cserver::_cb_svr_close, this)
				.set_read(cserver::_cb_svr_read, this)
				.set_write(cserver::_cb_svr_write, this);
			//
			set_callback(cb);
		}

		//display code for using easy.
		void cserver::_dp_n(const std::wstring& s_info)
		{
			if (!s_info.empty()) {
				clog::get_instance().trace(L"[I] - %ls\n",s_info.c_str());
			}
		}
		void cserver::_dp_w(const std::wstring& s_info)
		{
			if (!s_info.empty()) {
				clog::get_instance().trace(L"[W] - %ls\n", s_info.c_str());
			}
		}
		void cserver::_dp_e(const std::wstring& s_info)
		{
			if (!s_info.empty()) {
				clog::get_instance().trace(L"[E] - %ls\n", s_info.c_str());
			}
		}
		void cserver::_dp_n(const _mp::type_v_buffer& v_data)
		{
			clog::get_instance().trace(L"[I] - ");
			if (!v_data.empty()) {
				clog::get_instance().trace_hex(v_data,L".");
			}
			clog::get_instance().trace(L"\n");
		}
		void cserver::_dp_w(const _mp::type_v_buffer& v_data)
		{
			clog::get_instance().trace(L"[W] - ");
			if (!v_data.empty()) {
				clog::get_instance().trace_hex(v_data, L".");
			}
			clog::get_instance().trace(L"\n");
		}
		void cserver::_dp_e(const _mp::type_v_buffer& v_data)
		{
			clog::get_instance().trace(L"[E] - ");
			if (!v_data.empty()) {
				clog::get_instance().trace_hex(v_data, L".");
			}
			clog::get_instance().trace(L"\n");
		}

}