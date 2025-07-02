#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <ctime>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <mp_coffee_path.h>
#include <mp_cstring.h>
#include <mp_clog.h>

class cdll_ini
{
public:
	~cdll_ini() {}

	static cdll_ini& get_instance()
	{
		static cdll_ini instance;
		return instance;
	}
	//
	bool load_definition_file(const std::wstring& s_file_ini )
	{
		bool b_result(false);

		do {
			if (s_file_ini.empty()) {
				continue;
			}
			std::string s_path_ini = _mp::cstring::get_mcsc_from_unicode(s_file_ini);

            try {
                boost::property_tree::ptree pt;
                boost::property_tree::ini_parser::read_ini(s_path_ini, pt);


				int n_log_enable = pt.get<int>("LogSetting.logenable", -1);
				int n_io_mode = pt.get<int>("control.io", -1);
				long l_msec_timeout_ws_client_wait_for_connect_api = pt.get<long>("websocket.msec_timeout_ws_client_wait_for_connect_api", -2);
				long long ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_ssl_handshake_complete", -2);
				long long ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_async_connect_complete_in_wss", -2);
				long long ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss", -2);
				long long ll_msec_timeout_ws_client_wait_for_idle_in_wss = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_idle_in_wss", -2);
				//
				long long ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws", -2);
				long long ll_msec_timeout_ws_client_wait_for_idle_in_ws = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_idle_in_ws", -2);
				long long ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_async_connect_complete_in_ws", -2);

				m_b_load_ini_file = true;

				m_s_ini_file_full_path = s_file_ini;

				if (n_log_enable != -1) {
					if (n_log_enable == 1) {
						m_b_log_enable = true;
					}
					else {
						m_b_log_enable = false;
					}
				}
				if (n_io_mode >= 0 && n_io_mode <= 2) {
					m_n_io_mode = n_io_mode;
				}

				if( l_msec_timeout_ws_client_wait_for_connect_api != -2) {
					m_l_msec_timeout_ws_client_wait_for_connect_api = l_msec_timeout_ws_client_wait_for_connect_api;
				}
				if (ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss != -2) {
					m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss = ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss;
				}
				if (ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete != -2) {
					m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete = ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete;
				}
				if (ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss != -2) {
					m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss = ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss;
				}
				if (ll_msec_timeout_ws_client_wait_for_idle_in_wss != -2) {
					m_ll_msec_timeout_ws_client_wait_for_idle_in_wss = ll_msec_timeout_ws_client_wait_for_idle_in_wss;
				}
				//
				if (ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws != -2) {
					m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws = ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws;
				}
				if (ll_msec_timeout_ws_client_wait_for_idle_in_ws != -2) {
					m_ll_msec_timeout_ws_client_wait_for_idle_in_ws = ll_msec_timeout_ws_client_wait_for_idle_in_ws;
				}
				if (ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws != -2) {
					m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws = ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws;
				}
            }
            catch (const boost::property_tree::ini_parser_error& e) {
                //std::cerr << "INI parsing: " << e.what() << std::endl;
				m_b_load_ini_file = false;
				continue;
            }
            catch (const std::exception& e) {
                //std::cerr << "error: " << e.what() << std::endl;
				m_b_load_ini_file = false;
				continue;
            }
			//


			b_result = true;
		} while (false);
		return b_result;

	}

	void logging_load_info(_mp::clog& log) const
	{
		log.log_fmt(L"[=============.\n");

		if (m_b_load_ini_file) {
			log.log_fmt(L"[I] loaded ini file = %ls.\n", m_s_ini_file_full_path.c_str());
		}
		else {
			log.log_fmt(L"[I] default setting.\n");
		}

		if (m_b_log_enable) {
			log.log_fmt(L"[I] ini value : log = enable.\n");
		}
		else {
			log.log_fmt(L"[I] ini value : log = disable.\n");
		}

		log.log_fmt(L"[I] ini value : io mode = %d.\n", m_n_io_mode);
		log.log_fmt(L"[I] ini value : msec timeout ws client wait for connect api = %lld.\n", m_l_msec_timeout_ws_client_wait_for_connect_api);
		log.log_fmt(L"[I] ini value : msec timeout ws client wait for async connect complete in wss = %lld.\n", m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss);
		log.log_fmt(L"[I] ini value : msec timeout ws client wait for ssl handshake complete = %lld.\n", m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete);
		log.log_fmt(L"[I] ini value : msec timeout ws client wait for websocket handshake complete in wss = %lld.\n", m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss);
		log.log_fmt(L"[I] ini value : msec timeout ws client wait for idle in wss = %lld.\n", m_ll_msec_timeout_ws_client_wait_for_idle_in_wss);

		log.log_fmt(L"[I] ini value : msec timeout ws client wait for async connect complete in ws = %lld.\n", m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws);
		log.log_fmt(L"[I] ini value : msec timeout ws client wait for websocket handshake complete in ws = %lld.\n", m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws);
		log.log_fmt(L"[I] ini value : msec timeout ws client wait for idle in ws = %lld.\n", m_ll_msec_timeout_ws_client_wait_for_idle_in_ws);
		log.log_fmt(L"[=============.\n");
	}

	bool is_log_enable() const
	{
		return m_b_log_enable;
	}
	int get_io_mode() const
	{
		return m_n_io_mode;
	}

	long get_msec_timeout_ws_client_wait_for_connect_api() const
	{
		return m_l_msec_timeout_ws_client_wait_for_connect_api;
	}
	long long get_msec_timeout_ws_client_wait_for_ssl_handshake_complete() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete;
	}
	long long get_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss;
	}
	long long get_msec_timeout_ws_client_wait_for_idle_in_wss() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_idle_in_wss;
	}
	long long get_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws;
	}
	long long get_msec_timeout_ws_client_wait_for_idle_in_ws() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_idle_in_ws;
	}
	long long get_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss;
	}
	long long get_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws;
	}

private:
	cdll_ini():m_b_load_ini_file(false), m_b_log_enable(false),m_n_io_mode(0)
		, m_l_msec_timeout_ws_client_wait_for_connect_api(CONST_DEFAULT_WSS_CONNECT_TIMEOUT_IN_API_MSEC)
		, m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_SSL_HANSHAKE_COMPLETE_MSEC)
		, m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_WEBSOCKET_HANSHAKE_COMPLETE_IN_WSS_MSEC)
		, m_ll_msec_timeout_ws_client_wait_for_idle_in_wss(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_IDLE_IN_WSS_MSEC)
		, m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_WEBSOCKET_HANSHAKE_COMPLETE_IN_WS_MSEC)
		, m_ll_msec_timeout_ws_client_wait_for_idle_in_ws(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_IDLE_IN_WS_MSEC)
		, m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_ASYNC_CONNECT_COMPLETE_IN_WSS_MSEC)
		, m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws(CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_ASYNC_CONNECT_COMPLETE_IN_WS_MSEC)
	{}

private:
	std::wstring m_s_ini_file_full_path;

	bool m_b_load_ini_file;
	bool m_b_log_enable;
	int m_n_io_mode;

	long m_l_msec_timeout_ws_client_wait_for_connect_api;
	long long m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete;
	long long m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss;
	long long m_ll_msec_timeout_ws_client_wait_for_idle_in_wss;
	long long m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws;
	long long m_ll_msec_timeout_ws_client_wait_for_idle_in_ws;
	long long m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss;
	long long m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws;

private:
	//don't call these methods.'
	cdll_ini(const cdll_ini&) = delete;
	cdll_ini& operator=(const cdll_ini&) = delete;
};