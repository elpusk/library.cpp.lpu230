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

			m_s_ini_file_full_path = s_file_ini;
			std::string s_path_ini = _mp::cstring::get_mcsc_from_unicode(s_file_ini);

            try {
                boost::property_tree::ptree pt;
                boost::property_tree::ini_parser::read_ini(s_path_ini, pt);

				std::string s;

				// common section
				s = pt.get<std::string>("common.name", "");
				if (!s.empty()) {
					m_s_name = _mp::cstring::get_unicode_from_mcsc(s);
					m_b_exist_name = true;
				}
				s = pt.get<std::string>("common.version", "");
				if (!s.empty()) {
					m_s_version = _mp::cstring::get_unicode_from_mcsc(s);;
					m_b_exist_version = true;
				}
				else {
					m_s_version = L"2.0";
				}
				s = pt.get<std::string>("common.description", "");
				if (!s.empty()) {
					m_s_description = _mp::cstring::get_unicode_from_mcsc(s);;
					m_b_exist_description = true;
				}
				s = pt.get<std::string>("common.date", "");
				if (!s.empty()) {
					m_s_date = _mp::cstring::get_unicode_from_mcsc(s);;
					m_b_exist_date = true;
				}
				else {
					m_s_date = L"06262025";
				}

				//log section
				int n_log_enable = pt.get<int>("log.enable", -1);
				if (n_log_enable != -1) {
					m_b_exist_log_enable = true;
					if (n_log_enable == 1) {
						m_b_log_enable = true;
					}
					else {
						m_b_log_enable = false;
					}
				}
				else {
					m_b_log_enable = true; //default is enable.
				}
				long long ll_log_days_to_keep = pt.get<long long>("log.days", -1);
				if (ll_log_days_to_keep != -1) {
					m_b_exist_log_days_to_keep = true;
					m_ll_log_days_to_keep = (unsigned long long)ll_log_days_to_keep;
				}
				else {
					m_ll_log_days_to_keep = 3; //default is 3 days.
				}

				// control section
				int n_io_mode = pt.get<int>("control.io", -1);
				if (n_io_mode >= 0 && n_io_mode <= 2) {
					m_b_exist_io_mode = true;
					m_n_io_mode = n_io_mode;
				}
				else {
					m_n_io_mode = 0; //default is auto io mode.
				}

				// websocket section
				long l_msec_timeout_ws_client_wait_for_connect_api = pt.get<long>("websocket.msec_timeout_ws_client_wait_for_connect_api", -2);
				if( l_msec_timeout_ws_client_wait_for_connect_api != -2) {
					m_b_exist_msec_timeout_ws_client_wait_for_connect_api = true;
					m_l_msec_timeout_ws_client_wait_for_connect_api = l_msec_timeout_ws_client_wait_for_connect_api;
				}
				else {
					m_l_msec_timeout_ws_client_wait_for_connect_api = CONST_DEFAULT_WSS_CONNECT_TIMEOUT_IN_API_MSEC;//default
				}
				long long ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_ssl_handshake_complete", -2);
				if (ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete != -2) {
					m_b_exist_msec_timeout_ws_client_wait_for_ssl_handshake_complete = true;
					m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete = ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete;
				}
				else {
					m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_SSL_HANSHAKE_COMPLETE_MSEC; //default
				}
				long long ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_async_connect_complete_in_wss", -2);
				if (ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss != -2) {
					m_b_exist_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss = true;
					m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss = ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss;
				}
				else {
					m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_ASYNC_CONNECT_COMPLETE_IN_WSS_MSEC; //default
				}
				long long ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss", -2);
				if (ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss != -2) {
					m_b_exist_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss = true;
					m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss = ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss;
				}
				else {
					m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_WEBSOCKET_HANSHAKE_COMPLETE_IN_WSS_MSEC; //default
				}
				long long ll_msec_timeout_ws_client_wait_for_idle_in_wss = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_idle_in_wss", -2);
				if (ll_msec_timeout_ws_client_wait_for_idle_in_wss != -2) {
					m_b_exist_msec_timeout_ws_client_wait_for_idle_in_wss = true;
					m_ll_msec_timeout_ws_client_wait_for_idle_in_wss = ll_msec_timeout_ws_client_wait_for_idle_in_wss;
				}
				else {
					m_ll_msec_timeout_ws_client_wait_for_idle_in_wss = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_IDLE_IN_WSS_MSEC; //default
				}
				//
				long long ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws", -2);
				if (ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws != -2) {
					m_b_exist_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws = true;
					m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws = ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws;
				}
				else {
					m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_WEBSOCKET_HANSHAKE_COMPLETE_IN_WS_MSEC; //default
				}
				long long ll_msec_timeout_ws_client_wait_for_idle_in_ws = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_idle_in_ws", -2);
				if (ll_msec_timeout_ws_client_wait_for_idle_in_ws != -2) {
					m_b_exist_msec_timeout_ws_client_wait_for_idle_in_ws = true;
					m_ll_msec_timeout_ws_client_wait_for_idle_in_ws = ll_msec_timeout_ws_client_wait_for_idle_in_ws;
				}
				else {
					m_ll_msec_timeout_ws_client_wait_for_idle_in_ws = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_IDLE_IN_WS_MSEC; //default
				}
				long long ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws = pt.get<long long>("websocket.msec_timeout_ws_client_wait_for_async_connect_complete_in_ws", -2);
				if (ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws != -2) {
					m_b_exist_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws = true;
					m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws = ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws;
				}
				else {
					m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_ASYNC_CONNECT_COMPLETE_IN_WS_MSEC; //default
				}
            }
            catch (const boost::property_tree::ini_parser_error& e) {
                //std::cerr << "INI parsing: " << e.what() << std::endl;
				continue;
            }
            catch (const std::exception& e) {
                //std::cerr << "error: " << e.what() << std::endl;
				continue;
            }
			//


			b_result = true;
		} while (false);
		if (!b_result) {
			m_s_ini_file_full_path.clear();
		}
		return b_result;

	}

	std::wstring get_string() const
	{
		std::wstringstream ss;

		do {
			if(m_s_ini_file_full_path.empty()) {
				ss << L"[:] INI file not loaded(default setting).\n";
				ss << L"[:] Name(default): " << m_s_name << L"\n"
					<< L"[:] Version(default): " << m_s_version << L"\n"
					<< L"[:] Description(default): " << m_s_description << L"\n"
					<< L"[:] Date(default): " << m_s_date << L"\n"
					<< L"[:] Log Enable(default): " << (m_b_log_enable ? L"true" : L"false") << L"\n"
					<< L"[:] Log Days to Keep(default): " << m_ll_log_days_to_keep << L"\n"
					<< L"[:] IO Mode(default): " << m_n_io_mode << L"\n"
					<< L"[:] msec Timeout WS Client Wait For Connect API(default): " << m_l_msec_timeout_ws_client_wait_for_connect_api << L"\n"
					<< L"[:] msec Timeout WS Client Wait For SSL Handshake Complete(default): " << m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete << L"\n"
					<< L"[:] msec Timeout WS Client Wait For WebSocket Handshake Complete In WSS(default): " << m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss << L"\n"
					<< L"[:] msec Timeout WS Client Wait For Idle In WSS(default): " << m_ll_msec_timeout_ws_client_wait_for_idle_in_wss << L"\n"
					<< L"[:] msec Timeout WS Client Wait For WebSocket Handshake Complete In WS(default): " << m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws << L"\n"
					<< L"[:] msec Timeout WS Client Wait For Idle In WS(default): " << m_ll_msec_timeout_ws_client_wait_for_idle_in_ws << L"\n"
					<< L"[:] msec Timeout WS Client Wait For Async Connect Complete In WSS(default): " << m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss;
				ss << L"\n";
				continue;
			}
			ss << L"[:] INI file loaded : " << m_s_ini_file_full_path << L"\n";
			if( m_b_exist_name ) {
				ss << L"[:] Name: " << m_s_name << L"\n";
			}
			else {
				ss << L"[:] Name(default): " << m_s_name << L"\n";
			}
			if( m_b_exist_version ) {
				ss << L"[:] Version: " << m_s_version << L"\n";
			}
			else {
				ss << L"[:] Version(default): " << m_s_version << L"\n";
			}
			if( m_b_exist_description ) {
				ss << L"[:] Description: " << m_s_description << L"\n";
			}
			else {
				ss << L"[:] Description(default): " << m_s_description << L"\n";
			}
			if( m_b_exist_date ) {
				ss << L"[:] Date: " << m_s_date << L"\n";
			}
			else {
				ss << L"[:] Date(default): " << m_s_date << L"\n";
			}
			if( m_b_exist_log_enable ) {
				ss << L"[:] Log Enable: " << (m_b_log_enable ? L"true" : L"false") << L"\n";
			}
			else {
				ss << L"[:] Log Enable(default): " << (m_b_log_enable ? L"true" : L"false") << L"\n";
			}
			if( m_b_exist_log_days_to_keep ) {
				ss << L"[:] Log Days to Keep: " << m_ll_log_days_to_keep << L"\n";
			}
			else {
				ss << L"[:] Log Days to Keep(default): " << m_ll_log_days_to_keep << L"\n";
			}
			if( m_b_exist_io_mode ) {
				ss << L"[:] IO Mode: " << m_n_io_mode << L"\n";
			}
			else {
				ss << L"[:] IO Mode(default): " << m_n_io_mode << L"\n";
			}
			if( m_b_exist_msec_timeout_ws_client_wait_for_connect_api ) {
				ss << L"[:] msec Timeout WS Client Wait For Connect API: " << m_l_msec_timeout_ws_client_wait_for_connect_api << L"\n";
			}
			else {
				ss << L"[:] msec Timeout WS Client Wait For Connect API(default): " << m_l_msec_timeout_ws_client_wait_for_connect_api << L"\n";
			}
			if( m_b_exist_msec_timeout_ws_client_wait_for_ssl_handshake_complete ) {
				ss << L"[:] msec Timeout WS Client Wait For SSL Handshake Complete: " << m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete << L"\n";
			}
			else {
				ss << L"[:] msec Timeout WS Client Wait For SSL Handshake Complete(default): " << m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete << L"\n";
			}
			if( m_b_exist_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss ) {
				ss << L"[:] msec Timeout WS Client Wait For WebSocket Handshake Complete In WSS: " << m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss << L"\n";
			}
			else {
				ss << L"[:] msec Timeout WS Client Wait For WebSocket Handshake Complete In WSS(default): " << m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss << L"\n";
			}
			if( m_b_exist_msec_timeout_ws_client_wait_for_idle_in_wss ) {
				ss << L"[:] msec Timeout WS Client Wait For Idle In WSS: " << m_ll_msec_timeout_ws_client_wait_for_idle_in_wss << L"\n";
			}
			else {
				ss << L"[:] msec Timeout WS Client Wait For Idle In WSS(default): " << m_ll_msec_timeout_ws_client_wait_for_idle_in_wss << L"\n";
			}
			if( m_b_exist_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws ) {
				ss << L"[:] msec Timeout WS Client Wait For WebSocket Handshake Complete In WS: " << m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws << L"\n";
			}
			else {
				ss << L"[:] msec Timeout WS Client Wait For WebSocket Handshake Complete In WS(default): " << m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws << L"\n";
			}
			if( m_b_exist_msec_timeout_ws_client_wait_for_idle_in_ws ) {
				ss << L"[:] msec Timeout WS Client Wait For Idle In WS: " << m_ll_msec_timeout_ws_client_wait_for_idle_in_ws << L"\n";
			}
			else {
				ss << L"[:] msec Timeout WS Client Wait For Idle In WS(default): " << m_ll_msec_timeout_ws_client_wait_for_idle_in_ws << L"\n";
			}
			if( m_b_exist_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss ) {
				ss << L"[:] msec Timeout WS Client Wait For Async Connect Complete In WSS: " << m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss << L"\n";
			}
			else {
				ss << L"[:] msec Timeout WS Client Wait For Async Connect Complete In WSS(default): " << m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss << L"\n";
			}
		} while (false);
		return ss.str();
	}

	std::wstring get_ini_file_full_path() const
	{
		return m_s_ini_file_full_path;
	}

	std::wstring get_name() const
	{
		return m_s_name;
	}

	bool get_name(std::wstring& s_name) const
	{
		s_name = m_s_name;
		return m_b_exist_name;
	}
	std::wstring get_version() const
	{
		return m_s_version;
	}
	bool get_version(std::wstring& s_version) const
	{
		s_version = m_s_version;
		return m_b_exist_version;
	}
	std::wstring get_description() const
	{
		return m_s_description;
	}
	bool get_description(std::wstring& s_description) const
	{
		s_description = m_s_description;
		return m_b_exist_description;
	}
	std::wstring get_date() const
	{
		return m_s_date;
	}
	bool get_date(std::wstring& s_date) const
	{
		s_date = m_s_date;
		return m_b_exist_date;
	}

	bool get_log_enable() const
	{
		return m_b_log_enable;
	}
	bool get_log_enable(bool& b_log_enable) const
	{
		b_log_enable = m_b_log_enable;
		return m_b_exist_log_enable;
	}
	long long get_log_days_to_keep() const
	{
		return m_ll_log_days_to_keep;
	}
	bool get_log_days_to_keep(unsigned long long& ll_days) const
	{
		ll_days = m_ll_log_days_to_keep;
		return m_b_exist_log_days_to_keep;
	}
	int get_io_mode() const
	{
		return m_n_io_mode;
	}
	bool get_io_mode(int& n_io_mode) const
	{
		n_io_mode = m_n_io_mode;
		return m_b_exist_io_mode;
	}

	long get_msec_timeout_ws_client_wait_for_connect_api() const
	{
		return m_l_msec_timeout_ws_client_wait_for_connect_api;
	}
	bool get_msec_timeout_ws_client_wait_for_connect_api(long& l_msec) const
	{
		l_msec = m_l_msec_timeout_ws_client_wait_for_connect_api;
		return m_b_exist_msec_timeout_ws_client_wait_for_connect_api;
	}

	long long get_msec_timeout_ws_client_wait_for_ssl_handshake_complete() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete;
	}
	bool get_msec_timeout_ws_client_wait_for_ssl_handshake_complete(long long& ll_msec) const
	{
		ll_msec = m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete;
		return m_b_exist_msec_timeout_ws_client_wait_for_ssl_handshake_complete;
	}

	long long get_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss;
	}
	bool get_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss(long long& ll_msec) const
	{
		ll_msec = m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss;
		return m_b_exist_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss;
	}

	long long get_msec_timeout_ws_client_wait_for_idle_in_wss() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_idle_in_wss;
	}
	bool get_msec_timeout_ws_client_wait_for_idle_in_wss(long long& ll_msec) const
	{
		ll_msec = m_ll_msec_timeout_ws_client_wait_for_idle_in_wss;
		return m_b_exist_msec_timeout_ws_client_wait_for_idle_in_wss;
	}

	long long get_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws;
	}
	bool get_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws(long long& ll_msec) const
	{
		ll_msec = m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws;
		return m_b_exist_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws;
	}

	long long get_msec_timeout_ws_client_wait_for_idle_in_ws() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_idle_in_ws;
	}
	bool get_msec_timeout_ws_client_wait_for_idle_in_ws(long long& ll_msec) const
	{
		ll_msec = m_ll_msec_timeout_ws_client_wait_for_idle_in_ws;
		return m_b_exist_msec_timeout_ws_client_wait_for_idle_in_ws;
	}

	long long get_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss;
	}
	bool get_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss(long long& ll_msec) const
	{
		ll_msec = m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss;
		return m_b_exist_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss;
	}

	long long get_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws() const
	{
		return m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws;
	}
	bool get_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws(long long& ll_msec) const
	{
		ll_msec = m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws;
		return m_b_exist_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws;
	}

private:
	cdll_ini()
	{
		m_b_exist_name = false;

		m_b_exist_version = false;

		m_b_exist_description = false;

		m_b_exist_date = false;

		m_b_log_enable = true;
		m_b_exist_log_enable = false;

		m_ll_log_days_to_keep = 3;
		m_b_exist_log_days_to_keep = false;

		m_n_io_mode = 0;
		m_b_exist_io_mode = false;

		m_l_msec_timeout_ws_client_wait_for_connect_api = CONST_DEFAULT_WSS_CONNECT_TIMEOUT_IN_API_MSEC;
		m_b_exist_msec_timeout_ws_client_wait_for_connect_api = false;

		m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_ASYNC_CONNECT_COMPLETE_IN_WSS_MSEC;
		m_b_exist_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss = false;

		m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_SSL_HANSHAKE_COMPLETE_MSEC;
		m_b_exist_msec_timeout_ws_client_wait_for_ssl_handshake_complete = false;

		m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_WEBSOCKET_HANSHAKE_COMPLETE_IN_WSS_MSEC;
		m_b_exist_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss = false;

		m_ll_msec_timeout_ws_client_wait_for_idle_in_wss = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_IDLE_IN_WSS_MSEC;
		m_b_exist_msec_timeout_ws_client_wait_for_idle_in_wss = false;
		
		m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_WEBSOCKET_HANSHAKE_COMPLETE_IN_WS_MSEC;
		m_b_exist_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws = false;

		m_ll_msec_timeout_ws_client_wait_for_idle_in_ws = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_IDLE_IN_WS_MSEC;
		m_b_exist_msec_timeout_ws_client_wait_for_idle_in_ws = false;

		m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws = CONST_DEFAULT_WS_CLIENT_WAIIT_TIMEOUT_FOR_ASYNC_CONNECT_COMPLETE_IN_WS_MSEC;
		m_b_exist_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws = false;
	}

private:
	std::wstring m_s_ini_file_full_path;

	// common section
	std::wstring m_s_name;
	bool m_b_exist_name;
	
	std::wstring m_s_version;
	bool m_b_exist_version;

	std::wstring m_s_description;
	bool m_b_exist_description;

	std::wstring m_s_date;
	bool m_b_exist_date;

	//LogSetting section
	bool m_b_log_enable;
	bool m_b_exist_log_enable;

	unsigned long long m_ll_log_days_to_keep;
	bool m_b_exist_log_days_to_keep;

	//control section
	int m_n_io_mode;
	bool m_b_exist_io_mode;

	//websocket section
	long m_l_msec_timeout_ws_client_wait_for_connect_api;
	bool m_b_exist_msec_timeout_ws_client_wait_for_connect_api;

	long long m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss;
	bool m_b_exist_msec_timeout_ws_client_wait_for_async_connect_complete_in_wss;

	long long m_ll_msec_timeout_ws_client_wait_for_ssl_handshake_complete;
	bool m_b_exist_msec_timeout_ws_client_wait_for_ssl_handshake_complete;

	long long m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss;
	bool m_b_exist_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_wss;

	long long m_ll_msec_timeout_ws_client_wait_for_idle_in_wss;
	bool m_b_exist_msec_timeout_ws_client_wait_for_idle_in_wss;
	//
	long long m_ll_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws;
	bool m_b_exist_msec_timeout_ws_client_wait_for_websocket_handshake_complete_in_ws;

	long long m_ll_msec_timeout_ws_client_wait_for_idle_in_ws;
	bool m_b_exist_msec_timeout_ws_client_wait_for_idle_in_ws;

	long long m_ll_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws;
	bool m_b_exist_msec_timeout_ws_client_wait_for_async_connect_complete_in_ws;

private:
	//don't call these methods.'
	cdll_ini(const cdll_ini&) = delete;
	cdll_ini& operator=(const cdll_ini&) = delete;
};