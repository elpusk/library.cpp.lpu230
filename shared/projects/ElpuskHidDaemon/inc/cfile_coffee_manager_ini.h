#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <ctime>

#define BOOST_JSON_HEADER_ONLY
#include <boost/json/src.hpp>

#include <mp_coffee_path.h>
#include <mp_cstring.h>

class cfile_coffee_manager_ini
{
public:
	static cfile_coffee_manager_ini& get_instance()
	{
		static cfile_coffee_manager_ini obj;
		return obj;
	}

	virtual ~cfile_coffee_manager_ini()
	{
	}

	bool load_definition_file(const std::wstring& s_file = L"")
	{
		bool b_result(false);
		do {

			//load ini .json file
			std::wstring ws_path_ini(_mp::ccoffee_path::get_path_of_coffee_mgmt_ini_file());
			if( !s_file.empty()) {
				ws_path_ini = s_file;
			}

			if (ws_path_ini.empty()) {
				continue;
			}


			std::string s_path_ini = _mp::cstring::get_mcsc_from_unicode(ws_path_ini);

			// open json file
			std::ifstream file_json(s_path_ini);
			if (!file_json.is_open()) {
				continue;
			}

			//load json file
			std::string json_str( (std::istreambuf_iterator<char>(file_json)), std::istreambuf_iterator<char>() );
			file_json.close();


			// parse json string
			boost::system::error_code ec;
			boost::json::value jv = boost::json::parse(json_str, ec);
			if (ec) {
				//std::wcerr << L"Error parsing JSON: " << _mp::cstring::get_unicode_from_mcsc(ec.message()) << std::endl;
				continue;
			}

			m_s_ini_file_full_path = ws_path_ini;
			m_b_exist_name = false;
			m_b_exist_version = false;
			m_b_exist_date = false;
			m_b_exist_log_enable = false;
			m_b_exist_tls_enable = false;
			m_b_exist_server_port = false;
			m_b_exist_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client = false;
			m_b_exist_msec_timeout_ws_server_wait_for_idle = false;
			m_b_exist_msec_timeout_ws_server_wait_for_ssl_handshake_complete = false;

			// get JSON data
			boost::json::object obj = jv.as_object();

			///////////////////////////////
			// each element in JSON object
			if(obj.contains("name")) {
				std::string s_name = obj["name"].as_string().c_str();
				m_s_name = _mp::cstring::get_unicode_from_mcsc(s_name);
				m_b_exist_name = true;
			}
			else {
				m_s_name = L"elpusk-hid-d";
			}
			//
			m_s_version = L"2.0";

			if(obj.contains("version")) {
				std::string s_version = obj["version"].as_string().c_str();
				m_s_version = _mp::cstring::get_unicode_from_mcsc(s_version);

				const std::wregex pattern(LR"((25[0-5]|2[0-4]\d|1\d{2}|[1-9]?\d)\.(25[0-5]|2[0-4]\d|1\d{2}|[1-9]?\d))");

				if (!std::regex_match(m_s_version, pattern)) {
					m_s_version = L"2.0";
				}
				else {
					m_b_exist_version = true;
				}
			}
			//
			m_s_date = L"06252025";

			if (obj.contains("date")) {
				std::string s_version = obj["date"].as_string().c_str();
				m_s_date = _mp::cstring::get_unicode_from_mcsc(s_version);

				if (m_s_date.length() == 8 && std::regex_match(m_s_date, std::wregex(LR"(\d{8})"))) {
					int month = std::stoi(m_s_date.substr(0, 2));
					int day = std::stoi(m_s_date.substr(2, 2));
					int year = std::stoi(m_s_date.substr(4, 4));

					std::tm t = {};
					t.tm_year = year - 1900;
					t.tm_mon = month - 1;
					t.tm_mday = day;

					std::mktime(&t); 

					if (t.tm_year != year - 1900 ||
						t.tm_mon != month - 1 ||
						t.tm_mday != day) {
						m_s_version = L"06252025";
					}
					else {
						m_b_exist_date = true;
					}

				}
				else {
					m_s_version = L"06252025";
				}
			}
			//
			m_b_log_enable = true;

			if (obj.contains("log_enable")) {
				std::string s_log = obj["log_enable"].as_string().c_str();
				std::wstring ws_log = _mp::cstring::get_unicode_from_mcsc(s_log);
				if (ws_log == L"true" || ws_log == L"1" || ws_log == L"enable") {
					m_b_log_enable = true;
					m_b_exist_log_enable = true;
				}
				else if (ws_log == L"false" || ws_log == L"0" || ws_log == L"disable") {
					m_b_log_enable = false;
					m_b_exist_log_enable = true;
				}
			}
			//
			m_ll_log_days_to_keep = 3; // default 3 days
			if (obj.contains("log_days")) {
				int n_days = obj["log_days"].as_int64();
				if (n_days > 0) {
					m_ll_log_days_to_keep = (unsigned long long)n_days;
					m_b_exist_log_days_to_keep = true;
				}
				else {
					m_ll_log_days_to_keep = 3; //default is 3 days.
				}
			}
			//
			m_n_server_port = 443;

			int n_port = obj.contains("port") && obj["port"].is_int64() ? obj["port"].as_int64() : -1;
			if (n_port > 0) {
				m_n_server_port = n_port;
				m_b_exist_server_port = true;
			}
			//
			int n_timeout = 0;
			//
			n_timeout = obj.contains("msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client") && obj["msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client"].is_int64() ? obj["msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client"].as_int64() : -1;
			if (n_timeout > 0) {
				m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client = n_timeout;
				m_b_exist_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client = true;
			}
			//
			n_timeout = obj.contains("msec_timeout_ws_server_wait_for_idle") && obj["msec_timeout_ws_server_wait_for_idle"].is_int64() ? obj["msec_timeout_ws_server_wait_for_idle"].as_int64() : -1;
			if (n_timeout > 0) {
				m_ll_msec_timeout_ws_server_wait_for_idle = n_timeout;
				m_b_exist_msec_timeout_ws_server_wait_for_idle = true;
			}
			//
			n_timeout = obj.contains("msec_timeout_ws_server_wait_for_ssl_handshake_complete") && obj["msec_timeout_ws_server_wait_for_ssl_handshake_complete"].is_int64() ? obj["msec_timeout_ws_server_wait_for_ssl_handshake_complete"].as_int64() : -1;
			if (n_timeout > 0) {
				m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete = n_timeout;
				m_b_exist_msec_timeout_ws_server_wait_for_ssl_handshake_complete = true;
			}
			b_result = true;
		} while (false);
		return b_result;
	}

	//get ini file full path
	std::wstring get_ini_file_full_path() const
	{
		return m_s_ini_file_full_path;
	}

	bool get_name(std::wstring& s_name) const
	{
		s_name = m_s_name;
		return m_b_exist_name;
	}

	std::wstring get_name() const
	{
		return m_s_name;
	}

	bool get_version(std::wstring& s_version) const
	{
		s_version = m_s_version;
		return m_b_exist_version;
	}

	std::wstring get_version() const
	{
		return m_s_version;
	}

	bool get_description(std::wstring& s_description) const
	{
		s_description = m_s_description;
		return m_b_exist_description;
	}
	std::wstring get_description() const
	{
		return m_s_description;
	}

	bool get_date(std::wstring& s_date) const
	{
		s_date = m_s_date;
		return m_b_exist_date;
	}

	std::wstring get_date() const
	{
		return m_s_date;
	}

	bool get_log_enable(bool& b_enable) const
	{
		b_enable = m_b_log_enable;
		return m_b_exist_log_enable;
	}

	bool get_log_enable() const
	{
		return m_b_log_enable;
	}

	bool get_log_days_to_keep(unsigned long long& ll_days) const
	{
		ll_days = m_ll_log_days_to_keep;
		return m_b_exist_log_days_to_keep;
	}
	unsigned long long get_log_days_to_keep() const
	{
		return m_ll_log_days_to_keep;
	}
	
	
	bool get_tls_enable(bool& b_enable) const
	{
		b_enable = m_b_tls_enable;
		return m_b_exist_tls_enable;
	}

	bool get_tls_enable() const
	{
		return m_b_tls_enable;
	}

	bool get_server_port(int& n_port) const
	{
		n_port = m_n_server_port;
		return m_b_exist_server_port;
	}

	int get_server_port() const
	{
		return m_n_server_port;
	}

	long long get_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client() const
	{
		return m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client;
	}
	bool get_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client(long long& ll_msec) const
	{
		ll_msec = m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client;
		return m_b_exist_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client;
	}
	//
	long long get_msec_timeout_ws_server_wait_for_idle() const
	{
		return m_ll_msec_timeout_ws_server_wait_for_idle;
	}
	bool get_msec_timeout_ws_server_wait_for_idle(long long& ll_msec) const
	{
		ll_msec = m_ll_msec_timeout_ws_server_wait_for_idle;
		return m_b_exist_msec_timeout_ws_server_wait_for_idle;
	}
	//
	long long get_msec_timeout_ws_server_wait_for_ssl_handshake_complete() const
	{
		return m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete;
	}
	//
	bool get_msec_timeout_ws_server_wait_for_ssl_handshake_complete(long long& ll_msec) const
	{
		ll_msec = m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete;
		return m_b_exist_msec_timeout_ws_server_wait_for_ssl_handshake_complete;
	}


	std::wstring get_string() const
	{
		std::wstringstream ss;

		do {
			if (m_s_ini_file_full_path.empty()) {
				ss << L"[:] INI file not loaded(default setting).\n";
				ss << L"[:] Name(default): " << m_s_name << L"\n"
					<< L"[:] Version(default): " << m_s_version << L"\n"
					<< L"[:] Description(default): " << m_s_description << L"\n"
					<< L"[:] Date(default): " << m_s_date << L"\n"
					<< L"[:] Log Enable(default): " << (m_b_log_enable ? L"true" : L"false") << L"\n"
					<< L"[:] Log Days to Keep(default): " << m_ll_log_days_to_keep << L"\n"
					<< L"[:] TLS Enable(default): " << (m_b_tls_enable ? L"true" : L"false") << L"\n"
					<< L"[:] Server Port(default): " << m_n_server_port << L"\n"
					<< L"[:] WebSocket Upgrade Timeout(default): " << m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client << L"\n"
					<< L"[:] Idle Timeout(default): " << m_ll_msec_timeout_ws_server_wait_for_idle << L"\n"
					<< L"[:] SSL Handshake Complete Timeout(default): " << m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete;
				ss << L"\n";
				continue;
			}

			ss << L"[:] INI file: " << m_s_ini_file_full_path << L"\n";
			if (!m_b_exist_name) {
				ss << L"[:] Name(default): " << m_s_name << L"\n";
			}
			else {
				ss << L"[:] Name: " << m_s_name << L"\n";
			}
			if (!m_b_exist_version) {
				ss << L"[:] Version(default): " << m_s_version << L"\n";
			}
			else {
				ss << L"[:] Version: " << m_s_version << L"\n";
			}
			if (!m_b_exist_description) {
				ss << L"[:] Description(default): " << m_s_description << L"\n";
			}
			else {
				ss << L"[:] Description: " << m_s_description << L"\n";
			}
			if (!m_b_exist_date) {
				ss << L"[:] Date(default): " << m_s_date << L"\n";
			}
			else {
				ss << L"[:] Date: " << m_s_date << L"\n";
			}
			if (!m_b_exist_log_enable) {
				ss << L"[:] Log Enable(default): " << (m_b_log_enable ? L"true" : L"false") << L"\n";
			}
			else {
				ss << L"[:] Log Enable: " << (m_b_log_enable ? L"true" : L"false") << L"\n";
			}
			if (!m_b_exist_log_days_to_keep) {
				ss << L"[:] Log Days to Keep(default): " << m_ll_log_days_to_keep << L"\n";
			}
			else {
				ss << L"[:] Log Days to Keep: " << m_ll_log_days_to_keep << L"\n";
			}
			if (!m_b_exist_tls_enable) {
				ss << L"[:] TLS Enable(default): " << (m_b_tls_enable ? L"true" : L"false") << L"\n";
			}
			else {
				ss << L"[:] TLS Enable: " << (m_b_tls_enable ? L"true" : L"false") << L"\n";
			}
			if (!m_b_exist_server_port) {
				ss << L"[:] Server Port(default): " << m_n_server_port << L"\n";
			}
			else {
				ss << L"[:] Server Port: " << m_n_server_port << L"\n";
			}
			if (!m_b_exist_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client) {
				ss << L"[:] WebSocket Upgrade Timeout(default): " << m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client << L"\n";
			}
			else {
				ss << L"[:] WebSocket Upgrade Timeout: " << m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client << L"\n";
			}
			if (!m_b_exist_msec_timeout_ws_server_wait_for_idle) {
				ss << L"[:] Idle Timeout(default): " << m_ll_msec_timeout_ws_server_wait_for_idle << L"\n";
			}
			else {
				ss << L"[:] Idle Timeout: " << m_ll_msec_timeout_ws_server_wait_for_idle << L"\n";
			}
			if (!m_b_exist_msec_timeout_ws_server_wait_for_ssl_handshake_complete) {
				ss << L"[:] SSL Handshake Complete Timeout(default): " << m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete << L"\n";
			}
			else {
				ss << L"[:] SSL Handshake Complete Timeout: " << m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete << L"\n";
			}
			
		} while (false);

		return ss.str();
	}
private:
	cfile_coffee_manager_ini()
	{
		////
		m_s_name = L"elpusk-hid-d";
		m_s_version = L"2.0";
		m_s_description = L"default";
		m_s_date = L"06252025";
		//
		m_b_exist_name = false;
		m_b_exist_version = false;
		m_b_exist_description = false;
		m_b_exist_date = false;

		////
		m_b_log_enable = true;
		m_ll_log_days_to_keep = 3; // default 7 days to keep log files
		//
		m_b_exist_log_enable = false;
		m_b_exist_log_days_to_keep = false;

		///
		m_b_tls_enable = true;
		m_n_server_port = 443;
		//
		m_b_exist_tls_enable = false;
		m_b_exist_server_port = false;
		//
		m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client = CONST_DEFAULT_WS_SERVER_WAIIT_TIMEOUT_FOR_WEBSOCKET_UPGRADE_REQ_OF_CLIENT_MSEC;
		m_b_exist_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client = false;

		m_ll_msec_timeout_ws_server_wait_for_idle = CONST_DEFAULT_WS_SERVER_WAIIT_TIMEOUT_FOR_IDLE_MSEC;
		m_b_exist_msec_timeout_ws_server_wait_for_idle = false;

		m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete = CONST_DEFAULT_WS_SERVER_WAIIT_TIMEOUT_FOR_SSL_HANSHAKE_COMPLETE_MSEC;
		m_b_exist_msec_timeout_ws_server_wait_for_ssl_handshake_complete = false;
	}
private:
	std::wstring m_s_ini_file_full_path;

	// common part
	std::wstring m_s_name; // the setting application file name(without extension)
	std::wstring m_s_version; // json format version
	std::wstring m_s_description; // description of the setting file
	std::wstring m_s_date; //setting date in format MMDDYYYY

	bool m_b_exist_name;
	bool m_b_exist_version;
	bool m_b_exist_description;
	bool m_b_exist_date;

	// log part
	bool m_b_log_enable;
	unsigned long long m_ll_log_days_to_keep; // how many days to keep log files

	bool m_b_exist_log_enable;
	bool m_b_exist_log_days_to_keep;

	// wensocket server part
	bool m_b_tls_enable;
	int m_n_server_port;

	bool m_b_exist_tls_enable;
	bool m_b_exist_server_port;

	/////////////////////////////////////////////////////////////////////////////////////
	//timeout for websocket server
	// 
	//Timeout after the server receives the ssl handshake request and calls async_accept(), and the client responds with a WebSocket Upgrade request.
	long long m_ll_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client;
	bool m_b_exist_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client;

	//The maximum time that a WebSocket connection can last without sending or receiving any data
	long long m_ll_msec_timeout_ws_server_wait_for_idle;
	bool m_b_exist_msec_timeout_ws_server_wait_for_idle;

	//SSL handshake complete timeout
	long long m_ll_msec_timeout_ws_server_wait_for_ssl_handshake_complete;
	bool m_b_exist_msec_timeout_ws_server_wait_for_ssl_handshake_complete;

private:
	cfile_coffee_manager_ini(const cfile_coffee_manager_ini&) = delete;
	cfile_coffee_manager_ini& operator=(const cfile_coffee_manager_ini&) = delete;
};
