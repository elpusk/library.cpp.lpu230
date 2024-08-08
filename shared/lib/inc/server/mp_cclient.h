#pragma once

#include <memory>
#include <mutex>


#include <websocket/mp_cws_client.h>
#include <mp_clog.h>

#include <server/mp_cio_packet.h>
#include <server/mp_cclient_cb.h>

namespace _mp {
	class cclient
	{
	public:
		typedef	std::shared_ptr< cclient >		type_ptr;
		typedef	std::unique_ptr< cclient >		type_uptr;

		typedef	enum : int{
			status_not_connected,
			status_connecting,
			status_connected
		}type_status;

		enum : unsigned long {
			RESULT_SUCCESS = 0x00000000,
			RESULT_ERROR	= 0x80000000,
			RESULT_CANCEL = 0x40000000
		};

		enum : unsigned long {
			UNDEFINED_INDEX = 0x00000000 ////only for coffee_hlp_client_create_before_set_callback()
		};

		// for ctl_show() ' n_show
		enum : unsigned long {
			CTL_SHOW_SHOW = 0x00000001,
			CTL_SHOW_HIDE = 0x00000000
		};

	public:
		static _mp::clog* get_log(_mp::clog* p_log = nullptr)
		{
			static _mp::clog* _p_log(nullptr);
			if (p_log)
				_p_log = p_log;
			return _p_log;
		}

		~cclient();
		cclient();

		cclient::type_status get_status() const
		{
			cclient::type_status st((cclient::type_status)m_status.load());
			return st;
		}
		void set_status(cclient::type_status st)
		{
			m_status.store((int)st);
		}

		bool start();
		void stop();

		bool is_ssl() const
		{
			return m_b_ssl;
		}

		cclient_cb::type_cb_callbacks get_callbacks()
		{
			return m_ptr_cb->get_callbacks();
		}
		cclient_cb::type_cb_resolve get_callback_resolve()
		{
			return m_ptr_cb->get_callback_resolve();
		}
		cclient_cb::type_cb_connect get_callback_connect()
		{
			return m_ptr_cb->get_callback_connect();
		}
		cclient_cb::type_cb_handshake get_callback_handshake()
		{
			return m_ptr_cb->get_callback_handshake();
		}
		cclient_cb::type_cb_handshake_ssl get_callback_ssl_handshake()
		{
			return m_ptr_cb->get_callback_ssl_handshake();
		}
		cclient_cb::type_cb_read get_callback_read()
		{
			return m_ptr_cb->get_callback_read();
		}
		cclient_cb::type_cb_write get_callback_write()
		{
			return m_ptr_cb->get_callback_write();
		}
		cclient_cb::type_cb_close get_callback_close()
		{
			return m_ptr_cb->get_callback_close();
		}

		cclient& set_ssl(bool b_ssl)
		{
			m_b_ssl = b_ssl;
			return *this;
		}

		cclient& set_port(unsigned short w_port)
		{
			m_w_port = w_port;
			return *this;
		}
		cclient& set_root_cert_file(const std::wstring & s_root_cert_file)
		{
			m_s_root_cert_file = s_root_cert_file;
			return *this;
		}
		cclient& set_callbacks(const cclient_cb::type_cb_callbacks& cbs)
		{
			m_ptr_cb->set_callbacks(cbs);
			return *this;
		}
		cclient& set_callback_resolve(const cclient_cb::type_cb_resolve cb, void* p_user)
		{
			m_ptr_cb->set_callback_resolve(cb, p_user);
			return *this;
		}
		cclient& set_callback_connect(const cclient_cb::type_cb_connect cb, void* p_user)
		{
			m_ptr_cb->set_callback_connect(cb, p_user);
			return *this;
		}
		cclient& set_callback_handshake(const cclient_cb::type_cb_handshake cb, void* p_user)
		{
			m_ptr_cb->set_callback_handshake(cb, p_user);
			return *this;
		}
		cclient& set_callback_handshake_ssl(const cclient_cb::type_cb_handshake_ssl cb, void* p_user)
		{
			m_ptr_cb->set_callback_handshake_ssl(cb, p_user);
			return *this;
		}
		cclient& set_callback_read(const cclient_cb::type_cb_read cb, void* p_user)
		{
			m_ptr_cb->set_callback_read(cb, p_user);
			return *this;
		}
		cclient& set_callback_write(const cclient_cb::type_cb_write cb, void* p_user)
		{
			m_ptr_cb->set_callback_write(cb, p_user);
			return *this;
		}
		cclient& set_callback_close(const cclient_cb::type_cb_close cb, void* p_user)
		{
			m_ptr_cb->set_callback_close(cb, p_user);
			return *this;
		}

		unsigned long get_session_number_from_server() const
		{
			return m_n_session_from_server;
		}

		bool send_data_to_server_by_ip4(const type_v_buffer& vData);

	private:
		void _stop();
	
		void _push_q_user_callback_resolve(unsigned long n_result);
		void _push_q_user_callback_connect(unsigned long n_result);
		void _push_q_user_callback_handshake(unsigned long n_result);
		void _push_q_user_callback_ssl_handshake(unsigned long n_result);
		void _push_q_user_callback_read(cio_packet::type_act act_code,unsigned long n_result, unsigned long n_device_index, unsigned c_in_id, const type_v_buffer& v_rx);
		void _push_q_user_callback_write(unsigned long n_result);
		void _push_q_user_callback_close(unsigned long n_result);

		void _set_connect(bool b_connect)
		{
			if (b_connect) {
				m_b_need_set_session_from_server = true;
			}
			else {
				m_b_need_set_session_from_server = true;
			}
		}
		bool _is_need_set_session_from_server() const
		{
			return m_b_need_set_session_from_server;
		}
		void _clear_flag_for_set_session_from_server()
		{
			m_b_need_set_session_from_server = false;
		}
		void _set_session_number_from_server(unsigned long n_session_from_server)
		{
			m_n_session_from_server = n_session_from_server;
		}

		static void _callback_resolve(_mp::cws_client::csession& session, boost::beast::error_code& ec, boost::asio::ip::tcp::resolver::results_type&, void* p_parameter);
		static void _callback_connect(_mp::cws_client::csession& session, boost::beast::error_code& ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type&, void* p_parameter);
		static void _callback_handshake(_mp::cws_client::csession& session, boost::beast::error_code& ec, void* p_parameter);
		static void _callback_ssl_handshake(_mp::cws_client::csession& session, boost::beast::error_code& ec, void* p_parameter);
		static void _callback_read(_mp::cws_client::csession& session, boost::beast::error_code& ec, const type_v_buffer& v_data, void* p_parameter);
		static void _callback_write(_mp::cws_client::csession& session, boost::beast::error_code& ec, const type_v_buffer& v_data, void* p_parameter);
		static void _callback_close(_mp::cws_client::csession& session, boost::beast::error_code& ec, void* p_parameter);

		static void _worker(_mp::cws_client* p_client)
		{
			do {
				if (p_client == nullptr)
					continue;
				p_client->run();
			} while (false);
		}
	private:
		std::atomic_int m_status;
		bool m_b_ssl;
		unsigned short m_w_port;
		std::wstring m_s_root_cert_file;

		std::mutex m_mutex_client;
		_mp::cws_client::type_ptr m_ptr_client;
		std::thread m_thread_client;

		unsigned long m_n_session_from_server;
		bool m_b_need_set_session_from_server;

		cclient_cb::type_ptr m_ptr_cb;

	private:
		//don't call these methods
		cclient(const cclient&) = delete;
		cclient & operator= (const cclient&) = delete;

	};

}//the end of _mp namespace
