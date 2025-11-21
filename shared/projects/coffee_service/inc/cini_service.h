#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <ctime>

#include <boost/json.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <mp_coffee_path.h>
#include <mp_cstring.h>

class cini_service
{
public:
	static cini_service& get_instance()
	{
		static cini_service obj;
		return obj;
	}

	virtual ~cini_service()
	{
	}

	bool load_definition_file(const std::wstring& s_file = L"")
	{
		bool b_result(false);
		do {

			//load ini .json file
			std::wstring ws_path_ini(_mp::ccoffee_path::get_path_of_coffee_svr_ini_file());
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
			m_b_exist_log_days_to_keep = false;

			m_b_exist_process_check_interval_sec = false;
			m_b_exist_command_line = false;
			m_b_exist_working_directory = false;

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
				m_s_name = L"coffee_service";
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

			m_s_description = L"";
			if (obj.contains("description")) {
				std::string s_description = obj["description"].as_string().c_str();
				m_s_description = _mp::cstring::get_unicode_from_mcsc(s_description);
				m_b_exist_description = true;
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

			m_ll_log_days_to_keep = 3; // default 3 days
			if (obj.contains("log_days")) {
				int n_days = obj["log_days"].as_int64();
				if (n_days > 0) {
					m_ll_log_days_to_keep = (unsigned long long)n_days;
					m_b_exist_log_days_to_keep = true;
				}
				else {
					m_ll_log_days_to_keep = 3; // default is 3 days.
				}
			}
			//

			m_n_process_check_interval_sec = _mp::_coffee::CONST_N_MGMT_ALIVE_CHECK_INTERVAL_SEC;
			if (obj.contains("process_check_interval_sec")) {
				int n_data = obj["process_check_interval_sec"].as_int64();
				if (n_data > 0) {
					m_n_process_check_interval_sec = (UINT)n_data;
					m_b_exist_process_check_interval_sec = true;
				}
			}

			std::vector<wchar_t> v_expanded_path(MAX_PATH,0);

			if( obj.contains("command_line")) {
				std::string s_command_line = obj["command_line"].as_string().c_str();
				m_s_command_line = _mp::cstring::get_unicode_from_mcsc(s_command_line);
				m_b_exist_command_line = true;
				//
#ifdef _WIN32
				// expand environment variables in command line
				UINT n_len = ExpandEnvironmentStringsW(m_s_command_line.c_str(), &v_expanded_path[0], (DWORD)v_expanded_path.size());
				if(n_len == 0 || n_len > v_expanded_path.size()) {
					// if the expanded string is too long, use default coffee management path
					m_s_command_line = _mp::ccoffee_path::get_abs_full_path_of_coffee_mgmt();
				}
				else {
					m_s_command_line = std::wstring(&v_expanded_path[0]);
				}
#endif
			}
			else {
				m_s_command_line = _mp::ccoffee_path::get_abs_full_path_of_coffee_mgmt();
			}

			if( obj.contains("working_directory")) {
				std::string s_working_directory = obj["working_directory"].as_string().c_str();
				m_s_working_directory = _mp::cstring::get_unicode_from_mcsc(s_working_directory);
				m_b_exist_working_directory = true;
				//
#ifdef _WIN32
				v_expanded_path.resize(v_expanded_path.size(), 0);
				// expand environment variables in command line
				UINT n_len = ExpandEnvironmentStringsW(m_s_working_directory.c_str(), &v_expanded_path[0], (DWORD)v_expanded_path.size());
				if (n_len == 0 || n_len > v_expanded_path.size()) {
					// if the expanded string is too long, use default coffee management path
					m_s_working_directory = _mp::ccoffee_path::get_path_of_coffee_mgmt_folder_except_backslash();
				}
				else {
					m_s_working_directory = std::wstring(&v_expanded_path[0]);
				}
#endif

			}
			else {
				m_s_working_directory = _mp::ccoffee_path::get_path_of_coffee_mgmt_folder_except_backslash();
			}

			//
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
	
	unsigned long long get_process_check_interval_sec() const
	{
		return (unsigned long long)m_n_process_check_interval_sec;
	}
	bool get_process_check_interval_sec(unsigned long long& n_sec) const
	{
		n_sec = (unsigned long long)m_n_process_check_interval_sec;
		return m_b_exist_process_check_interval_sec;
	}
	std::wstring get_command_line() const
	{
		return m_s_command_line;
	}
	bool get_command_line(std::wstring& s_command_line) const
	{
		s_command_line = m_s_command_line;
		return m_b_exist_command_line;
	}
	std::wstring get_working_directory() const
	{
		return m_s_working_directory;
	}
	bool get_working_directory(std::wstring& s_working_directory) const
	{
		s_working_directory = m_s_working_directory;
		return m_b_exist_working_directory;
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
					<< L"[:] Process Check Interval(default): " << m_n_process_check_interval_sec << L" sec\n"
					<< L"[:] Command Line(default): " << m_s_command_line << L"\n"
					<< L"[:] Working Directory(default): " << m_s_working_directory << L"\n";
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
			if(!m_b_exist_process_check_interval_sec) {
				ss << L"[:] Process Check Interval(default): " << m_n_process_check_interval_sec << L" sec\n";
			}
			else {
				ss << L"[:] Process Check Interval: " << m_n_process_check_interval_sec << L" sec\n";
			}
			if (!m_b_exist_command_line) {
				ss << L"[:] Command Line(default): " << m_s_command_line << L"\n";
			}
			else {
				ss << L"[:] Command Line: " << m_s_command_line << L"\n";
			}
			if (!m_b_exist_working_directory) {
				ss << L"[:] Working Directory(default): " << m_s_working_directory << L"\n";
			}
			else {
				ss << L"[:] Working Directory: " << m_s_working_directory << L"\n";
			}
			
		} while (false);

		return ss.str();
	}
private:
	cini_service()
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
		m_n_process_check_interval_sec = _mp::_coffee::CONST_N_MGMT_ALIVE_CHECK_INTERVAL_SEC;
		m_b_exist_process_check_interval_sec = false;

		m_s_command_line = _mp::ccoffee_path::get_abs_full_path_of_coffee_mgmt();
		m_b_exist_command_line = false;

		m_s_working_directory = _mp::ccoffee_path::get_path_of_coffee_mgmt_folder_except_backslash();
		m_b_exist_working_directory = false;
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

	// process part
	int m_n_process_check_interval_sec; // how often to check the process status in seconds
	bool m_b_exist_process_check_interval_sec;

	std::wstring m_s_command_line; // command line to start the process
	bool m_b_exist_command_line;
	
	std::wstring m_s_working_directory; // working directory for the process
	bool m_b_exist_working_directory;

private:
	cini_service(const cini_service&) = delete;
	cini_service& operator=(const cini_service&) = delete;
};
