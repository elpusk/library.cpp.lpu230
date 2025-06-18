#pragma once

#include <mutex>
#include <memory>
#include <functional>

#include <boost/asio/strand.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <websocket/mp_ws_tools.h>
#include <openssl/ssl.h>

#include <mp_clog.h>

#ifdef _WIN32
#include <wincrypt.h>
#endif

namespace _mp
{
	class cws_client
	{
	public:
		typedef	std::shared_ptr< cws_client >	type_ptr;
		typedef	std::weak_ptr< cws_client >		type_wptr;
		typedef	std::unique_ptr< cws_client >	type_uptr;

		class csession;

		typedef std::function<void(cws_client::csession&, boost::beast::error_code&, boost::asio::ip::tcp::resolver::results_type&, void*)>	type_callback_resolve;
		typedef std::function<void(cws_client::csession&, boost::beast::error_code&, boost::asio::ip::tcp::resolver::results_type::endpoint_type&, void*)>	type_callback_connect;
		typedef std::function<void(cws_client::csession&, boost::beast::error_code&, void*)>	type_callback_handshake;
		typedef std::function<void(cws_client::csession&, boost::beast::error_code&, void*)>	type_callback_ssl_handshake;
		typedef std::function<void(cws_client::csession&, boost::beast::error_code&, const type_v_buffer&, void*)>	type_callback_read;
		typedef std::function<void(cws_client::csession&, boost::beast::error_code&, const type_v_buffer&, void*)>	type_callback_write;
		typedef std::function<void(cws_client::csession&, boost::beast::error_code&, void*)>	type_callback_close;
	public:
		class ccallback
		{
		public:
			cws_client::ccallback & set_resolve(
				cws_client::type_callback_resolve cb_resolve
				, void* p_resolve_parameters
			)
			{
				m_cb_resolve = cb_resolve;
				m_p_resolve_parameters = p_resolve_parameters;
				return *this;
			}
			cws_client::ccallback& set_connect(
				cws_client::type_callback_connect cb_connect
				, void* p_connect_parameters
			)
			{
				m_cb_connect = cb_connect;
				m_p_connect_parameters = p_connect_parameters;
				return *this;
			}
			cws_client::ccallback& set_handshake(
				cws_client::type_callback_handshake cb_handshake
				, void* p_handshake_parameters
			)
			{
				m_cb_handshake = cb_handshake;
				m_p_handshake_parameters = p_handshake_parameters;
				return *this;
			}
			cws_client::ccallback& set_ssl_handshake(
				cws_client::type_callback_ssl_handshake cb_ssl_handshake
				, void* p_ssl_handshake_parameters
			)
			{
				m_cb_ssl_handshake = cb_ssl_handshake;
				m_p_ssl_handshake_parameters = p_ssl_handshake_parameters;
				return *this;
			}
			cws_client::ccallback& set_read(
				cws_client::type_callback_read cb_read
				, void* p_read_parameters
			)
			{
				m_cb_read = cb_read;
				m_p_read_parameters = p_read_parameters;
				return *this;
			}
			cws_client::ccallback& set_write(
				cws_client::type_callback_write cb_write
				, void* p_write_parameters
			)
			{
				m_cb_write = cb_write;
				m_p_write_parameters = p_write_parameters;
				return *this;
			}
			cws_client::ccallback& set_close(
				cws_client::type_callback_close cb_close
				, void* p_close_parameters
			)
			{
				m_cb_close = cb_close;
				m_p_close_parameters = p_close_parameters;
				return *this;
			}


			//
			ccallback() :
				m_cb_resolve(nullptr)
				, m_p_resolve_parameters(nullptr)
				, m_cb_connect(nullptr)
				, m_p_connect_parameters(nullptr)
				, m_cb_handshake(nullptr)
				, m_p_handshake_parameters(nullptr)
				, m_cb_ssl_handshake(nullptr)
				, m_p_ssl_handshake_parameters(nullptr)
				, m_cb_read(nullptr)
				, m_p_read_parameters(nullptr)
				, m_cb_write(nullptr)
				, m_p_write_parameters(nullptr)
				, m_cb_close(nullptr)
				, m_p_close_parameters(nullptr)
			{
			}

			virtual ~ccallback()
			{}

			ccallback& operator=(const ccallback& src)
			{
				m_cb_resolve = src.m_cb_resolve;
				m_p_resolve_parameters = src.m_p_resolve_parameters;
				m_cb_connect = src.m_cb_connect;
				m_p_connect_parameters = src.m_p_connect_parameters;
				m_cb_handshake = src.m_cb_handshake;
				m_p_handshake_parameters = src.m_p_handshake_parameters;

				m_cb_ssl_handshake = src.m_cb_ssl_handshake;
				m_p_ssl_handshake_parameters = src.m_p_ssl_handshake_parameters;

				m_cb_read = src.m_cb_read;
				m_p_read_parameters = src.m_p_read_parameters;
				m_cb_write = src.m_cb_write;
				m_p_write_parameters = src.m_p_write_parameters;
				m_cb_close = src.m_cb_close;
				m_p_close_parameters = src.m_p_close_parameters;
				return *this;
			}

			ccallback(const ccallback& src)
			{
				*this = src;
			}

			void run_resolve(cws_client::csession& session, boost::beast::error_code& ec , boost::asio::ip::tcp::resolver::results_type& res_type) 
			{
				if (m_cb_resolve)
					m_cb_resolve(session, ec, res_type, m_p_resolve_parameters);
			}
			void run_handshake(cws_client::csession& session, boost::beast::error_code& ec)
			{
				if (m_cb_handshake)
					m_cb_handshake(session,ec,m_p_handshake_parameters);
			}
			void run_ssl_handshake(cws_client::csession& session, boost::beast::error_code& ec)
			{
				if (m_cb_ssl_handshake)
					m_cb_ssl_handshake(session, ec, m_p_ssl_handshake_parameters);
			}
			void run_connect(cws_client::csession& session, boost::beast::error_code& ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type& end_type)
			{
				if (m_cb_connect)
					m_cb_connect(session, ec, end_type, m_p_connect_parameters);
			}
			void run_read(cws_client::csession& session, boost::beast::error_code& ec, const type_v_buffer& v_buffer)
			{
				if (m_cb_read)
					m_cb_read(session, ec, v_buffer, m_p_read_parameters);
			}
			void run_write(cws_client::csession& session, boost::beast::error_code& ec, const type_v_buffer& v_buffer)
			{
				if (m_cb_write)
					m_cb_write(session, ec, v_buffer, m_p_write_parameters);
			}
			void run_close(cws_client::csession& session, boost::beast::error_code& ec)
			{
				if (m_cb_close)
					m_cb_close(session,ec, m_p_close_parameters);
			}
		private:
			cws_client::type_callback_resolve m_cb_resolve;
			void* m_p_resolve_parameters;

			cws_client::type_callback_connect m_cb_connect;
			void* m_p_connect_parameters;

			cws_client::type_callback_ssl_handshake m_cb_ssl_handshake;
			void* m_p_ssl_handshake_parameters;

			cws_client::type_callback_handshake m_cb_handshake;
			void* m_p_handshake_parameters;

			cws_client::type_callback_read m_cb_read;
			void* m_p_read_parameters;

			cws_client::type_callback_write m_cb_write;
			void* m_p_write_parameters;

			cws_client::type_callback_close m_cb_close;
			void* m_p_close_parameters;

		}; 

	public:
		class csession
		{
		public:
			typedef		std::shared_ptr< cws_client::csession >	type_ptr_session;
		private:
			typedef	std::deque<type_v_buffer>	_type_queue_v_buffer;

			typedef	boost::beast::websocket::stream<boost::beast::tcp_stream>	_type_ws;
			typedef	std::shared_ptr< _type_ws >	_type_ptr_ws;
			typedef	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream> >	_type_wss;
			typedef	std::shared_ptr <_type_wss>	_type_ptr_wss;

		public:
			csession& set_callback(const cws_client::ccallback& cb)
			{
				m_callback = cb;
				return *this;
			}
			// Resolver and socket require an io_context
			explicit csession(boost::asio::io_context& ioc, cws_client& parent_client)
				: 
				m_resolver(boost::asio::make_strand(ioc))
				, m_w_port(0)
				, m_parent_client(parent_client)
			{
				m_b_ssl = parent_client.is_ssl();

				if (m_b_ssl) {
					m_ptr_wss = std::make_shared<csession::_type_wss>(boost::asio::make_strand(ioc), parent_client.get_ssl_context());
				}
				else {
					m_ptr_ws = std::make_shared<csession::_type_ws>(boost::asio::make_strand(ioc));
				}

				clog::get_instance().log_fmt(L"[I] - %ls : X.\n", __WFUNCTION__);
				clog::get_instance().trace(L"[I] - %ls : X.\n", __WFUNCTION__);

			}
			virtual ~csession()
			{
				clog::get_instance().log_fmt(L"[I] - %ls : X.\n", __WFUNCTION__);
				clog::get_instance().trace(L"[I] - %ls : X.\n", __WFUNCTION__);

				std::lock_guard<std::mutex> lock(m_mutex_session);
				if (m_ptr_ws) {
					m_ptr_ws.reset();
				}
				if (m_ptr_wss) {
					m_ptr_wss.reset();
				}
				
				//todo. call close device
			}

			bool is_open()
			{
				bool b_result(false);

				do {
					std::lock_guard<std::mutex> lock(m_mutex_session);
					if (m_b_ssl) {
						if (!m_ptr_wss)
							continue;
						if (m_ptr_wss->is_open())
							b_result = true;
						continue;
					}

					if (!m_ptr_ws)
						continue;
					if (m_ptr_ws->is_open())
						b_result = true;
				} while (false);
				return b_result;
			}
			// Start the asynchronous operation
			void do_resolve(const std::string & s_domain, unsigned short w_port)
			{
				// Save these for later
				m_s_domain = s_domain;
				m_w_port = w_port;

				// Look up the domain name
				m_resolver.async_resolve(m_s_domain, std::to_string((unsigned long)w_port), std::bind(csession::_on_resolve, std::placeholders::_1, std::placeholders::_2, std::ref(m_parent_client)));
			}

			bool do_write(const type_v_buffer& v_data)
			{
				bool b_result(false);
				do {
					if (!is_open())
						continue;
					if (v_data.empty())
						continue;
					m_queue_write.push_back(v_data);

					b_result = true;

					if (m_queue_write.size() > 1)
						continue;
					_write(m_queue_write.front());
					
				} while (false);
				return b_result;
			}

			void do_read()
			{
				std::lock_guard<std::mutex> lock(m_mutex_session);
				if (m_b_ssl) {
					m_ptr_wss->async_read(
						m_buffer_rx,
						std::bind(cws_client::csession::_on_read, std::placeholders::_1, std::placeholders::_2, std::ref(m_parent_client))
					);
				}
				else {
					m_ptr_ws->async_read(
						m_buffer_rx,
						std::bind(cws_client::csession::_on_read, std::placeholders::_1, std::placeholders::_2, std::ref(m_parent_client))
					);
				}
			}

			void do_close()
			{
				std::lock_guard<std::mutex> lock(m_mutex_session);
				if (m_b_ssl) {
					if (m_ptr_wss->is_open()) {
						boost::beast::websocket::close_reason cr(boost::beast::websocket::close_code::normal);
						// Close the WebSocket connection
						m_ptr_wss->async_close(
							cr,
							std::bind(cws_client::csession::_on_close, std::placeholders::_1, std::ref(m_parent_client))
						);
					}
				}
				else {
					if (m_ptr_ws->is_open()) {
						boost::beast::websocket::close_reason cr(boost::beast::websocket::close_code::normal);
						// Close the WebSocket connection
						m_ptr_ws->async_close(
							cr,
							std::bind(cws_client::csession::_on_close, std::placeholders::_1, std::ref(m_parent_client))
						);
					}
				}
			}

		private:
			void _do_connect(boost::asio::ip::tcp::resolver::results_type results)
			{
				std::lock_guard<std::mutex> lock(m_mutex_session);
				if (m_b_ssl) {
					// Set the timeout for the operation
					boost::beast::get_lowest_layer(*m_ptr_wss).expires_after(std::chrono::milliseconds(30));

					// Make the connection on the IP address we get from a lookup
					boost::beast::get_lowest_layer(*m_ptr_wss).async_connect
					(
						results, std::bind(cws_client::csession::_on_connect, std::placeholders::_1, std::placeholders::_2, std::ref(m_parent_client))
					);
				}
				else {
					// Set the timeout for the operation
					boost::beast::get_lowest_layer(*m_ptr_ws).expires_after(std::chrono::milliseconds(30));

					// Make the connection on the IP address we get from a lookup
					boost::beast::get_lowest_layer(*m_ptr_ws).async_connect
					(
						results, std::bind(cws_client::csession::_on_connect, std::placeholders::_1, std::placeholders::_2, std::ref(m_parent_client))
					);
				}
			}

			void _do_ssl_handshake()
			{
				std::lock_guard<std::mutex> lock(m_mutex_session);
				// Set a timeout on the operation
				boost::beast::get_lowest_layer(*m_ptr_wss).expires_after(std::chrono::seconds(30));

				// Perform the websocket handshake
				m_ptr_wss->next_layer().async_handshake(boost::asio::ssl::stream_base::client,
					std::bind(cws_client::csession::_on_ssl_handshake, std::placeholders::_1, std::ref(m_parent_client))
				);
			}
			void _do_handshake()
			{
				std::lock_guard<std::mutex> lock(m_mutex_session);
				if (m_b_ssl) {
					// Turn off the timeout on the tcp_stream, because
					// the websocket stream has its own timeout system.
					boost::beast::get_lowest_layer(*m_ptr_wss).expires_never();

					// Set suggested timeout settings for the websocket
					boost::beast::websocket::stream_base::timeout to_server{};
					to_server.handshake_timeout = std::chrono::seconds(30);
					to_server.idle_timeout = boost::beast::websocket::stream_base::none();
					to_server.keep_alive_pings = false;
					m_ptr_wss->set_option(to_server);

					// Set a decorator to change the User-Agent of the handshake
					m_ptr_wss->set_option(boost::beast::websocket::stream_base::decorator(
						[&](boost::beast::websocket::request_type& req)
						{
							req.set(boost::beast::http::field::server, _WS_TOOLS_SERVER_NAME);
							req.set(boost::beast::http::field::sec_websocket_protocol, _WS_TOOLS_SERVER_PROTOCOL);
						}));

					// Perform the websocket handshake
					m_ptr_wss->async_handshake(m_s_domain, "/",
						std::bind(cws_client::csession::_on_handshake, std::placeholders::_1, std::ref(m_parent_client))
					);
				}
				else {
					// Turn off the timeout on the tcp_stream, because
					// the websocket stream has its own timeout system.
					boost::beast::get_lowest_layer(*m_ptr_ws).expires_never();

					// Set suggested timeout settings for the websocket
					boost::beast::websocket::stream_base::timeout to_server{};
					to_server.handshake_timeout = std::chrono::seconds(30);
					to_server.idle_timeout = boost::beast::websocket::stream_base::none();
					to_server.keep_alive_pings = false;
					m_ptr_ws->set_option(to_server);

					// Set a decorator to change the User-Agent of the handshake
					m_ptr_ws->set_option(boost::beast::websocket::stream_base::decorator(
						[&](boost::beast::websocket::request_type& req)
						{
							req.set(boost::beast::http::field::server, _WS_TOOLS_SERVER_NAME);
							req.set(boost::beast::http::field::sec_websocket_protocol, _WS_TOOLS_SERVER_PROTOCOL);
						}));

					// Perform the websocket handshake
					m_ptr_ws->async_handshake(m_s_domain, "/",
						std::bind(cws_client::csession::_on_handshake, std::placeholders::_1, std::ref(m_parent_client))
					);
				}
			}

			static void _on_resolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results, cws_client & client)
			{
				do {
					cws_client::csession::type_ptr_session ptr_session = client.get_session();
					if (!ptr_session) {
						clog::get_instance().log_fmt(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						clog::get_instance().trace(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						continue;
					}
					ptr_session->m_callback.run_resolve(*ptr_session, ec, results);
					if (ec) {
						clog::get_instance().log_fmt(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						clog::get_instance().trace(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						continue;
					}

					clog::get_instance().log_fmt(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
					clog::get_instance().trace(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());

					ptr_session->_do_connect(results);
				} while (false);
			}

			static void _on_ssl_handshake(boost::beast::error_code ec, cws_client& client)
			{
				do {
					cws_client::csession::type_ptr_session ptr_session = client.get_session();
					if (!ptr_session) {
						clog::get_instance().log_fmt(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						clog::get_instance().trace(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						continue;
					}
					ptr_session->m_callback.run_ssl_handshake(*ptr_session, ec);
					if (ec) {
						clog::get_instance().log_fmt(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						clog::get_instance().trace(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						continue;
					}

					clog::get_instance().log_fmt(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
					clog::get_instance().trace(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());

					ptr_session->_do_handshake();
				} while (false);
			}

			static void _on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type type, cws_client& client)
			{
				do {
					cws_client::csession::type_ptr_session ptr_session = client.get_session();
					if (!ptr_session) {
						clog::get_instance().log_fmt(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						clog::get_instance().trace(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						continue;
					}
					ptr_session->m_callback.run_connect(*ptr_session, ec, type);
					if (ec) {
						clog::get_instance().log_fmt(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						clog::get_instance().trace(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						continue;
					}

					clog::get_instance().log_fmt(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
					clog::get_instance().trace(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());

					if (ptr_session->m_b_ssl) {
						ptr_session->_do_ssl_handshake();
					}
					else {
						ptr_session->_do_handshake();
					}
				} while (false);
			}

			static void _on_handshake(boost::beast::error_code ec, cws_client& client)
			{
				do {
					cws_client::csession::type_ptr_session ptr_session = client.get_session();
					if (!ptr_session) {
						clog::get_instance().log_fmt(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						clog::get_instance().trace(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						continue;
					}
					ptr_session->m_callback.run_handshake(*ptr_session, ec);
					if (ec) {
						clog::get_instance().log_fmt(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						clog::get_instance().trace(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						continue;
					}
					clog::get_instance().log_fmt(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
					clog::get_instance().trace(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());

					//ready for send data ....... good
					//////////////////////////////////////////////
					ptr_session->do_read();// Read a message
				} while (false);
			}

			static void _on_write( boost::beast::error_code ec,std::size_t bytes_transferred, cws_client& client)
			{
				//boost::ignore_unused(bytes_transferred);

				do {
					cws_client::csession::type_ptr_session ptr_session = client.get_session();
					if (!ptr_session) {
						clog::get_instance().log_fmt(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						clog::get_instance().trace(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						continue;
					}

					type_v_buffer v_data(bytes_transferred);
					if (v_data.size() > 0)
						memcpy(&v_data[0], (const unsigned char*)ptr_session->m_buffer_tx.cdata().data(), v_data.size());

					ptr_session->m_callback.run_write(*ptr_session, ec, v_data);

					if (ec) {
						clog::get_instance().log_fmt(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						clog::get_instance().trace(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						continue;
					}
					clog::get_instance().log_fmt(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
					clog::get_instance().trace(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());


					ptr_session->m_queue_write.erase(std::begin(ptr_session->m_queue_write));
					ptr_session->m_buffer_tx.consume(ptr_session->m_buffer_tx.size());// Clear the buffer
					//
					if (!ptr_session->m_queue_write.empty()) {
						ptr_session->_write(ptr_session->m_queue_write.front());
					}
				} while (false);
			}

			static void _on_read(boost::beast::error_code ec, std::size_t bytes_transferred, cws_client& client)
			{
				//boost::ignore_unused(bytes_transferred);

				do {
					cws_client::csession::type_ptr_session ptr_session = client.get_session();
					if (!ptr_session) {
						clog::get_instance().log_fmt(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						clog::get_instance().trace(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						continue;
					}

					type_v_buffer v_data;

					v_data.resize(ptr_session->m_buffer_rx.cdata().size());
					if (v_data.size() > 0)
						memcpy(&v_data[0], (const unsigned char*)ptr_session->m_buffer_rx.cdata().data(), v_data.size());

					ptr_session->m_callback.run_read(*ptr_session, ec, v_data);
					if (ec) {
						clog::get_instance().log_fmt(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						clog::get_instance().trace(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
					}
					else {
						clog::get_instance().log_fmt(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						clog::get_instance().trace(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
					}

					if (ec == boost::beast::websocket::error::closed)
						continue;

					ptr_session->m_buffer_rx.consume(ptr_session->m_buffer_rx.size());// Clear the buffer
					if (ec) {
						clog::get_instance().log_fmt(L"[E] - %ls : consume().\n", __WFUNCTION__);
						clog::get_instance().trace(L"[E] - %ls : consume().\n", __WFUNCTION__);
						ptr_session->do_close();
					}
					else {
						ptr_session->do_read();
					}
				} while (false);
			}

			static void _on_close(boost::beast::error_code ec, cws_client& client)
			{
				do {
					cws_client::csession::type_ptr_session ptr_session = client.get_session();
					if (!ptr_session) {
						clog::get_instance().log_fmt(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						clog::get_instance().trace(L"[I] - %ls : session removed.\n", __WFUNCTION__);
						continue;
					}
					ptr_session->m_callback.run_close(*ptr_session, ec);
					if (ec) {
						clog::get_instance().log_fmt(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						clog::get_instance().trace(L"[E] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
					}
					else {
						clog::get_instance().log_fmt(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
						clog::get_instance().trace(L"[I] - %ls : 0x%p.\n", __WFUNCTION__, ptr_session.get());
					}
				} while (false);
			}

			void _write(const type_v_buffer& v_tx)
			{
				do {
					m_buffer_tx.reserve(v_tx.size());
					if (v_tx.size() > 0) {
						memcpy((unsigned char*)m_buffer_tx.cdata().data(), &v_tx[0], v_tx.size());
					}
					m_buffer_tx.commit(v_tx.size());
					if (m_buffer_tx.size() == 0)
						continue;
					//
					std::lock_guard<std::mutex> lock(m_mutex_session);
					if (m_b_ssl) {
						m_ptr_wss->async_write(
							m_buffer_tx.data(),
							std::bind(cws_client::csession::_on_write, std::placeholders::_1, std::placeholders::_2, std::ref(m_parent_client))
						);
					}
					else {
						m_ptr_ws->async_write(
							m_buffer_tx.data(),
							std::bind(cws_client::csession::_on_write, std::placeholders::_1, std::placeholders::_2, std::ref(m_parent_client))
						);
					}
				} while (false);
			}

		private:
			std::mutex m_mutex_session;
			boost::asio::ip::tcp::resolver m_resolver;
			csession::_type_ptr_ws m_ptr_ws;
			csession::_type_ptr_wss m_ptr_wss;

			boost::beast::flat_buffer m_buffer_rx;
			boost::beast::flat_buffer m_buffer_tx;
			std::string m_s_domain;
			unsigned short m_w_port;

			cws_client& m_parent_client;

			cws_client::ccallback m_callback;
			cws_client::csession::_type_queue_v_buffer m_queue_write;
			bool m_b_ssl;

		private://don't call these methods.
			csession();
			csession(const csession&);
			csession& operator=(const csession&);

		};//the end of session
	
	private:
		typedef		std::map<unsigned long, cws_client::csession::type_ptr_session>	_type_map_index_ptr_session;
	public:
		explicit cws_client(const std::string & s_domain, unsigned short w_port) :
			m_ptr_ioc(std::make_shared<boost::asio::io_context>()),
			//m_ssl_ctx(boost::asio::ssl::context(boost::asio::ssl::context::tlsv12_client))
			m_ssl_ctx(boost::asio::ssl::context(boost::asio::ssl::context::tlsv12))
			,m_s_domain(s_domain), m_w_port(w_port), m_run(false), m_b_ssl(false)
		{
			m_ptr_session = std::make_shared<cws_client::csession>(*m_ptr_ioc, *this);
			if(m_ptr_session)
				m_b_ini = true;
		}

		explicit cws_client(const std::string& s_domain, unsigned short w_port
			, const std::string& s_root_cert_file
		) : m_ptr_ioc(std::make_shared<boost::asio::io_context>()),
			//m_ssl_ctx(boost::asio::ssl::context(boost::asio::ssl::context::tlsv12_client))
			m_ssl_ctx(boost::asio::ssl::context(boost::asio::ssl::context::tlsv13))
			,m_s_domain(s_domain), m_w_port(w_port), m_run(false), m_b_ssl(true)
		{
			do {
				m_ptr_session = std::make_shared<cws_client::csession>(*m_ptr_ioc, *this);
				if (!m_ptr_session)
					continue;
				if (!_load_root_certificate(m_ssl_ctx, s_root_cert_file))
					continue;
				m_b_ini = true;
			} while (false);
		}

		virtual ~cws_client()
		{
			stop();

			if (m_b_ssl)
				_unload_root_certificate(m_ssl_ctx);
			//
			m_ptr_session.reset();//import code. if remove this code, occur access voliation.
			m_ptr_ioc.reset();
		}

		bool is_ini()
		{
			return m_b_ini;
		}

		cws_client & set_callback(const cws_client::ccallback & cb)
		{
			m_callback = cb;
			return *this;
		}

		bool start()
		{
			bool b_result(false);
			do {
				if (!m_b_ini)
					continue;
				if (m_run.load())
					continue;
				m_run.store(true);
				m_ptr_session->set_callback(m_callback).do_resolve(m_s_domain, m_w_port);
				b_result = true;
			} while (false);
			return b_result;
		}

		size_t run()
		{
			size_t n_result(0);
			try {
				
				boost::system::error_code ec;
				n_result = m_ptr_ioc->run(ec);
				while (m_run.load()) {
					m_ptr_ioc->restart();
				}//end while.
			}
			//catch (std::exception& e) {
			//boost::ignore_unused(e);
			catch (...) {
				n_result = -1;
			}
			return n_result;
		}

		cws_client & stop()
		{
			do {
				if (!m_run.load())
					continue;
				m_run.store(false);
				m_ptr_session->do_close();
				while (m_ptr_session->is_open()) {
					std::this_thread::sleep_for(std::chrono::milliseconds(20));
				}
				m_ptr_ioc->stop();
			} while (false);
			return *this;
		}

		bool stopped()
		{
			return m_ptr_ioc->stopped();
		}

		boost::asio::io_context& get_io_context()
		{
			return *m_ptr_ioc;
		}

		cws_client::csession::type_ptr_session get_session()
		{
			return m_ptr_session;
		}

		bool is_ssl()
		{
			return m_b_ssl;
		}
		boost::asio::ssl::context& get_ssl_context()
		{
			return m_ssl_ctx;
		}

	private:

		void _unload_root_certificate(boost::asio::ssl::context& ctx)
		{
			SSL_CTX* sslContext = ctx.native_handle();
		}

#ifdef _WINDOWS
		bool _add_windows_root_certs(boost::asio::ssl::context& ctx)
		{
			bool b_result(false);

			do {
				HCERTSTORE hStore = CertOpenSystemStoreA(0, "ROOT");
				if (hStore == NULL) {
					continue;
				}

				X509_STORE* store = X509_STORE_new();
				PCCERT_CONTEXT pContext = NULL;
				while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != NULL) {
					X509* x509 = d2i_X509(NULL,
						(const unsigned char**)& pContext->pbCertEncoded,
						pContext->cbCertEncoded);
					if (x509 != NULL) {
						X509_STORE_add_cert(store, x509);
						X509_free(x509);
					}
				}

				CertFreeCertificateContext(pContext);
				CertCloseStore(hStore, 0);

				SSL_CTX_set_cert_store(ctx.native_handle(), store);
				b_result = true;
			} while (false);
			return b_result;
		}
#endif
		/*
		static std::string _callback_get_password(
			std::size_t max_length,  // the maximum length for a password
			boost::asio::ssl::context::password_purpose purpose) // for_reading or for_writing
		{
			return "test";
		}
		*/

		bool _load_root_certificate(
			boost::asio::ssl::context& ctx
			, const std::string& s_root_cert_file
		)
		{
			bool b_result(false);

			do {
#ifdef _WINDOWS
				if (!_add_windows_root_certs(ctx))
					continue;
#else
				ctx.set_default_verify_paths();
#endif // _WINDOWS

				//ctx.set_password_callback(cws_client::_callback_get_password);
				b_result = true;
			} while (false);

			return b_result;
		}
	private:
		std::atomic<bool> m_run;

		cws_client::csession::type_ptr_session m_ptr_session;
		bool m_b_ini;

		std::string m_s_domain;
		unsigned short m_w_port;
		//boost::asio::io_context m_ioc;
		std::shared_ptr<boost::asio::io_context> m_ptr_ioc;

		cws_client::ccallback m_callback;
		bool m_b_ssl;
		boost::asio::ssl::context m_ssl_ctx;

	private://don't call these methods.
		cws_client();
		cws_client(const cws_client &);
		cws_client & operator=(const cws_client &);
	};//the end of cws_client
}//the end of _mp