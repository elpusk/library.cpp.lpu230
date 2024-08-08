
#include <websocket/mp_win_nt.h>
#include <server/mp_cclient.h>
#include <mp_clog.h>

namespace _mp {
	bool cclient::send_data_to_server_by_ip4(const type_v_buffer& vData)
	{
		bool b_result(false);

		do {
			cclient::type_status status(get_status());
			if (status != cclient::status_connected)
				continue;
			std::lock_guard<std::mutex> lock(m_mutex_client);//for m_ptr_client

			if (!m_ptr_client)
				continue;
			if (vData.empty())
				continue;
			//
			cws_client::csession::type_ptr_session ptr_session = m_ptr_client->get_session();
			if (!ptr_session)
				continue;
			if (!ptr_session->do_write(vData))
				continue;
			b_result = true;
		} while (false);

		return b_result;
	}

	cclient::cclient()
		: m_status((int)cclient::status_not_connected)
		, m_b_ssl(true)
		, m_w_port(_ws_tools::WEBSOCKET_SECURITY_SERVER_PORT_COFFEE_MANAGER )
		, m_n_session_from_server(_MP_TOOLS_INVALID_SESSION_NUMBER)
		, m_b_need_set_session_from_server(true)
		, m_ptr_cb(std::make_shared<cclient_cb>())
	{
	}

	void cclient::_push_q_user_callback_resolve(unsigned long n_result)
	{
		m_ptr_cb->push_back_resolve(n_result);
	}

	void cclient::_push_q_user_callback_connect(unsigned long n_result)
	{
		m_ptr_cb->push_back_connect(n_result);
	}

	void cclient::_push_q_user_callback_handshake(unsigned long n_result)
	{
		m_ptr_cb->push_back_handshake(n_result);
	}

	void cclient::_push_q_user_callback_ssl_handshake(unsigned long n_result)
	{
		m_ptr_cb->push_back_handshake_ssl(n_result);
	}

	void cclient::_push_q_user_callback_read(cio_packet::type_act act_code,unsigned long n_result, unsigned long n_device_index, unsigned c_in_id, const type_v_buffer& v_rx)
	{
		m_ptr_cb->push_back_read(act_code, n_result, n_device_index, c_in_id, v_rx);
	}

	void cclient::_push_q_user_callback_write(unsigned long n_result)
	{
		m_ptr_cb->push_back_write(n_result);
	}

	void cclient::_push_q_user_callback_close(unsigned long n_result)
	{
		m_ptr_cb->push_back_close(n_result);
	}

	bool cclient::start()
	{
		bool b_result(false);
		//
		do {
			cclient::type_status status(get_status());
			if (status == cclient::status_connected)
				continue;
			if (status == cclient::status_connecting)
				continue;

			std::lock_guard<std::mutex> lock(m_mutex_client);//for m_ptr_client

			if (m_ptr_client){
				//may be previous connection failed.
				_stop();//remove  previous client.
			}

			cws_client::ccallback cb;
			cb.set_resolve(cclient::_callback_resolve, this)
				.set_connect(cclient::_callback_connect, this)
				.set_handshake(cclient::_callback_handshake, this)
				.set_ssl_handshake(cclient::_callback_ssl_handshake, this)
				.set_read(cclient::_callback_read, this)
				.set_write(cclient::_callback_write, this)
				.set_close(cclient::_callback_close, this);

			unsigned short w_port(m_w_port);

			std::string s_root_cert_file("");

			if (!m_s_root_cert_file.empty()) {
				s_root_cert_file = cstring::get_mcsc_from_unicode(m_s_root_cert_file);
			}

			if(m_b_ssl)
				m_ptr_client = std::make_shared<cws_client>(std::string("127.0.0.1"), w_port, s_root_cert_file);//load root-ca from system ca-storage
			else
				m_ptr_client = std::make_shared<cws_client>(std::string("127.0.0.1"), w_port);
			//
			if (!m_ptr_client)
				continue;
			if (!m_ptr_client->is_ini()) {
				m_ptr_client.reset();
				continue;
			}
			if (!m_ptr_client->set_callback(cb).start()) {
				m_ptr_client.reset();
				continue;
			}

			m_thread_client = std::thread(cclient::_worker, m_ptr_client.get());
			set_status(cclient::status_connecting);
			b_result = true;
		} while (false);
		return b_result;
	}

	void cclient::_stop()
	{
		do {
			if (!m_ptr_client)
				continue;
			if (!m_ptr_client->stopped()) {
				m_ptr_client->stop();

				while (!m_ptr_client->stopped()) { 
					std::this_thread::sleep_for(std::chrono::milliseconds(20));
				}
			}
			if (m_thread_client.joinable())
				m_thread_client.join();
			m_ptr_client.reset();
		} while (false);

	}
	void cclient::stop()
	{
		std::lock_guard<std::mutex> lock(m_mutex_client);//for m_ptr_client
		_stop();
		set_status(cclient::status_not_connected);
	}

	cclient::~cclient()
	{
		stop();
		m_ptr_cb.reset();
	}

	void cclient::_callback_resolve(cws_client::csession& session, boost::beast::error_code& ec, boost::asio::ip::tcp::resolver::results_type&, void* p_parameter)
	{
		do {
			cclient* p_obj = (cclient*)p_parameter;
			if (p_obj == nullptr)
				continue;

			unsigned long n_result(cclient::RESULT_ERROR);

			if (ec) {
				clog::get_instance().trace(L" :: E : _callback_resolve\n");
				p_obj->set_status(cclient::status_not_connected);
			}
			else {
				n_result = cclient::RESULT_SUCCESS;
				clog::get_instance().trace(L" :: S : _callback_resolve\n");
			}
			p_obj->_push_q_user_callback_resolve(n_result);

		} while (false);
	}
	void cclient::_callback_connect(cws_client::csession& session, boost::beast::error_code& ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type&, void* p_parameter)
	{
		do {
			cclient* p_obj = (cclient*)p_parameter;
			if (p_obj == nullptr)
				continue;
			unsigned long n_result(cclient::RESULT_ERROR);
			if (ec) {
				clog::get_instance().trace(L" :: E : _callback_connect\n");
				p_obj->set_status(cclient::status_not_connected);
			}
			else {
				n_result = cclient::RESULT_SUCCESS;
				clog::get_instance().trace(L" :: S : _callback_connect\n");
			}
			p_obj->_push_q_user_callback_connect(n_result);
		} while (false);
	}
	void cclient::_callback_handshake(cws_client::csession& session, boost::beast::error_code& ec, void* p_parameter)
	{
		do {
			cclient* p_obj = (cclient*)p_parameter;
			if (p_obj == nullptr)
				continue;
			unsigned long n_result(cclient::RESULT_ERROR);
			if (ec) {
				clog::get_instance().trace(L" :: E : _callback_handshake\n");
				p_obj->set_status(cclient::status_not_connected);
				p_obj->_push_q_user_callback_handshake(n_result);
			}
			else {
				n_result = cclient::RESULT_SUCCESS;
				clog::get_instance().trace(L" :: S : _callback_handshake\n");
				p_obj->set_status(cclient::status_connected);
				p_obj->_set_connect(true);
				// don't call callback_handshake.
				// when handshake is succcessed, callback_handshake is call in _callback_read.
			}
		
		
		} while (false);
	}
	void cclient::_callback_ssl_handshake(cws_client::csession& session, boost::beast::error_code& ec, void* p_parameter)
	{
		do {
			cclient* p_obj = (cclient*)p_parameter;
			if (p_obj == nullptr)
				continue;
			unsigned long n_result(cclient::RESULT_ERROR);
			if (ec) {
				clog::get_instance().trace(L" :: E : _callback_ssl_handshake\n");
				p_obj->set_status(cclient::status_not_connected);
			}
			else {
				n_result = cclient::RESULT_SUCCESS;
				clog::get_instance().trace(L" :: S : _callback_ssl_handshake\n");
			}
			p_obj->_push_q_user_callback_ssl_handshake(n_result);
		} while (false);
	}
	void cclient::_callback_read(cws_client::csession& session, boost::beast::error_code& ec, const type_v_buffer& v_data, void* p_parameter)
	{
		//v_data is json format
		do {
			cclient* p_obj = (cclient*)p_parameter;
			if (p_obj == nullptr)
				continue;
			cio_packet::type_act act_code(cio_packet::act_mgmt_unknown);
			unsigned long n_result(cclient::RESULT_ERROR);
			type_v_buffer v_data_field(0);
			bool b_need_callback(true);
			unsigned long n_device_index(0);
			unsigned char c_in_id(0);

			do {
				if (ec) {
					if (ec.value() == boost::system::errc::errc_t::operation_not_permitted) {
						//server may be terminated.
						clog::get_instance().trace(L" :: E : _callback_read : server terminated\n");
					}
					else {
						clog::get_instance().trace(L" :: E : _callback_read\n");
					}
					continue;
				}

				clog::get_instance().trace(L" :: S : _callback_read.\n");

				std::string s_json_error;
				cio_packet::type_ptr ptr_packet = cio_packet::build_from_json(v_data, s_json_error);
				if (!ptr_packet)
					continue;
			
				if (p_obj->_is_need_set_session_from_server()) {
					//the first receving data from server ....... may be session number by my protocol.
					//save session number.
					p_obj->_set_session_number_from_server(ptr_packet->get_session_number());
					p_obj->_clear_flag_for_set_session_from_server();
					b_need_callback = false;
					// this is import code.
					p_obj->_push_q_user_callback_handshake(cclient::RESULT_SUCCESS);//connect complete.
					continue;
				}

				act_code = ptr_packet->get_action();
				unsigned long n_session_from_server(p_obj->get_session_number_from_server());
				if (ptr_packet->get_session_number() != n_session_from_server)
					continue;

				if (ptr_packet->is_owner_manager()) {
					//request for manager
					n_result = cclient::RESULT_SUCCESS;
					ptr_packet->get_data_field(v_data_field);
					continue;
				}

				n_device_index = ptr_packet->get_device_index();
				c_in_id = ptr_packet->get_in_id();

				if(ptr_packet->get_data_field_type() == cio_packet::data_field_binary){
					//success case
					n_result = cclient::RESULT_SUCCESS;
					ptr_packet->get_data_field(v_data_field);
					continue;
				}
				if (ptr_packet->is_success()) {
					//success case
					n_result = cclient::RESULT_SUCCESS;
					continue;
				}
			
				if (ptr_packet->is_error()) {
					n_result = cclient::RESULT_ERROR;
					continue;
				}

				if (ptr_packet->is_cancel()) {
					n_result = cclient::RESULT_CANCEL;
					continue;
				}

				// server & client protocol error
				b_need_callback = false;
				clog::get_instance().trace(L" :: E : _callback_read : server & client protocol error.\n");
			} while (false);

			if( b_need_callback )
				p_obj->_push_q_user_callback_read(act_code,n_result, n_device_index, c_in_id, v_data_field);
		} while (false);
	}
	void cclient::_callback_write(cws_client::csession& session, boost::beast::error_code& ec, const type_v_buffer& v_data, void* p_parameter)
	{
		//v_data is json format
		do {
			cclient* p_obj = (cclient*)p_parameter;
			if (p_obj == nullptr)
				continue;
			unsigned long n_device_index(0);
			unsigned char c_in_id(0);
			unsigned long n_result(cclient::RESULT_ERROR);
			if (ec) {
				clog::get_instance().trace(L" :: E : _callback_write\n");
			}
			else {
				clog::get_instance().trace(L" :: S : _callback_write.\n");
				n_result = cclient::RESULT_SUCCESS;

			}
			p_obj->_push_q_user_callback_write(n_result);
		} while (false);
	}
	void cclient::_callback_close(cws_client::csession& session, boost::beast::error_code& ec, void* p_parameter)
	{
		do {
			cclient* p_obj = (cclient*)p_parameter;
			if (p_obj == nullptr)
				continue;
			unsigned long n_result(cclient::RESULT_ERROR);
			if (ec) {
				clog::get_instance().trace(L" :: E : _callback_close\n");
			}
			else {
				n_result = cclient::RESULT_SUCCESS;
				clog::get_instance().trace(L" :: S : _callback_close\n");
			}
			p_obj->set_status(cclient::status_not_connected);
			p_obj->_set_connect(false);

			p_obj->_push_q_user_callback_close(n_result);
		} while (false);
	}
}//the end of _mp namespace