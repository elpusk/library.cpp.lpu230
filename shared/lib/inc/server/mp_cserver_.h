#pragma once
#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>

#include <mp_clog.h>
#include <websocket/mp_cws_server.h>

namespace _mp{

	/**
	 * send data to client.
	 * receive data from client.
	 * start/stop server
	 */
	class cserver
	{
	public:
		enum {
			const_default_worker_sleep_interval_mmsec = 3,
		};
	public:
		static cserver& get_instance(
			long long ll_worker_sleep_interval_mmsec = cserver::const_default_worker_sleep_interval_mmsec
			,clog* p_log = nullptr
		);

		virtual ~cserver();
	public:
		bool start(int n_thread_for_server, const std::wstring& s_root_folder_except_backslash);
		void stop();

		bool is_ssl();

		cserver& set_cert_file(const std::wstring& s_server_cert_file);
		cserver& set_private_key_file(const std::wstring& s_server_private_key_file);
		cserver& set_ssl(bool b_ssl);
		cserver& set_callback(const _mp::cws_server::ccallback& callack);

		cserver& set_port(unsigned short w_port);

		cserver& set_timeout_websocket_upgrade_req(long long ll_timeout);
		cserver& set_timeout_idle(long long ll_timeout);
		cserver& set_timeout_ssl_handshake_complete(long long ll_timeout);

		bool send_data_to_client_by_ip4_device_plug_out(unsigned long n_session, unsigned short w_device_index, const std::wstring& s_device_path);

		bool broadcast_by_ip4_device_plug_in(const std::wstring& s_device_path);

		bool send_data_to_client_by_ip4(const _mp::type_v_buffer& v_data, unsigned long n_session);

		bool broadcast_by_ip4(unsigned long n_owner_session, const _mp::type_v_buffer& v_data);

		_mp::cws_server::csession::type_ptr_session get_session(unsigned long n_session);
	private:
		static void _server_worker(const _mp::cws_server::type_ptr ptr_server);


		//callback for server events.
		static void _cb_svr_handshake(unsigned long n_session, boost::beast::error_code& ec, void* p_parameter);

		static void _cb_svr_accept(unsigned long n_session, boost::beast::error_code& ec, void* p_parameter);

		static void _cb_svr_close(unsigned long n_session, boost::beast::error_code& ec, void* p_parameter);

		static void _cb_svr_read(unsigned long n_session, boost::beast::error_code& ec, const _mp::type_v_buffer& v_rx, void* p_parameter);
		static void _cb_svr_write(unsigned long n_session, boost::beast::error_code& ec, const _mp::type_v_buffer& v_data, void* p_parameter);

	private:
		cserver(long long ll_worker_sleep_interval_mmsec);

		//display code for using easy.
		void _dp_n(const std::wstring& s_info);
		void _dp_w(const std::wstring& s_info);
		void _dp_e(const std::wstring& s_info);
		void _dp_n(const _mp::type_v_buffer& v_data);
		void _dp_w(const _mp::type_v_buffer& v_data);
		void _dp_e(const _mp::type_v_buffer& v_data);
	private:
		long long m_ll_worker_sleep_interval_mmsec;
		clog* m_p_log;
		//for server
		bool m_b_ssl;

		std::wstring m_s_server_cert_file;
		std::wstring m_s_server_private_key_file;

		unsigned short m_w_port;
		_mp::cws_server::ccallback m_callack;

		_mp::cws_server::type_ptr m_ptr_server_for_ip4;

		std::vector<std::thread> m_v_thread_server_ip4;

		/////////////////////////////////////////////////////////////////////////////////////
		//timeout for websocket server
		// 
		//Timeout after the server receives the ssl handshake request and calls async_accept(), and the client responds with a WebSocket Upgrade request.
		long long m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client;

		//The maximum time that a WebSocket connection can last without sending or receiving any data
		long long m_ll_msec_timeout_ws_server_wait_for_idle;

		//SSL handshake complete timeout
		long long m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete;

	private:
		cserver(const cserver&) = delete;
		cserver& operator=(const cserver&) = delete;
	};
}