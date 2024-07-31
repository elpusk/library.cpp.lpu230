#pragma once

#include <mutex>
#include <chrono>
#include <random>
#include <deque>
#include <ctime>

#include <boost/asio/strand.hpp>
#include <boost/asio/dispatch.hpp>

#include <websocket/mp_ws_tools.h>

#include <mp_cfile.h>
#include <mp_clog.h>

namespace _mp
{
	// Accepts incoming connections and launches the sessions
	class cws_server
	{
	public:

		typedef	std::shared_ptr< cws_server >	type_ptr;

		class csession;

		typedef	std::function<void(unsigned long, boost::beast::error_code&, void*)>	type_callback_handshake;//for only wss
		typedef	std::function<void(unsigned long, boost::beast::error_code&, void*)>	type_callback_accept;
		typedef	cws_server::type_callback_accept	type_callback_close;
		typedef	std::function<void(unsigned long, boost::beast::error_code&, const _mp::type_v_buffer&, void*)>	type_callback_read;
		typedef	cws_server::type_callback_read	type_callback_write;

		enum {
			const_default_expires_after_sec = 5,
			const_default_handshake_timeout_msec = 1000,
			const_default_idle_timeout_msec = 500,
			const_default_max_rx_size_bytes = (1048576 + 1024)
		};
		enum {
			const_default_max_tx_size_bytes = (1048576 + 1024)//1M + 1K byets
		};

	public:
		class ccallback
		{
		public:
			cws_server::ccallback& set_write(
				cws_server::type_callback_write cb_write
				, void* p_write_parameters
			)
			{
				m_cb_write = cb_write;
				m_p_write_parameters = p_write_parameters;
				return *this;
			}
			cws_server::ccallback& set_read(
				cws_server::type_callback_read cb_read
				, void* p_read_parameters
			)
			{
				m_cb_read = cb_read;
				m_p_read_parameters = p_read_parameters;
				return *this;
			}
			cws_server::ccallback& set_accept(
				cws_server::type_callback_accept cb_accept
				, void* p_accept_parameters
			)
			{
				m_cb_accept = cb_accept;
				m_p_accept_parameters = p_accept_parameters;
				return *this;
			}

			cws_server::ccallback& set_close(
				cws_server::type_callback_close cb_close
				, void* p_close_parameters
			)
			{
				m_cb_close = cb_close;
				m_p_close_parameters = p_close_parameters;
				return *this;
			}
			cws_server::ccallback& set_handshake(
				cws_server::type_callback_handshake cb_handshake
				, void* p_handshake_parameters
			)
			{
				m_cb_handshake = cb_handshake;
				m_p_handshake_parameters = p_handshake_parameters;
				return *this;
			}

			//
			ccallback() :
				m_cb_handshake(nullptr)
				, m_p_handshake_parameters(nullptr)
				, m_cb_accept(nullptr)
				, m_p_accept_parameters(nullptr)
				, m_cb_read(nullptr)
				, m_p_read_parameters(nullptr)
				, m_cb_write(nullptr)
				, m_p_write_parameters(nullptr)
				, m_cb_close(nullptr)
				, m_p_close_parameters(nullptr)
			{
			}

			ccallback(const ccallback& src)
			{
				*this = src;
			}
			ccallback& operator=(const ccallback& src)
			{
				m_cb_handshake = src.m_cb_handshake;
				m_p_handshake_parameters = src.m_p_handshake_parameters;
				m_cb_accept = src.m_cb_accept;
				m_p_accept_parameters = src.m_p_accept_parameters;
				m_cb_read = src.m_cb_read;
				m_p_read_parameters = src.m_p_read_parameters;
				m_cb_write = src.m_cb_write;
				m_p_write_parameters = src.m_p_write_parameters;
				m_cb_close = src.m_cb_close;
				m_p_close_parameters = src.m_p_close_parameters;
				return *this;
			}
			virtual ~ccallback()
			{
			}

			void run_handshake(unsigned long n_session, boost::beast::error_code& ec)
			{
				if (m_cb_handshake)
					m_cb_handshake(n_session, ec, m_p_handshake_parameters);
			}
			void run_accept(unsigned long n_session, boost::beast::error_code& ec)
			{
				if (m_cb_accept)
					m_cb_accept(n_session, ec, m_p_accept_parameters);
			}
			void run_close(unsigned long n_session, boost::beast::error_code& ec)
			{
				if (m_cb_close)
					m_cb_close(n_session, ec, m_p_close_parameters);
			}
			void run_read(unsigned long n_session, boost::beast::error_code& ec, const _mp::type_v_buffer& v_buffer)
			{
				if (m_cb_read)
					m_cb_read(n_session, ec, v_buffer, m_p_read_parameters);
			}
			void run_write(unsigned long n_session, boost::beast::error_code& ec, const _mp::type_v_buffer& v_buffer)
			{
				if (m_cb_write)
					m_cb_write(n_session, ec, v_buffer, m_p_write_parameters);
			}

		private:
			cws_server::type_callback_accept m_cb_handshake;
			void* m_p_handshake_parameters;

			cws_server::type_callback_accept m_cb_accept;
			void* m_p_accept_parameters;
			cws_server::type_callback_close m_cb_close;
			void* m_p_close_parameters;
			cws_server::type_callback_read m_cb_read;
			void* m_p_read_parameters;
			cws_server::type_callback_write m_cb_write;
			void* m_p_write_parameters;

		};//the end of ccallback class

	public:
		class csession : public std::enable_shared_from_this<cws_server::csession>
		{
		public:
			typedef		std::shared_ptr< cws_server::csession >	type_ptr_session;

		private:
			typedef	std::deque<_mp::type_v_buffer>	_type_queue_v_buffer;

			typedef	boost::beast::websocket::stream<boost::beast::tcp_stream>	_type_ws;
			typedef	std::shared_ptr< _type_ws >	_type_ptr_ws;
			typedef	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream> >	_type_wss;
			typedef	std::shared_ptr <_type_wss>	_type_ptr_wss;

		public:
			csession& set_callback(const cws_server::ccallback& cb)
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				m_callback = cb;
				return *this;
			}

			explicit csession(
				boost::asio::ip::tcp::socket& socket, 
				cws_server& parent_server, 
				unsigned long n_session, 
				const std::wstring& s_root_folder_except_backslash)
				:
				m_n_session(n_session)
				, m_s_root_folder_except_backslash(s_root_folder_except_backslash)
				, m_b_now_using_temp_file(false)
				, m_b_setup(false)
				, m_b_dont_use_this_this_will_be_removed(false)
				, m_b_read_mode(false)
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				m_b_ssl = parent_server.is_ssl();

				if (m_b_ssl)
					m_ptr_wss = std::make_shared<csession::_type_wss>(std::move(socket), parent_server.get_ssl_context());
				else
					m_ptr_ws = std::make_shared<csession::_type_ws>(std::move(socket));

				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}
			virtual ~csession()
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				clog::get_instance().log_fmt(L"[I] [%ls ] : %u : deleted csession.\n", __WFUNCTION__, m_n_session);
				if (m_ptr_ws) {
					if (m_ptr_ws->is_open()) {
						boost::beast::get_lowest_layer(*m_ptr_ws).close();
					}
					m_ptr_ws.reset();
				}
				if (m_ptr_wss) {
					if (m_ptr_wss->is_open()) {
						boost::beast::get_lowest_layer(*m_ptr_wss).close();
					}
					boost::beast::get_lowest_layer(*m_ptr_wss).release_socket();
					m_ptr_wss.reset();
				}
				//close all opened files
				close_all_opened_file();
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}

			void request_run()
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session);
					if (m_b_ssl) {
						if (!m_ptr_wss) {
							clog::get_instance().log(L"E : [%ls ] : %u : none wss socket.\n", __WFUNCTION__, m_n_session);
							continue;
						}

						boost::asio::dispatch(m_ptr_wss->get_executor(), boost::beast::bind_front_handler(&csession::_run, shared_from_this()));//shared_from_this()
					}
					else {
						if (!m_ptr_ws) {
							clog::get_instance().log(L"E : [%ls ] : %u : none ws socket.\n", __WFUNCTION__, m_n_session);
							continue;
						}
						boost::asio::dispatch(m_ptr_ws->get_executor(), boost::beast::bind_front_handler(&csession::_run, shared_from_this())); //shared_from_this()
					}
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}

			void close_socket()
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session);

					if (m_b_ssl) {
						boost::beast::get_lowest_layer(*m_ptr_wss).socket().close();
					}
					else {
						boost::beast::get_lowest_layer(*m_ptr_ws).socket().close();
					}

				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}
			bool do_send(const _mp::type_ptr_v_buffer& ptr_v_data)
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				bool b_result(false);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session);

					if (m_b_dont_use_this_this_will_be_removed) {
						clog::get_instance().log(L"I : [%ls ] : %u : dont_use_this_this_will_be_removed.\n", __WFUNCTION__, m_n_session);
						continue;
					}
					if (!ptr_v_data) {
						clog::get_instance().log(L"E : [%ls ] : %u : ptr_v_data is null.\n", __WFUNCTION__, m_n_session);
						continue;
					}
					if (ptr_v_data->empty()) {
						clog::get_instance().log(L"E : [%ls ] : %u : ptr_v_data->empty().\n", __WFUNCTION__, m_n_session);
						continue;
					}
					if (!m_b_setup) {
						clog::get_instance().log(L"I : [%ls ] : %u : is not ready for.\n", __WFUNCTION__, m_n_session);
						continue;
					}

					b_result = true;

					if (m_b_ssl) {
						boost::asio::post(
							m_ptr_wss->get_executor(),
							boost::beast::bind_front_handler(
								&csession::_on_send,
								shared_from_this(),
								m_n_session,
								ptr_v_data
							)//shared_from_this()
						);
					}
					else {
						boost::asio::post(
							m_ptr_ws->get_executor(),
							boost::beast::bind_front_handler(
								&csession::_on_send,
								shared_from_this(),
								m_n_session,
								ptr_v_data
							)//shared_from_this()
						);
					}
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
				return b_result;
			}

			unsigned long get_session_number() const
			{
				return m_n_session;
			}

			void set_virtual_full_path_of_temp_file(const std::wstring& s_virtual_full_path_of_temp_file)
			{
				m_s_virtual_full_path_of_temp_file = s_virtual_full_path_of_temp_file;
			}

			bool is_using_temp_file()
			{
				std::lock_guard<std::mutex> lock(m_mutex_temp_file);
				return m_b_now_using_temp_file;
			}

			void set_using_temp_file(bool b_now_using)
			{
				std::lock_guard<std::mutex> lock(m_mutex_temp_file);
				m_b_now_using_temp_file = b_now_using;
			}

			void close_all_opened_file()
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				std::lock_guard<std::mutex> lock(m_mutex_session_files);

				std::for_each(std::begin(m_map_opened_files), std::end(m_map_opened_files), [&](const _mp::type_pair_wfilename_ptr_fstream& pair_item) {
					if (pair_item.second) {
						if (pair_item.second->is_open()) {
							pair_item.second->close();
							clog::get_instance().log(L"I : [%ls ] : %u : closed : %ls.\n", __WFUNCTION__, m_n_session, pair_item.first.c_str());
						}
					}
					});
				//
				//remove temp file here of session.
				delete_temp_file();//no matter delete is failed or success.
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}

			bool delete_temp_file()
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					if (m_s_virtual_full_path_of_temp_file.empty())
						continue;
					if (is_using_temp_file())
						continue;
#ifdef _WIN32
					std::wstring s_deleted_file_abs_path(m_s_root_folder_except_backslash + L"\\" + m_s_virtual_full_path_of_temp_file);
					if (!DeleteFile(s_deleted_file_abs_path.c_str()))
						continue;
#else
					std::wstring s_deleted_file_abs_path(m_s_root_folder_except_backslash + L"/" + m_s_virtual_full_path_of_temp_file);
					std::string ps = _mp::cstring::get_mcsc_from_unicode(s_deleted_file_abs_path);
					if (unlink(ps.c_str()) != 0) {
						continue;
					}
#endif
					b_result = true;
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
				return b_result;
			}
			bool get_opened_file_path(_mp::type_set_wstring& out_set_opened_file_paths)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session_files);

					out_set_opened_file_paths.clear();
					if (m_map_opened_files.empty())
						continue;

					std::for_each(std::begin(m_map_opened_files), std::end(m_map_opened_files), [&](const _mp::type_pair_wfilename_ptr_fstream& pair_item) {
						if (!pair_item.first.empty()) {
							out_set_opened_file_paths.insert(pair_item.first);
						}
						});
					//
					b_result = true;
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
				return b_result;
			}

			bool get_first_opened_file_path(std::wstring& out_s_paths)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				std::lock_guard<std::mutex> lock(m_mutex_session_files);

				if (m_map_opened_files.empty()) {
					out_s_paths.clear();
				}
				else {
					out_s_paths = std::begin(m_map_opened_files)->first;
					b_result = true;
				}
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
				return b_result;
			}

			bool file_firmware(const std::wstring& s_sub, const std::wstring& s_virtual_abs_file)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session_files);
					if (s_sub.empty())
						continue;

					_mp::type_map_wfilename_ptr_fstream::iterator it = m_map_opened_files.find(s_virtual_abs_file);
					if (it != std::end(m_map_opened_files))
						continue;//already opend

					std::string s_file = _mp::cstring::get_mcsc_from_unicode(m_s_root_folder_except_backslash + L"\\" + s_virtual_abs_file);
					//
					if (s_sub.compare(L"create") == 0) {//create firmware file.
						//check exist file
						std::ifstream in_f(s_file);
						if (in_f.is_open()) {
							in_f.close();
							continue;//file already exsit.
						}
						//
						_mp::type_ptr_fstream ptr_fstream(std::make_shared<std::fstream>());
						// create file
						ptr_fstream->open(s_file
							, std::fstream::out | std::fstream::binary | std::fstream::ate);
						if (!ptr_fstream->is_open())
							continue;

						ptr_fstream->close();
						//reopen for io mode
						ptr_fstream->open(s_file
							, std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::ate);
						if (!ptr_fstream->is_open())
							continue;

						m_map_opened_files[s_virtual_abs_file] = ptr_fstream;
						//
						b_result = true;
						continue;
					}
					if (s_sub.compare(L"delete") == 0) {//delete firmware file.
						std::ifstream in_f(s_file);
						if (!in_f.is_open())
							continue;//not found file
						//
						in_f.close();
#ifdef _WIN32
						std::wstring s_del_file = m_s_root_folder_except_backslash + L"\\" + s_virtual_abs_file;
						if (!DeleteFile(s_del_file.c_str()))
							continue;
#else
						std::wstring s_del_file = m_s_root_folder_except_backslash + L"/" + s_virtual_abs_file;
						std::string ps = _mp::cstring::get_mcsc_from_unicode(s_del_file);
						if (unlink(ps.c_str()) != 0) {
							continue;
						}
#endif
						b_result = true;
					}
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
				return b_result;
			}

			bool file_create(const std::wstring& s_virtual_abs_file)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session_files);
					if (s_virtual_abs_file.empty())
						continue;
					_mp::type_map_wfilename_ptr_fstream::iterator it = m_map_opened_files.find(s_virtual_abs_file);
					if (it != std::end(m_map_opened_files))
						continue;//already opend

					std::string s_file = _mp::cstring::get_mcsc_from_unicode(m_s_root_folder_except_backslash + L"\\" + s_virtual_abs_file);
					//check exist file
					std::ifstream in_f(s_file);
					if (in_f.is_open()) {
						in_f.close();
						continue;//file already exsit.
					}
					//
					_mp::type_ptr_fstream ptr_fstream(std::make_shared<std::fstream>());
					// create file
					ptr_fstream->open(s_file
						, std::fstream::out | std::fstream::binary | std::fstream::ate);
					if (!ptr_fstream->is_open())
						continue;

					ptr_fstream->close();
					//reopen for io mode
					ptr_fstream->open(s_file
						, std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::ate);
					if (!ptr_fstream->is_open())
						continue;

					m_map_opened_files[s_virtual_abs_file] = ptr_fstream;
					//
					b_result = true;
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
				return b_result;
			}
			bool file_open(const std::wstring& s_virtual_abs_file)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session_files);
					if (s_virtual_abs_file.empty())
						continue;
					_mp::type_map_wfilename_ptr_fstream::iterator it = m_map_opened_files.find(s_virtual_abs_file);
					if (it != std::end(m_map_opened_files))
						continue;//already opend

					std::string s_file = _mp::cstring::get_mcsc_from_unicode(m_s_root_folder_except_backslash + L"\\" + s_virtual_abs_file);
					//check exist file
					std::ifstream in_f(s_file);
					if (!in_f.is_open())
						continue;//file not exsit.
					in_f.close();
					//
					_mp::type_ptr_fstream ptr_fstream(std::make_shared<std::fstream>());
					// file must be exsit. else will be failed by std::fstream::in.
					ptr_fstream->open(s_file
						, std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::ate);
					if (!ptr_fstream->is_open())
						continue;

					m_map_opened_files[s_virtual_abs_file] = ptr_fstream;
					//
					b_result = true;
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
				return b_result;
			}
			bool file_is_open(const std::wstring& s_virtual_abs_file)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session_files);
					if (s_virtual_abs_file.empty())
						continue;
					_mp::type_map_wfilename_ptr_fstream::iterator it = m_map_opened_files.find(s_virtual_abs_file);
					if (it == std::end(m_map_opened_files))
						continue;
					//
					b_result = true;
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
				return b_result;
			}
			bool file_close(const std::wstring& s_virtual_abs_file)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session_files);
					if (s_virtual_abs_file.empty())
						continue;
					_mp::type_map_wfilename_ptr_fstream::iterator it = m_map_opened_files.find(s_virtual_abs_file);
					if (it == std::end(m_map_opened_files))
						continue;
					if (!it->second)
						continue;
					it->second->close();
					m_map_opened_files.erase(it);
					//
					b_result = true;
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
				return b_result;
			}
			bool file_truncate(const std::wstring& s_virtual_abs_file)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session_files);
					if (s_virtual_abs_file.empty())
						continue;
					_mp::type_map_wfilename_ptr_fstream::iterator it = m_map_opened_files.find(s_virtual_abs_file);
					if (it == std::end(m_map_opened_files))
						continue;
					if (!it->second)
						continue;
					it->second->close();

					std::wstring s_file = m_s_root_folder_except_backslash + L"\\" + s_virtual_abs_file;
					it->second->open(_mp::cstring::get_mcsc_from_unicode(s_file)
						, std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::trunc);
					//
					b_result = true;
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
				return b_result;
			}
			bool file_write(const std::wstring& s_virtual_abs_file, const _mp::type_v_buffer& v_data)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session_files);
					if (s_virtual_abs_file.empty())
						continue;
					if (v_data.empty())
						continue;
					_mp::type_map_wfilename_ptr_fstream::iterator it = m_map_opened_files.find(s_virtual_abs_file);
					if (it == std::end(m_map_opened_files))
						continue;
					if (!it->second)
						continue;
					it->second->write((const char*)&v_data[0], v_data.size()).flush();
					//
					b_result = true;
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
				return b_result;
			}
			int file_get_size(const std::wstring& s_virtual_abs_file)
			{
				int n_size(-1);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session_files);
					if (s_virtual_abs_file.empty())
						continue;
					_mp::type_map_wfilename_ptr_fstream::iterator it = m_map_opened_files.find(s_virtual_abs_file);
					if (it == std::end(m_map_opened_files))
						continue;
					if (!it->second)
						continue;

					it->second->seekg(0, it->second->end);
					n_size = (int)it->second->tellg();//tellg return the length of file in only binary mode.
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
				return n_size;
			}
			bool is_will_be_removed()
			{
				return m_b_dont_use_this_this_will_be_removed;
			}

		private:
			void _do_close()
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				if (m_b_ssl) {
					if (m_ptr_wss->is_open()) {
						boost::beast::websocket::close_reason cr(boost::beast::websocket::close_code::normal);
						clog::get_instance().log(L"I : [%ls ] : %u : async_close()\n", __WFUNCTION__, m_n_session);
						m_ptr_wss->async_close(
							cr,
							boost::beast::bind_front_handler(&csession::_on_close, shared_from_this(), m_n_session)//shared_from_this()
						);
					}
				}
				else {
					if (m_ptr_ws->is_open()) {
						boost::beast::websocket::close_reason cr(boost::beast::websocket::close_code::normal);
						clog::get_instance().log(L"I : [%ls ] : %u : async_close()\n", __WFUNCTION__, m_n_session);
						m_ptr_ws->async_close(
							cr,
							boost::beast::bind_front_handler(&csession::_on_close, shared_from_this(), m_n_session)//shared_from_this()
						);
					}
				}
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}

			void _do_cancel()
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				if (m_b_ssl) {
					if (m_ptr_wss->is_open()) {
						boost::beast::get_lowest_layer(*m_ptr_wss).cancel();
					}
				}
				else {
					if (m_ptr_ws->is_open()) {
						boost::beast::get_lowest_layer(*m_ptr_ws).cancel();
					}
				}
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}

			void _do_read()
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				// Read a message into our buffer
				if (m_b_ssl) {
					clog::get_instance().log(L"I : [%ls ] : %u : async_read()\n", __WFUNCTION__, m_n_session);
					m_ptr_wss->async_read(
						m_buffer_rx,
						boost::beast::bind_front_handler(&csession::_on_read, shared_from_this(), m_n_session)//shared_from_this()
					);
				}
				else {
					clog::get_instance().log(L"I : [%ls ] : %u : async_read()\n", __WFUNCTION__, m_n_session);
					m_ptr_ws->async_read(
						m_buffer_rx,
						boost::beast::bind_front_handler(&csession::_on_read, shared_from_this(), m_n_session)//shared_from_this()
					);
				}
				m_b_read_mode = true;
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}

			void _on_close(unsigned long n_session, boost::beast::error_code ec)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session);

					close_all_opened_file();
					m_b_dont_use_this_this_will_be_removed = true;

					clog::get_instance().log_fmt(L"[I] ws:: se:: call_on_close::run_close.\n");
					m_callback.run_close(m_n_session, ec);

					if (!ec) {
						b_result = true;
						continue;
					}

					std::wstring ws_msg(_mp::cstring::get_unicode_english_error_message(ec));
					clog::get_instance().log(L"E : [%ls ] : %u : %ls.\n", __WFUNCTION__, m_n_session, ws_msg.c_str());
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}
			void _on_send(unsigned long n_session, const _mp::type_ptr_v_buffer& ptr_data)
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session);

					m_queue_write.push_back(*ptr_data);
					if (m_queue_write.size() > 1)
						continue;
					_write(m_queue_write.front());
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}

			void _on_write(unsigned long n_session, boost::beast::error_code ec, size_t bytes_transferred)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session);

					if (ec) {
						std::wstring ws_msg(_mp::cstring::get_unicode_english_error_message(ec));
						clog::get_instance().log(L"E : [%ls ] : %u : %ls(%d).\n", __WFUNCTION__, m_n_session, ws_msg.c_str(), ec.value());
						m_callback.run_write(m_n_session, ec, _mp::type_v_buffer(0));
						continue;
					}

					_mp::type_v_buffer v_data(bytes_transferred);

					if (v_data.size() > 0)
						memcpy(&v_data[0], (const unsigned char*)m_buffer_tx.cdata().data(), v_data.size());
					m_callback.run_write(m_n_session, ec, v_data);

					m_queue_write.erase(std::begin(m_queue_write));
					m_buffer_tx.consume(m_buffer_tx.size());// Clear the buffer
					//
					if (!m_queue_write.empty()) {
						_write(m_queue_write.front());
					}

					b_result = true;

				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}
			void _on_read(unsigned long n_session, boost::beast::error_code ec, size_t bytes_transferred)
			{
				//boost::ignore_unused(bytes_transferred);
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session);
					clog::get_instance().log(L"I : [%ls ] : %u : bytes_transferred = %u.\n", __WFUNCTION__, m_n_session, (unsigned int)bytes_transferred);

					_mp::type_v_buffer v_data;
					v_data.resize(m_buffer_rx.cdata().size());
					clog::get_instance().log(L"I : [%ls ] : %u : v_data.size() = %u.\n", __WFUNCTION__, m_n_session, (unsigned int)v_data.size());
					if (v_data.size() > 0) {
						memcpy(&v_data[0], (const unsigned char*)m_buffer_rx.cdata().data(), v_data.size());
					}

					m_callback.run_read(m_n_session, ec, v_data);
					if (ec == boost::beast::websocket::error::closed) {
						close_all_opened_file();
						m_b_dont_use_this_this_will_be_removed = true;
						continue;
					}

					m_buffer_rx.consume(m_buffer_rx.size());// Clear the buffer
					if (ec) {
						clog::get_instance().log_fmt(L"[E] call_on_read : (%d) %ls.\n", ec.value(), _mp::cstring::get_unicode_english_error_message(ec).c_str());
						//session.m_s_error = "read";	session.m_ec = ec;
						std::wstring s_bin(L"<T>");

						if (m_b_ssl) {
							if (!m_ptr_wss->is_open()) {
								if (m_ptr_wss->got_binary()) {
									s_bin = L"<B>";
								}

								clog::get_instance().log(L"I : [%ls ] : %u : ssl session is closed.\n", __WFUNCTION__, m_n_session);
								close_all_opened_file();
								m_b_dont_use_this_this_will_be_removed = true;
								clog::get_instance().log_fmt(L"[I] wss%ls:: se:: call_on_read::run_close : %ls.\n", s_bin.c_str(), _mp::cstring::get_unicode_from_mcsc(std::string(m_ptr_wss->reason().reason.c_str())).c_str());
								m_callback.run_close(m_n_session, ec);
							}
						}
						else {
							if (!m_ptr_ws->is_open()) {
								if (m_ptr_ws->got_binary()) {
									s_bin = L"<B>";
								}

								clog::get_instance().log(L"I : [%ls ] : %u : session is closed.\n", __WFUNCTION__, m_n_session);
								close_all_opened_file();
								m_b_dont_use_this_this_will_be_removed = true;
								clog::get_instance().log_fmt(L"[I] ws%ls:: se:: _on_read::run_close(wss) : %ls.\n", s_bin.c_str(), _mp::cstring::get_unicode_from_mcsc(std::string(m_ptr_ws->reason().reason.c_str())).c_str());
								m_callback.run_close(m_n_session, ec);
							}
						}

						clog::get_instance().log(L"I : [%ls ] : %u : %ls(%d).\n", __WFUNCTION__, m_n_session, _mp::cstring::get_unicode_english_error_message(ec).c_str(), ec.value());
						continue;
					}

					_do_read();
					b_result = true;
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}
			void _on_accept(unsigned long n_session, boost::beast::error_code ec)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					//std::lock_guard<std::mutex> lock(m_mutex_session);<><>!!
					clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);

					if (ec) {
						//session.m_s_error = "accept";	session.m_ec = ec;
						std::wstring ws_msg(_mp::cstring::get_unicode_english_error_message(ec));
						clog::get_instance().log(L"E : [%ls ] : %u : %ls(%d).\n", __WFUNCTION__, m_n_session, ws_msg.c_str(), ec.value());
						m_callback.run_accept(m_n_session, ec);
						continue;
					}
					else {
						//session setup ok.
						m_b_setup = true;
						m_callback.run_accept(m_n_session, ec);
					}

					_do_read();// Read a message
					b_result = true;
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}
			void _on_handshake(unsigned long n_session, boost::beast::error_code ec)
			{
				bool b_result(false);
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					//
					std::lock_guard<std::mutex> lock(m_mutex_session);
					clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
					if (!m_b_ssl) {
						clog::get_instance().log(L"I : [%ls ] : %u : not ssl.\n", __WFUNCTION__, m_n_session);
						continue;
					}

					m_callback.run_handshake(m_n_session, ec);

					if (ec) {
						close_all_opened_file();
						m_b_dont_use_this_this_will_be_removed = true;
						//session.m_s_error = "_on_handshake";	session.m_ec = ec;
						std::wstring s_error = _mp::cstring::get_unicode_english_error_message(ec);
						clog::get_instance().log(L"E : [%ls ] : %u : %ls.\n", __WFUNCTION__, m_n_session, s_error.c_str());
						continue;
					}


					// Turn off the timeout on the tcp_stream, because  the websocket stream has its own timeout system.
					boost::beast::get_lowest_layer(*m_ptr_wss).expires_never();

					boost::beast::websocket::stream_base::timeout to_server{};
					to_server.handshake_timeout = std::chrono::milliseconds(cws_server::const_default_handshake_timeout_msec); //std::chrono::seconds(500);
					to_server.idle_timeout = std::chrono::milliseconds(cws_server::const_default_idle_timeout_msec);
					to_server.keep_alive_pings = true;
					m_ptr_wss->set_option(to_server);

					// Set a decorator to change the Server of the handshake
					m_ptr_wss->set_option(boost::beast::websocket::stream_base::decorator(
						[](boost::beast::websocket::response_type& res)
						{
							res.set(boost::beast::http::field::server, _WS_TOOLS_SERVER_NAME);
							res.set(boost::beast::http::field::sec_websocket_protocol, _WS_TOOLS_SERVER_PROTOCOL);
						}));

					// don't use shared_from_this() here, otherwise it creates a cycle!
					// Use shared_from_this when you want to extend the lifetime of the socket.
					m_ptr_wss->control_callback(boost::beast::bind_front_handler(&csession::_on_callback_control, this, m_n_session));

					// Accept the websocket handshake
					clog::get_instance().log(L"I : [%ls ] : %u : async_accept()\n", __WFUNCTION__, m_n_session);
					m_ptr_wss->async_accept(boost::beast::bind_front_handler(&csession::_on_accept, shared_from_this(), m_n_session));//shared_from_this()
					b_result = true;
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}
			void _on_callback_control(unsigned long n_session, boost::beast::websocket::frame_type ftype, boost::beast::string_view str_view)
			{
				bool b_result(false);
				do {
					std::lock_guard<std::mutex> lock(m_mutex_session);
					switch (ftype) {
					case boost::beast::websocket::frame_type::close:
						break;
					case boost::beast::websocket::frame_type::ping:
						continue;
					case boost::beast::websocket::frame_type::pong:
						continue;
					default:
						continue;
					}//end switch
					clog::get_instance().log(L"I : [%ls ] : %u : frame_type::close.\n", __WFUNCTION__, m_n_session);
					m_b_setup = false;

					close_all_opened_file();

					clog::get_instance().log_fmt(L"[I] ws:: se:: call_callback_control::run_close.\n");
					std::wstring s_bin(L"<T>");
					if (m_b_ssl) {
						if (m_ptr_wss->got_binary()) {
							s_bin = L"<B>";
						}
						clog::get_instance().log_fmt(L"[I] wss%ls:: se:: reason : %ls.\n", s_bin.c_str(), cstring::get_unicode_from_mcsc(std::string(m_ptr_wss->reason().reason.c_str())).c_str());
					}
					else {
						if (m_ptr_ws->got_binary()) {
							s_bin = L"<B>";
						}
						clog::get_instance().log_fmt(L"[I] ws%ls:: se:: reason : %ls.\n", s_bin.c_str(), cstring::get_unicode_from_mcsc(std::string(m_ptr_ws->reason().reason.c_str())).c_str());
					}
					//
					m_b_dont_use_this_this_will_be_removed = true;
					//
					boost::beast::error_code ec(boost::beast::websocket::error::closed);
					m_callback.run_close(m_n_session, ec);

					if (m_b_ssl) {
						clog::get_instance().log(L"I : [%ls ] : %u : run_close() ssl session  : %ls.\n", __WFUNCTION__, m_n_session, cstring::get_unicode_from_mcsc(std::string(m_ptr_wss->reason().reason.c_str())).c_str());
					}
					else {
						clog::get_instance().log(L"I : [%ls ] : %u : run_close() session :  %ls.\n", __WFUNCTION__, m_n_session, cstring::get_unicode_from_mcsc(std::string(m_ptr_ws->reason().reason.c_str())).c_str());
					}

					b_result = true;
				} while (false);
			}
			//

			void _write(const _mp::type_v_buffer& v_tx)
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				do {
					//When you call websocket::stream::async_write, 
					//you must wait until the operation completes(your completion handler is invoked) 
					//before calling async_write again on all session.
					if (v_tx.size() == 0)
						continue;
					m_buffer_tx.reserve(v_tx.size());
					memcpy((unsigned char*)m_buffer_tx.cdata().data(), &v_tx[0], v_tx.size());
					m_buffer_tx.commit(v_tx.size());

					if (m_b_read_mode) {
						m_b_read_mode = false;
					}
					//
					if (m_b_ssl) {
						clog::get_instance().log(L"I : [%ls ] : %u : async_write()\n", __WFUNCTION__, m_n_session);
						m_ptr_wss->async_write(
							m_buffer_tx.data(),
							boost::beast::bind_front_handler(&csession::_on_write, shared_from_this(), m_n_session)//shared_from_this()
						);
					}
					else {
						clog::get_instance().log(L"I : [%ls ] : %u : async_write()\n", __WFUNCTION__, m_n_session);
						m_ptr_ws->async_write(
							m_buffer_tx.data(),
							boost::beast::bind_front_handler(&csession::_on_write, shared_from_this(), m_n_session)//shared_from_this()
						);
					}
				} while (false);
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}

			// Start the asynchronous operation
			void _run()
			{
				clog::get_instance().log(L"I : [%ls ] : %u.\n", __WFUNCTION__, m_n_session);
				if (m_b_ssl) {
					boost::beast::get_lowest_layer(*m_ptr_wss).expires_after(std::chrono::seconds(cws_server::const_default_expires_after_sec));

					// Perform the SSL handshake
					clog::get_instance().log(L"I : [%ls ] : %u : async_handshake()\n", __WFUNCTION__, m_n_session);
					m_ptr_wss->next_layer().async_handshake(
						boost::asio::ssl::stream_base::server,
						boost::beast::bind_front_handler(&csession::_on_handshake, shared_from_this(), m_n_session)//shared_from_this()
					);
				}
				else {
					// Set suggested timeout settings for the websocket
					//m_ws.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));
					boost::beast::websocket::stream_base::timeout to_server{};
					to_server.handshake_timeout = std::chrono::milliseconds(cws_server::const_default_handshake_timeout_msec); //std::chrono::seconds(500);
					to_server.idle_timeout = std::chrono::milliseconds(cws_server::const_default_idle_timeout_msec);
					to_server.keep_alive_pings = true;
					m_ptr_ws->set_option(to_server);

					// Set a decorator to change the Server of the handshake
					m_ptr_ws->set_option(boost::beast::websocket::stream_base::decorator(
						[](boost::beast::websocket::response_type& res)
						{
							res.set(boost::beast::http::field::server, _WS_TOOLS_SERVER_NAME);
							res.set(boost::beast::http::field::sec_websocket_protocol, _WS_TOOLS_SERVER_PROTOCOL);
						}));

					// don't use shared_from_this() here, otherwise it creates a cycle!
					// Use shared_from_this when you want to extend the lifetime of the socket.
					m_ptr_ws->control_callback(boost::beast::bind_front_handler(&csession::_on_callback_control, this, m_n_session));

					// Accept the websocket handshake
					clog::get_instance().log(L"I : [%ls ] : %u : async_accept()\n", __WFUNCTION__, m_n_session);
					m_ptr_ws->async_accept(boost::beast::bind_front_handler(&csession::_on_accept, shared_from_this(), m_n_session));//shared_from_this()
				}
				clog::get_instance().log(L"I : [%ls ] : %u : X.\n", __WFUNCTION__, m_n_session);
			}

		private:
			std::mutex m_mutex_session;
			bool m_b_read_mode;
			csession::_type_ptr_ws m_ptr_ws;
			csession::_type_ptr_wss m_ptr_wss;
			boost::beast::flat_buffer m_buffer_rx;
			boost::beast::flat_buffer m_buffer_tx;
			boost::beast::error_code m_ec;
			std::string m_s_error;
			bool m_b_setup;//session setup ok
			bool m_b_dont_use_this_this_will_be_removed;

			cws_server::ccallback m_callback;

			_type_queue_v_buffer m_queue_write;

			const unsigned long m_n_session;//session number
			bool m_b_ssl;

			std::mutex m_mutex_session_files;
			std::wstring m_s_root_folder_except_backslash;
			_mp::type_map_wfilename_ptr_fstream m_map_opened_files;

			std::wstring m_s_virtual_full_path_of_temp_file;

			std::mutex m_mutex_temp_file;
			bool m_b_now_using_temp_file;

		private://don't call these methods.
			csession();
			csession(const csession&);
			csession& operator=(const csession&);
		};//the end of csession class


	private:
		typedef		std::map<unsigned long, cws_server::csession::type_ptr_session>	_type_map_index_ptr_session;
		typedef		std::deque<cws_server::csession::type_ptr_session>	_type_deque_ptr_session;

	public:
		static bool action_by_ws(bool b_start_server, const cws_server::ccallback& callack, unsigned short w_port = _mp::_ws_tools::WEBSOCKET_SERVER_PORT_COFFEE_MANAGER)
		{
			bool b_result(false);
			static boost::asio::ip::address ip6_address = _ws_tools::get_local_ip6();
			static boost::asio::ip::address ip4_address = _ws_tools::get_local_ip4();

			static _mp::cws_server::type_ptr ptr_server_for_ip6;
			static _mp::cws_server::type_ptr ptr_server_for_ip4;

			static std::thread thread_server_ip4;
			static std::thread thread_server_ip6;

			do {
				if (b_start_server) {
					if (ptr_server_for_ip6)
						continue;
					if (ptr_server_for_ip4)
						continue;

					int n_threads = 1;

					// create server
					std::wstring s_root_folder_except_backslash(L".");
					ptr_server_for_ip6 = std::make_shared<_mp::cws_server>(ip6_address, w_port, n_threads, s_root_folder_except_backslash);
					ptr_server_for_ip4 = std::make_shared<_mp::cws_server>(ip4_address, w_port, n_threads, s_root_folder_except_backslash);

					ptr_server_for_ip6->set_callback(callack).start();
					ptr_server_for_ip4->set_callback(callack).start();

					// Run the I/O service on the requested number of threads
					thread_server_ip4 = std::thread(cws_server::_ws_worker, ptr_server_for_ip4.get());
					thread_server_ip6 = std::thread(cws_server::_ws_worker, ptr_server_for_ip6.get());

					b_result = true;
					continue;
				}

				//stop server
				b_result = true;

				do {
					if (!ptr_server_for_ip6)
						continue;
					if (!ptr_server_for_ip6->stopped()) {
						ptr_server_for_ip6->stop();
						while (!ptr_server_for_ip6->stopped()) {
#ifdef _WIN32
							Sleep(20); 
#else
							usleep(20 * 1000);
#endif
						}
					}

					if (thread_server_ip6.joinable())
						thread_server_ip6.join();

					ptr_server_for_ip6.reset();
				} while (false);

				do {
					if (!ptr_server_for_ip4)
						continue;
					if (!ptr_server_for_ip4->stopped()) {
						ptr_server_for_ip4->stop();
						while (!ptr_server_for_ip4->stopped()) {
#ifdef _WIN32
							Sleep(20);
#else
							usleep(20 * 1000);
#endif

						}
					}

					if (thread_server_ip4.joinable())
						thread_server_ip4.join();

					ptr_server_for_ip4.reset();
				} while (false);
			} while (false);

			return b_result;
		}

	private:
		static void _ws_worker(cws_server* p_server)
		{
			do {
				if (p_server == nullptr)
					continue;
				p_server->run();
			} while (false);
		}

	public:

		//for ws server
		explicit cws_server(const boost::asio::ip::address& addr, unsigned short w_port, int n_concurrency_hint_of_io_context, const std::wstring& s_root_folder_except_backslash)
			: m_b_need_free_x509(false)
			, m_b_need_free_evp_pkey(false)
			, m_b_ssl(false)
			, m_s_root_folder_except_backslash(s_root_folder_except_backslash)
		{//ws constructor

			m_ptr_ioc = std::make_shared<boost::asio::io_context>(n_concurrency_hint_of_io_context);
			m_ptr_endpoint = std::make_shared<boost::asio::ip::tcp::endpoint>(addr, w_port);

			do {
				_ini();
				m_n_concurrency_hint_of_io_context = n_concurrency_hint_of_io_context;

				if (n_concurrency_hint_of_io_context <= 0) {
					//m_s_error = "concurrency_hint_of_io_context";
					clog::get_instance().log(L"E : [%ls ] : concurrency_hint_of_io_context.\n", __WFUNCTION__);
					continue;
				}

				m_uptr_acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(boost::asio::make_strand(*m_ptr_ioc));
				if (!m_uptr_acceptor) {
					clog::get_instance().log(L"E : [%ls ] : create  tcp::acceptor.\n", __WFUNCTION__);
					continue;
				}

				boost::beast::error_code ec;

				// Open the acceptor
				m_uptr_acceptor->open(m_ptr_endpoint->protocol(), ec);
				if (ec) {
					std::wstring ws_msg(_mp::cstring::get_unicode_english_error_message(ec));
					clog::get_instance().log_fmt(L"E : [%ls ] : open() : %ls\n", __WFUNCTION__, ws_msg.c_str());
					continue;
				}

				// Allow address reuse
				m_uptr_acceptor->set_option(boost::asio::socket_base::reuse_address(true), ec);
				if (ec) {
					std::wstring ws_msg(_mp::cstring::get_unicode_english_error_message(ec));
					clog::get_instance().log_fmt(L"E : [%ls ] : set_option() : %ls\n", __WFUNCTION__, ws_msg.c_str());
					continue;
				}

				// Bind to the server address
				m_uptr_acceptor->bind(*m_ptr_endpoint, ec);
				if (ec) {
					std::wstring ws_msg(_mp::cstring::get_unicode_english_error_message(ec));
					clog::get_instance().log_fmt(L"E : [%ls ] : bind() : %ls\n", __WFUNCTION__, ws_msg.c_str());
					continue;
				}

				// Start listening for connections
				m_uptr_acceptor->listen(boost::asio::socket_base::max_listen_connections, ec);
				if (ec) {
					std::wstring ws_msg(_mp::cstring::get_unicode_english_error_message(ec));
					clog::get_instance().log_fmt(L"E : [%ls ] : listen() : %ls\n", __WFUNCTION__, ws_msg.c_str());
					continue;
				}

				m_b_ini = true;

				clog::get_instance().log(L"I : [%ls ] : constructor() : p_ico = 0x%p.\n", __WFUNCTION__, m_ptr_ioc.get());
			} while (false);
		}

		//for wss server
		explicit cws_server(const boost::asio::ip::address& addr, unsigned short w_port, int n_concurrency_hint_of_io_context
			, const std::string& s_server_cert_file
			, const std::string& s_server_private_key_file
			, const std::wstring& s_root_folder_except_backslash
		)
			: m_b_need_free_x509(false)
			, m_b_need_free_evp_pkey(false)
			, m_b_ssl(true)
			, m_s_root_folder_except_backslash(s_root_folder_except_backslash)
		{
			m_ptr_ioc = std::make_shared<boost::asio::io_context>(n_concurrency_hint_of_io_context);
			m_ptr_endpoint = std::make_shared<boost::asio::ip::tcp::endpoint>(addr, w_port);
			do {
				_ini();
				m_ptr_ssl_ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context(boost::asio::ssl::context::tlsv13));
				m_n_concurrency_hint_of_io_context = n_concurrency_hint_of_io_context;

				if (n_concurrency_hint_of_io_context <= 0) {
					clog::get_instance().log(L"E : [%ls ] : concurrency_hint_of_io_context.\n", __WFUNCTION__);
					continue;
				}

				if (s_server_cert_file.empty())
					continue;
				if (s_server_private_key_file.empty())
					continue;
				bool b_load = _load_server_certificate(*m_ptr_ssl_ctx, s_server_cert_file, s_server_private_key_file);

				if (!b_load) {
					_unload_server_certificate(*m_ptr_ssl_ctx);
					clog::get_instance().log(L"E : [%ls ] : _load_server_certificate().\n", __WFUNCTION__);
					continue;
				}

				m_uptr_acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(boost::asio::make_strand(*m_ptr_ioc));
				if (!m_uptr_acceptor) {
					clog::get_instance().log(L"E : [%ls ] : create  tcp::acceptor.\n", __WFUNCTION__);
					continue;
				}
				boost::beast::error_code ec;

				// Open the acceptor
				m_uptr_acceptor->open(m_ptr_endpoint->protocol(), ec);
				if (ec) {
					std::wstring ws_msg(_mp::cstring::get_unicode_english_error_message(ec));
					clog::get_instance().log_fmt(L"E : [%ls ] : open() : %ls.\n", __WFUNCTION__, ws_msg.c_str());
					continue;
				}

				// Allow address reuse
				m_uptr_acceptor->set_option(boost::asio::socket_base::reuse_address(true), ec);
				if (ec) {
					std::wstring ws_msg(_mp::cstring::get_unicode_english_error_message(ec));
					clog::get_instance().log_fmt(L"E : [%ls ] : set_option() : %ls.\n", __WFUNCTION__, ws_msg.c_str());
					continue;
				}

				// Bind to the server address
				m_uptr_acceptor->bind(*m_ptr_endpoint, ec);
				if (ec) {
					std::wstring ws_msg(_mp::cstring::get_unicode_english_error_message(ec));
					clog::get_instance().log_fmt(L"E : [%ls ] : bind() : %ls.\n", __WFUNCTION__, ws_msg.c_str());
					continue;
				}

				// Start listening for connections
				m_uptr_acceptor->listen(boost::asio::socket_base::max_listen_connections, ec);
				if (ec) {
					std::wstring ws_msg(_mp::cstring::get_unicode_english_error_message(ec));
					clog::get_instance().log_fmt(L"E : [%ls ] : listen() : %ls.\n", __WFUNCTION__, ws_msg.c_str());
					continue;
				}

				m_b_ini = true;
				clog::get_instance().log(L"I : [%ls ] : constructor()s : p_ico = 0x%p.\n", __WFUNCTION__, m_ptr_ioc.get());
			} while (false);
		}

		virtual ~cws_server()
		{
			if (m_ptr_ioc)
				m_ptr_ioc->stop();
			//
			_remove_all_sesssion();
			//
			if (m_b_ssl)
				_unload_server_certificate(*m_ptr_ssl_ctx);
			if (m_ptr_ssl_ctx)
				m_ptr_ssl_ctx.reset();
			//
			m_uptr_acceptor.reset();
			m_ptr_endpoint.reset();

			clog::get_instance().log(L"I : [%ls ] : destructor() : p_ico = 0x%p.\n", __WFUNCTION__, m_ptr_ioc.get());
			m_ptr_ioc.reset();
		}

		bool is_ini()
		{
			return m_b_ini;
		}

		cws_server& set_callback(const cws_server::ccallback& cb)
		{
			m_callback = cb;
			return *this;
		}

		// Start accepting incoming connections
		cws_server& start()
		{
			_do_accept();
			return *this;
		}
		cws_server& stop()
		{
			if (m_ptr_ioc) {
				m_ptr_ioc->stop();
			}
			return *this;
		}
		bool stopped()
		{
			if (m_ptr_ioc)
				return m_ptr_ioc->stopped();
			else
				return true;
		}
		size_t run()
		{
			size_t n_result(0);
			bool b_run(true);
			do {
				try {
					n_result = m_ptr_ioc->run();
					b_run = false;//exit normally.
				}
				catch (std::exception& e) {
					n_result = -1;
					std::wstring ws_error;
					std::string s_error(e.what());
					ws_error.assign(std::begin(s_error), std::end(s_error));

					clog::get_instance().log(L"[E] exception server thread : %ls\n", ws_error.c_str());
					clog::get_instance().log_fmt(L"E : [%ls ] : exception : %ls.\n", __WFUNCTION__, ws_error.c_str());
				}
			} while (b_run);
			clog::get_instance().log(L"[I] exit server thread\n");
			clog::get_instance().log(L"I : [%ls ] : exit.\n", __WFUNCTION__);
			return n_result;
		}

		boost::asio::io_context& get_io_context()
		{
			return *m_ptr_ioc;
		}

		std::mutex& get_mutex_write()
		{
			return m_mutex_write;
		}

		cws_server::csession::type_ptr_session get_session(unsigned long n_session)
		{
			cws_server::csession::type_ptr_session ptr_session;
			do {
				std::lock_guard<std::mutex> lock(m_mutex_map_session);
				_type_map_index_ptr_session::iterator it = m_map_ptr_session.find(n_session);
				if (it == std::end(m_map_ptr_session))
					continue;
				//
				if (!it->second)
					continue;
				if (it->second->is_will_be_removed())
					continue;
				ptr_session = it->second;
			} while (false);
			return ptr_session;
		}

		bool is_ssl()
		{
			return m_b_ssl;
		}
		boost::asio::ssl::context& get_ssl_context()
		{
			return *m_ptr_ssl_ctx;
		}

		void broadcast(unsigned long n_owner_session, const _mp::type_v_buffer& v_data)
		{
			do {
				if (v_data.empty())
					continue;

				std::lock_guard<std::mutex> lock(m_mutex_map_session);
				_type_map_index_ptr_session::iterator it = std::begin(m_map_ptr_session);
				for (; it != std::end(m_map_ptr_session); ++it) {
					if (it->second) {
						if (it->second->get_session_number() != n_owner_session) {
							it->second->do_send(std::make_shared<_mp::type_v_buffer>(v_data));
						}
					}
				}//end for
			} while (false);
		}
	private:
		void _ini()
		{
			m_b_ini = false;
			m_n_concurrency_hint_of_io_context = 1;
			m_n_next_session = _generate_session_number();
		}

		void _do_accept()
		{
			clog::get_instance().log(L"I : [%ls ].\n", __WFUNCTION__);
			// The new connection gets its own strand
			clog::get_instance().log(L"I : [%ls ] : async_accept()\n", __WFUNCTION__);
			m_uptr_acceptor->async_accept(
				boost::asio::make_strand(*m_ptr_ioc),
				boost::beast::bind_front_handler(&cws_server::_on_accept, this)
			);
		}

		//static void _on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket, cws_server & server)
		void _on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
		{
			clog::get_instance().log(L"I : [%ls ].\n", __WFUNCTION__);
			do {
				_remove_remove_reserved_sesssion();
				if (ec) {
					//server.m_s_error = "accept";	server.m_ec = ec;
					std::wstring ws_msg(_mp::cstring::get_unicode_english_error_message(ec));
					clog::get_instance().log_fmt(L"E : [%ls ] : %ls.\n", __WFUNCTION__, ws_msg.c_str());
					continue;
				}
				// Create the session and run it
				cws_server::csession::type_ptr_session& ptr_session(_create_session_and_insert_to_map(socket));
				if (!ptr_session) {
					clog::get_instance().log_fmt(L"E : [%ls ] : _create_session_and_insert_to_map().\n", __WFUNCTION__);
					continue;
				}
				ptr_session->set_callback(m_callback);
				ptr_session->request_run();
			} while (false);

			_do_accept();// Accept another connection
		}

		cws_server::csession::type_ptr_session& _create_session_and_insert_to_map(boost::asio::ip::tcp::socket& socket)
		{
			static cws_server::csession::type_ptr_session ptr_empty_session;

			std::lock_guard<std::mutex> lock(m_mutex_map_session);
			auto result = m_map_ptr_session.emplace(
				m_n_next_session,
				std::make_shared<cws_server::csession>(
					socket,
					*this,
					m_n_next_session,
					m_s_root_folder_except_backslash
				)->shared_from_this()
			);
			if (!result.second) {
				clog::get_instance().log(L"E : [%ls ] : emplace map.\n", __WFUNCTION__);
				return ptr_empty_session;
			}
			else {
				clog::get_instance().log(L"X : [%ls ] : %u : 0x%p.\n", __WFUNCTION__, result.first->first, result.first->second.get());

				if (result.first->second.unique()) {
					clog::get_instance().log(L"X7 : [%ls ] : unique.\n", __WFUNCTION__);
				}
				else {
					clog::get_instance().log(L"X7 : [%ls ] : not unique : %d.\n", __WFUNCTION__, result.first->second.use_count());
				}

				unsigned long n_cur_session = m_n_next_session;

				//generate session number for next time.
				unsigned long n_next_session(_generate_session_number());

				while (m_map_ptr_session.find(n_next_session) != std::end(m_map_ptr_session)) {
					n_next_session = _generate_session_number();//regenerate
				}//end while
				m_n_next_session = n_next_session;
				clog::get_instance().log(L"I : [%ls ] : current session_number = %u.\n", __WFUNCTION__, n_cur_session);
				//
				return result.first->second;
			}
		}

		void _remove_remove_reserved_sesssion()
		{
			clog::get_instance().log(L"I : [%ls ].\n", __WFUNCTION__);
			std::lock_guard<std::mutex> lock(m_mutex_map_session);

			cws_server::_type_map_index_ptr_session::iterator it = std::begin(m_map_ptr_session);

			for (; it != std::end(m_map_ptr_session);) {
				if (it->second) {
					if (it->second->is_will_be_removed()) {
						if (it->second.unique()) {
							clog::get_instance().log(L"X2 : [%ls ] : %u : unique.\n", __WFUNCTION__, it->second->get_session_number());
							clog::get_instance().log(L"X2 : [%ls ] : %u : will be removed.\n", __WFUNCTION__, it->second->get_session_number());
						}
						else {
							clog::get_instance().log(L"X2 : [%ls ] : %u : not unique : address = 0x%p : cnt = %d.\n",
								__WFUNCTION__,
								it->second->get_session_number(),
								it->second.get(),
								it->second.use_count());
						}
						//
						it->second.reset();//call destructor
						m_map_ptr_session.erase(it++);
						continue;
					}
				}

				++it;
			}//end for

		}
		void _remove_all_sesssion()
		{
			clog::get_instance().log(L"I : [%ls ].\n", __WFUNCTION__);
			std::lock_guard<std::mutex> lock(m_mutex_map_session);
			//
			for (auto item : m_map_ptr_session) {
				if (item.second) {
					item.second.reset();
				}
			}
			//
			m_map_ptr_session.clear();
		}

		static bool _cb_verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx)
		{
			int8_t subject_name[256];
			int32_t length = 0;
			X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
			clog::get_instance().log(L"E : [%ls ] : CTX ERROR : %d\n", __WFUNCTION__, X509_STORE_CTX_get_error(ctx.native_handle()));

			int32_t depth = X509_STORE_CTX_get_error_depth(ctx.native_handle());
			clog::get_instance().log(L"E : [%ls ] : CTX DEPTH : %d\n", __WFUNCTION__, depth);

			switch (X509_STORE_CTX_get_error(ctx.native_handle()))
			{
			case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
				clog::get_instance().log(L"E : [%ls ] : X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT\n", __WFUNCTION__);
				break;
			case X509_V_ERR_CERT_NOT_YET_VALID:
			case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
				clog::get_instance().log(L"E : [%ls ] : Certificate not yet valid!n", __WFUNCTION__);
				break;
			case X509_V_ERR_CERT_HAS_EXPIRED:
			case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
				clog::get_instance().log(L"E : [%ls ] : Certificate expired..\n", __WFUNCTION__);
				break;
			case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
				clog::get_instance().log(L"I : [%ls ] : Self signed certificate in chain!!!\n", __WFUNCTION__);
				preverified = true;
				break;
			default:
				break;
			}
			const int32_t name_length = 256;
			X509_NAME_oneline(X509_get_subject_name(cert), reinterpret_cast<char*>(subject_name), name_length);
			std::wstring sw_subject_name = _mp::cstring::get_unicode_from_mcsc(std::string(reinterpret_cast<char*>(subject_name)));
			clog::get_instance().log_fmt(L"I : [%ls ] : Verifying %ls", __WFUNCTION__, sw_subject_name.c_str());
			clog::get_instance().log(L"I : [%ls ] : Verification status : %d", __WFUNCTION__, preverified);
			return preverified;
		}

		void _unload_server_certificate(boost::asio::ssl::context& ctx)
		{
			SSL_CTX* sslContext = ctx.native_handle();
			/*
			if (m_b_need_free_evp_pkey)
				EVP_PKEY_free(&ctx.native_handle);
			if (m_b_need_free_x509)
				X509_free(&sslContext->CERT);
			*/
			m_b_need_free_evp_pkey = m_b_need_free_x509 = false;
		}

		bool _load_server_certificate(
			boost::asio::ssl::context& ctx
			, const std::string& s_server_cert_file
			, const std::string& s_server_private_key_file
		)
		{
			bool b_result(false);

			do {
				//ctx.set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::single_dh_use);
				ctx.set_options(boost::asio::ssl::context::default_workarounds);

				boost::system::error_code ec;

				ctx.use_certificate_file(s_server_cert_file, boost::asio::ssl::context::pem, ec);//pem format
				if (ec.failed()) {
					continue;
				}
				m_b_need_free_x509 = true;
				ec.clear();
				ctx.use_private_key_file(s_server_private_key_file, boost::asio::ssl::context::pem, ec);//pem format
				if (ec.failed()) {
					continue;
				}
				m_b_need_free_evp_pkey = true;
				ec.clear();
				//ctx.use_tmp_dh_file("../server_cert/coffee-manager-dh.pem",ec);
				if (ec.failed()) {
					continue;
				}

				b_result = true;
			} while (false);

			return b_result;
		}

		unsigned long _generate_session_number()
		{
			unsigned long n_session(_MP_TOOLS_INVALID_SESSION_NUMBER);
			auto seed = std::chrono::system_clock::now().time_since_epoch().count();
			std::mt19937 engine((unsigned int)seed);
			std::uniform_int_distribution<unsigned int> distribution(0, 0xFFFFFFFE);
			auto generator = std::bind(distribution, engine);
			do {
				n_session = (unsigned long)generator();
			} while (n_session == _MP_TOOLS_INVALID_SESSION_NUMBER);
			return n_session;
		}
	private:
		bool m_b_need_free_x509;
		bool m_b_need_free_evp_pkey;

		std::mutex m_mutex_write;

		std::mutex m_mutex_map_session;
		cws_server::_type_map_index_ptr_session	m_map_ptr_session;

		bool m_b_ini;
		int m_n_concurrency_hint_of_io_context;
		std::shared_ptr<boost::asio::ip::tcp::endpoint> m_ptr_endpoint;

		std::shared_ptr < boost::asio::io_context > m_ptr_ioc;

		std::unique_ptr<boost::asio::ip::tcp::acceptor> m_uptr_acceptor;
		boost::beast::error_code m_ec;
		std::string m_s_error;

		cws_server::ccallback m_callback;

		bool m_b_ssl;
		//boost::asio::ssl::context m_ssl_ctx;
		std::shared_ptr<boost::asio::ssl::context> m_ptr_ssl_ctx;

		unsigned long m_n_next_session;

		std::wstring m_s_root_folder_except_backslash;
	private://don't call these method
	};//the end class

}//the end of _mp
