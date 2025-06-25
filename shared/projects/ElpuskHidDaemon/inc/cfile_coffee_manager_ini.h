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
			m_s_version = L"06252025";

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

			if (obj.contains("log")) {
				std::string s_log = obj["log"].as_string().c_str();
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
			m_n_server_port = 443;

			int n_port = obj.contains("port") && obj["port"].is_int64() ? obj["age"].as_int64() : -1;
			if (n_port > 0) {
				m_n_server_port = n_port;
				m_b_exist_server_port = true;
			}

			b_result = true;
		} while (false);
		return b_result;
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

private:
	cfile_coffee_manager_ini()
	{
		m_s_name = L"elpusk-hid-d";
		m_s_version = L"2.0";
		m_s_date = L"06252025";
		m_b_log_enable = true;
		m_b_tls_enable = true;
		m_n_server_port = 443;
		//
		m_b_exist_name = false;
		m_b_exist_version = false;
		m_b_exist_date = false;
		m_b_exist_log_enable = false;
		m_b_exist_tls_enable = false;
		m_b_exist_server_port = false;
	}
private:
	std::wstring m_s_name;
	std::wstring m_s_version;
	std::wstring m_s_date;
	bool m_b_log_enable;
	bool m_b_tls_enable;
	int m_n_server_port;

	bool m_b_exist_name;
	bool m_b_exist_version;
	bool m_b_exist_date;
	bool m_b_exist_log_enable;
	bool m_b_exist_tls_enable;
	bool m_b_exist_server_port;

private:
	cfile_coffee_manager_ini(const cfile_coffee_manager_ini&) = delete;
	cfile_coffee_manager_ini& operator=(const cfile_coffee_manager_ini&) = delete;
};
