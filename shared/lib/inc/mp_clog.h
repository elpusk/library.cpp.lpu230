#pragma once

#include <mp_type.h>

#include <memory>
#include <string>
#include <cstdio>
#include <iostream>
#include <sstream>

#include<boost/filesystem.hpp>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <mutex>
#include <thread>

#include <mp_coperation.h>
#include <mp_cfile.h>
#include <mp_cconvert.h>
#include <mp_cnamed_pipe_.h>
#include <mp_cstring.h>

namespace _mp
{
	class clog
	{
	public:
		typedef	std::shared_ptr< clog>		type_ptr_log;

	public:

		/**
		* for only debugging vesion
		* Independent tracing by stdio
		*/
		static void Trace(const wchar_t* s_fmt, ...)
		{
#ifdef _DEBUG
			static std::mutex _m;

			do {
				std::lock_guard<std::mutex>	lock(_m);
				//
				va_list ap;
				va_start(ap, s_fmt);

				do {
					if (s_fmt == NULL)
						continue;
					//
					vwprintf(s_fmt, ap);
				} while (false);

				va_end(ap);

			} while (false);
#endif // _DEBUG
		}

		static void TraceHex(const type_v_buffer& v_data, const std::wstring& s_delimiter = L"")
		{
#ifdef _DEBUG
			static std::mutex _m;
			do {
				std::lock_guard<std::mutex>	lock(_m);

				std::wstring s = cconvert::hex_string_from_binary(v_data, s_delimiter);
				if (s.empty()) {
					continue;
				}

				std::wcout << s;
			} while (false);
#endif // _DEBUG
		}

		/**
		* get client named pipe of tracing system.
		*/
		static cnamed_pipe::type_ptr get_trace_client(const std::string & s_pipe_name_of_trace)
		{
			cnamed_pipe::type_ptr ptr = std::make_shared<_mp::cnamed_pipe>(s_pipe_name_of_trace, false);
			return ptr;
		}
	public:
		static clog & get_instance()
		{
			static clog obj;
			return obj;
		}
	
	public://main methods
		///////////////////////////////////////
		// for tracing system
		/**
		* @brief create or remove tracing server bu owner.
		* @param s_pipe_name_of_trace - the name of trace pipe.
		* @param b_enable : true(enable trace), false(disable trace).
		* @param b_owner : true( owner of trace pipe , use in server), false(use in client)
		*/
		bool enable_trace(const std::string& s_pipe_name_of_trace, bool b_enable, bool b_owner = true)
		{
			bool b_result(false);

			do {
				std::lock_guard<std::mutex>	lock(m_mutex_trace);

				if (!b_enable) {
					m_ptr_track_pipe.reset();
					b_result = true;
					continue;
				}
				//enable is true from here.
				if (m_ptr_track_pipe) {
					b_result = true;
					continue;
				}

				if (s_pipe_name_of_trace.empty()) {
					continue;
				}
				m_ptr_track_pipe = std::make_shared<_mp::cnamed_pipe>(s_pipe_name_of_trace, b_owner);
				if (!m_ptr_track_pipe->is_ini()) {
					m_ptr_track_pipe.reset();
					continue;
				}
				m_b_trace = b_enable;
			} while (false);
			return b_result;
		}

		void trace(const wchar_t* s_fmt, ...)
		{
			static uint32_t n_cont(0);

			do {
				std::lock_guard<std::mutex>	lock(m_mutex_trace);
				//
				if (s_fmt == NULL) {
					continue;
				}
				++n_cont;
				std::wostringstream ss_out;
#ifdef _WIN32
				DWORD pid = GetCurrentProcessId();
#else
				pid_t pid = getpid();
#endif
				ss_out << std::setfill(L'0') << std::setw(sizeof(n_cont) * 2) << std::hex << n_cont << L"[PID:" << pid << L"] " << s_fmt;
				std::wstring s = ss_out.str();
				const wchar_t* ps_fmt = s.c_str();

				va_list ap;
				va_start(ap, s_fmt);

				do {
					if (!m_ptr_track_pipe) {
						continue;
					}
					//
					type_v_ws_buffer v(
						m_ptr_track_pipe->get_max_size_of_one_message()/sizeof(wchar_t),
						0
					);
					if (v.size() == 0) {
						continue;
					}
#ifdef _DEBUG
					va_list ap_cp;
					va_copy(ap_cp, ap);
					//
					vswprintf(&v[0], v.size(), ps_fmt, ap);
					m_ptr_track_pipe->write(&v[0]);

					vwprintf(ps_fmt, ap_cp);
					va_end(ap_cp);
#else
					vswprintf(&v[0], v.size(), ps_fmt, ap);
					m_ptr_track_pipe->write(&v[0]);
#endif
				} while (false);

				va_end(ap);

			} while (false);
		}
		void trace_hex(const type_v_buffer& v_data, const std::wstring& s_delimiter = L"")
		{
			do {
				std::lock_guard<std::mutex>	lock(m_mutex_trace);
				if (!m_ptr_track_pipe) {
					continue;
				}

				std::wstring s = cconvert::hex_string_from_binary(v_data, s_delimiter);
				if (s.empty()) {
					continue;
				}
				m_ptr_track_pipe->write(s);
#ifdef _DEBUG
				std::wcout << s;
#endif
			} while (false);
		}


		///////////////////////////////////////
		// foir logging system
		/**
		* configuration logging system
		* Don't creates log() function.
		*/
		bool config(
			const std::wstring & s_in_folder_path 
			, size_t n_employee_id
			, const std::wstring& s_program_name
			, const std::wstring & s_module_name
			, const std::wstring& s_in_prefix_log_file
		)
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			return _config(s_in_folder_path, n_employee_id, s_program_name, s_module_name, s_in_prefix_log_file);
		}

		void enable(bool b_enable = true)
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			m_b_enable = b_enable;
		}

		void enable_and_create_log_folder(bool b_enable = true)
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			m_b_enable = b_enable;
			//
			if (!m_s_log_folder_path.empty() && m_b_enable) {
				cfile::is_exist_folder(m_s_log_folder_path, true);
			}
		}

		bool is_enable()
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			return m_b_enable;
		}

		void enable_pid(bool b_enable = true)
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			m_b_enable_pid = b_enable;
		}

		void enable_time(bool b_enable = true)
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			m_b_enable_time = b_enable;
		}
		void enable_systick(bool b_enable = true)
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			m_b_enable_systick = b_enable;
		}

		void log_and_set_format(bool b_enable_time, bool b_enable_systick, bool b_enable_pid, const std::wstring& s_message)
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			_log_and_set_format(b_enable_time, b_enable_systick, b_enable_pid, s_message);
		}
		void log_and_set_format(bool b_enable_time, bool b_enable_systick, bool b_enable_pid, const std::string& s_message)
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			std::wstring sw_message = cstring::get_unicode_from_mcsc(s_message);
			_log_and_set_format(b_enable_time, b_enable_systick, b_enable_pid, sw_message);
		}

		void log(bool b_enable_time, bool b_enable_systick, bool b_enable_pid, const std::wstring & s_message)
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			_log(b_enable_time, b_enable_systick, b_enable_pid, s_message);
		}
		void log(bool b_enable_time, bool b_enable_systick, bool b_enable_pid, const std::string& s_message)
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			std::wstring sw_message = cstring::get_unicode_from_mcsc(s_message);
			_log(b_enable_time, b_enable_systick, b_enable_pid, sw_message);
		}

		void log_string(const wchar_t* s_msg)
		{
			if (s_msg) {
				log_fmt(L"%ls", s_msg);
			}
		}
		void log_string(const char* s_msg)
		{
			if (s_msg) {
				std::wstring sw_message = cstring::get_unicode_from_mcsc(std::string(s_msg));
				log_fmt(L"%ls", sw_message.c_str());
			}
		}

		// 

		/**
		* log_fmt : if log(const wchar_t *s_fmt, ...) is error ,use log_fmt() !
		*/
		void log_fmt(const wchar_t* s_fmt, ...)
		{
			do {
				std::lock_guard<std::mutex>	lock(m_mutex_log);
				if (!m_b_enable)
					continue;
				if (m_s_log_file_path.empty())
					continue;
				//
				va_list ap;
				va_start(ap, s_fmt);

				do {
					if (s_fmt == NULL)
						continue;
#ifdef _WIN32
					size_t n_len = _vscwprintf(s_fmt, ap) + 1; // for '\0'

					std::shared_ptr<type_v_ws_buffer> ptr_v_ws_buffer(std::make_shared<type_v_ws_buffer>(n_len, 0));
					if (!ptr_v_ws_buffer)
						continue;
					if (vswprintf_s(&(*ptr_v_ws_buffer)[0], ptr_v_ws_buffer->size(), s_fmt, ap) <= 0)
						continue;
#else
					std::shared_ptr<type_v_ws_buffer> ptr_v_ws_buffer(std::make_shared<type_v_ws_buffer>(2048, 0));
					if (!ptr_v_ws_buffer)
						continue;
					if (vswprintf(&(*ptr_v_ws_buffer)[0], ptr_v_ws_buffer->size(), s_fmt, ap) <= 0)
						continue;
#endif
					//
					_write_file(std::wstring(&(*ptr_v_ws_buffer)[0]), m_b_enable_time, m_b_enable_systick, m_b_enable_pid);

				} while (false);

				va_end(ap);

			} while (false);
		}
		/**
		* log_fmt : if log(const char *s_fmt, ...) is error ,use log_fmt() !
		*/
		void log_fmt(const char* s_fmt, ...)
		{
			do {
				std::lock_guard<std::mutex>	lock(m_mutex_log);
				if (!m_b_enable)
					continue;
				if (m_s_log_file_path.empty())
					continue;
				//
				va_list ap;
				va_start(ap, s_fmt);

				do {
					if (s_fmt == NULL)
						continue;
#ifdef _WIN32
					size_t n_len = _vscprintf(s_fmt, ap) + 1; // for '\0'

					std::shared_ptr<type_v_s_buffer> ptr_v_s_buffer(std::make_shared<type_v_s_buffer>(n_len, 0));
					if (!ptr_v_s_buffer)
						continue;
					if (vsprintf_s(&(*ptr_v_s_buffer)[0], ptr_v_s_buffer->size(), s_fmt, ap) <= 0)
						continue;
#else
					std::shared_ptr<type_v_s_buffer> ptr_v_s_buffer(std::make_shared<type_v_s_buffer>(2048, 0));
					if (!ptr_v_s_buffer)
						continue;
					if (vsnprintf(&(*ptr_v_s_buffer)[0], ptr_v_s_buffer->size(), s_fmt, ap) <= 0)
						continue;
#endif
					//
					std::wstring sw_message = cstring::get_unicode_from_mcsc(std::string(&(*ptr_v_s_buffer)[0]));
					_write_file(sw_message, m_b_enable_time, m_b_enable_systick, m_b_enable_pid);

				} while (false);

				va_end(ap);

			} while (false);
		}

		void log_fmt_in_debug_mode(const wchar_t* s_fmt, ...)
		{
#ifdef _DEBUG
			do {
				std::lock_guard<std::mutex>	lock(m_mutex_log);
				if (!m_b_enable)
					continue;
				if (m_s_log_file_path.empty())
					continue;
				//
				va_list ap;
				va_start(ap, s_fmt);

				do {
					if (s_fmt == NULL)
						continue;
#ifdef _WIN32
					size_t n_len = _vscwprintf(s_fmt, ap) + 1; // for '\0'

					std::shared_ptr<type_v_ws_buffer> ptr_v_ws_buffer(std::make_shared<type_v_ws_buffer>(n_len, 0));
					if (!ptr_v_ws_buffer)
						continue;
					if (vswprintf_s(&(*ptr_v_ws_buffer)[0], ptr_v_ws_buffer->size(), s_fmt, ap) <= 0)
						continue;
#else
					std::shared_ptr<type_v_ws_buffer> ptr_v_ws_buffer(std::make_shared<type_v_ws_buffer>(2048, 0));
					if (!ptr_v_ws_buffer)
						continue;
					if (vswprintf(&(*ptr_v_ws_buffer)[0], ptr_v_ws_buffer->size(), s_fmt, ap) <= 0)
						continue;
#endif
					//
					_write_file(std::wstring(&(*ptr_v_ws_buffer)[0]), m_b_enable_time, m_b_enable_systick, m_b_enable_pid);

				} while (false);

				va_end(ap);

			} while (false);
#endif //_DEBUG
		}
		/**
		* log_fmt : if log(const char *s_fmt, ...) is error ,use log_fmt() !
		*/
		void log_fmt_in_debug_mode(const char* s_fmt, ...)
		{
#ifdef _DEBUG
			do {
				std::lock_guard<std::mutex>	lock(m_mutex_log);
				if (!m_b_enable)
					continue;
				if (m_s_log_file_path.empty())
					continue;
				//
				va_list ap;
				va_start(ap, s_fmt);

				do {
					if (s_fmt == NULL)
						continue;
#ifdef _WIN32
					size_t n_len = _vscprintf(s_fmt, ap) + 1; // for '\0'

					std::shared_ptr<type_v_s_buffer> ptr_v_s_buffer(std::make_shared<type_v_s_buffer>(n_len, 0));
					if (!ptr_v_s_buffer)
						continue;
					if (vsprintf_s(&(*ptr_v_s_buffer)[0], ptr_v_s_buffer->size(), s_fmt, ap) <= 0)
						continue;
#else
					std::shared_ptr<type_v_s_buffer> ptr_v_s_buffer(std::make_shared<type_v_s_buffer>(2048, 0));
					if (!ptr_v_s_buffer)
						continue;
					if (vsnprintf(&(*ptr_v_s_buffer)[0], ptr_v_s_buffer->size(), s_fmt, ap) <= 0)
						continue;
#endif
					//
					std::wstring sw_message = cstring::get_unicode_from_mcsc(std::string(&(*ptr_v_s_buffer)[0]));
					_write_file(sw_message, m_b_enable_time, m_b_enable_systick, m_b_enable_pid);

				} while (false);

				va_end(ap);

			} while (false);
#endif //_DEBUG
		}
		void log_data(const type_v_buffer & v_data, const std::wstring & s_prefix = std::wstring(), const std::wstring & s_postfix = std::wstring())
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			_log_data(v_data, s_prefix, s_postfix);
		}
		void log_data_in_debug_mode(const type_v_buffer& v_data, const std::wstring& s_prefix = std::wstring(), const std::wstring& s_postfix = std::wstring())
		{
#ifdef _DEBUG
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			_log_data(v_data, s_prefix, s_postfix);
#endif //_DEBUG
		}

		void log_ic_request(const type_v_buffer & v_request, const std::wstring & s_prefix = std::wstring(), const std::wstring & s_postfix = std::wstring())
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			_log_ic_request(v_request, s_prefix, s_postfix);
		}

		void log_ic_response(const type_v_buffer & v_response, const std::wstring & s_prefix = std::wstring(), const std::wstring & s_postfix = std::wstring())
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			_log_ic_response(v_response, s_prefix, s_postfix);
		}

		void log_msr_data(const type_v_buffer & v_msr, const std::wstring & s_prefix = std::wstring(), const std::wstring & s_postfix = std::wstring())
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			_log_msr_data(v_msr, s_prefix, s_postfix);
		}

	public://log file management methods
		unsigned long long get_log_folder_size(bool b_except_sub_folder = true)
		{
			unsigned long long ll_size(0);

			do {
				std::lock_guard<std::mutex>	lock(m_mutex_log);
				if (!m_b_ini)
					continue;
				//
				if (b_except_sub_folder)
					ll_size = cfile::get_folder_size(m_s_log_folder_path, cfile::folder_area_current_folder);
				else
					ll_size = cfile::get_folder_size(m_s_log_folder_path, cfile::folder_area_all_sub_folder);
			} while (false);
			return ll_size;
		}

		bool remove_log_files(bool b_except_sub_folder = true) {
			bool b_result = false;
			do {
				std::lock_guard<std::mutex> lock(m_mutex_log);
				if (!m_b_ini)
					continue;

				std::list<std::wstring> list_found;
				boost::filesystem::path log_folder_path = m_s_log_folder_path;

				// Find only files
				if (boost::filesystem::exists(log_folder_path) && boost::filesystem::is_directory(log_folder_path)) {
					boost::filesystem::recursive_directory_iterator end_iter;
					for (boost::filesystem::recursive_directory_iterator dir_iter(log_folder_path); dir_iter != end_iter; ++dir_iter) {
						if (boost::filesystem::is_regular_file(dir_iter->status())) {
							list_found.push_back(dir_iter->path().wstring());
						}
					}
				}

				b_result = true;

				// Delete only files
				for (const auto& s_path : list_found) {
					if (m_s_log_file_path.compare(s_path) != 0) {
						boost::filesystem::remove(boost::filesystem::path(s_path));
					}
				}

				if (!b_result)
					continue;

				if (b_except_sub_folder)
					continue;

				std::list<std::wstring> list_folder_found;
				if (boost::filesystem::exists(log_folder_path) && boost::filesystem::is_directory(log_folder_path)) {
					for (
						boost::filesystem::recursive_directory_iterator dir_iter(log_folder_path);
						dir_iter != boost::filesystem::recursive_directory_iterator();
						++dir_iter) 
					{
						if (boost::filesystem::is_directory(dir_iter->status())) {
							list_folder_found.push_back(dir_iter->path().wstring());
						}
					}
				}

				// Sort folders for deletion
#ifdef _WIN32
				list_folder_found.sort([=](const std::wstring& s_1, const std::wstring& s_2) -> bool {
					return std::count(s_1.begin(), s_1.end(), L'\\') > std::count(s_2.begin(), s_2.end(), L'\\');
					});
#else
				list_folder_found.sort([=](const std::wstring& s_1, const std::wstring& s_2) -> bool {
					return std::count(s_1.begin(), s_1.end(), L'/') > std::count(s_2.begin(), s_2.end(), L'/');
					});
#endif

				// Delete folders
				for (const auto& s_path : list_folder_found) {
					boost::filesystem::remove_all(boost::filesystem::path(s_path));
				}
			} while (false);

			return b_result;
		}


		bool remove_log_files_older_then_now_hour(
			unsigned long long ll_limit_hour
			, cfile::type_found_file_time found_file_time_type = cfile::file_time_create
		)
		{
			return _remove_log_files_older_then_now(
				0
				, ll_limit_hour
				, 0
				, 0
				,found_file_time_type
			);
		}

		bool remove_log_files_older_then_now_day(
			unsigned long long ll_limit_day
			, cfile::type_found_file_time found_file_time_type = cfile::file_time_create
		)
		{
			return _remove_log_files_older_then_now(
				ll_limit_day
				, 0
				, 0
				, 0
				, found_file_time_type
			);
		}


	public:
		virtual ~clog()
		{
			m_ptr_track_pipe.reset();
		}
		bool is_ini()
		{ 
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			return m_b_ini;
		}
		std::wstring get_file_path()
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			return m_s_log_file_path;
		}
		std::wstring get_folder_path()
		{
			std::lock_guard<std::mutex>	lock(m_mutex_log);
			return m_s_log_folder_path;
		}

	private:
		clog() : m_b_ini(false), m_n_employee_id(6), 
			m_b_enable(false), m_b_enable_time(true), m_b_enable_systick(true), m_b_enable_pid(true),
			m_b_trace(false)
		{
		}


		/**
		* @brief configuration of log file. if log folder doesn't exist, create it.<br>
		* @param s_in_folder_abs_path_without_backslash - abs path of log file folder.<br>
		* @param n_employee_id - using id , if you don't know, set to zero.
		* @param s_program_name - the name of program, except path and extension. this is summation of all modules.
		* @param s_module_name - the name of module, except path and extension.
		* @param s_in_prefix_log_file - the prefix of log file name
		* @return if folder dosen't exsit, error(failure of create folder).
		*/
		bool _config(
			const std::wstring& s_in_folder_abs_path_without_backslash, 
			size_t n_employee_id, 
			const std::wstring& s_program_name,
			const std::wstring& s_module_name,
			const std::wstring& s_in_prefix_log_file
		)
		{
#ifdef _WIN32
			std::wstring s_separator(L"\\");
#else
			std::wstring s_separator(L"/");
#endif
			bool b_result(false);
			do {
				std::wstring s_log_folder_without_backslash;
				if (s_in_folder_abs_path_without_backslash.empty())
					continue;

				s_log_folder_without_backslash = s_in_folder_abs_path_without_backslash;

				if (n_employee_id != (size_t)(-1)) {
					s_log_folder_without_backslash += s_separator;
					std::wstring s_em;
					_mp::cstring::format_c_style(s_em, L"%08u", n_employee_id);
					s_log_folder_without_backslash += s_em;
				}

				if(!s_program_name.empty()) {
					s_log_folder_without_backslash += s_separator;
					s_log_folder_without_backslash += s_program_name;
				}
				if (!s_module_name.empty()) {
					s_log_folder_without_backslash += s_separator;
					s_log_folder_without_backslash += s_module_name;
				}
#ifdef _WIN32
				s_log_folder_without_backslash += s_separator;
				s_log_folder_without_backslash += L"log";
#endif
				

				cfile::is_exist_folder(s_log_folder_without_backslash, true);
				if (!cfile::is_exist_folder(s_log_folder_without_backslash, false)) {
					continue;
				}
				m_s_log_file_path.clear();
				m_b_ini = false;
				std::wstring s_folder_path(s_log_folder_without_backslash);
				std::wstring s_file_path;
				s_file_path = _get_log_file_abs_path(s_folder_path, s_in_prefix_log_file);

				if (s_folder_path.empty())
					continue;
				//
				m_s_log_folder_path = s_folder_path;
				m_s_log_file_path = s_file_path;
				m_n_employee_id = n_employee_id;
				m_b_ini = true;
				b_result = true;
			} while (false);
			return b_result;
		}

		std::wstring _get_log_file_abs_path(const std::wstring & s_in_folder_path_without_backslash, const std::wstring& s_in_prefix_log_file)
		{
			std::wstring s_full_abs_path;

			do {
				if (s_in_folder_path_without_backslash.empty())
					continue;
				//
#ifdef _WIN32
				time_t t;

				struct tm dt = { 0, };
				t = time(NULL);
				localtime_s(&dt, &t);
				DWORD dwStTick = ::GetTickCount();
				DWORD dw_pid = GetCurrentProcessId();

				//
				if (s_in_prefix_log_file.empty()) {
					cstring::format(
						s_full_abs_path,
						L"%ls\\%02d%02d%02d%02d%02d-%09u-pid%09u.txt",
						s_in_folder_path_without_backslash.c_str(),
						dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec,
						dwStTick, dw_pid
					);
				}
				else {
					cstring::format(
						s_full_abs_path,
						L"%ls\\%ls-%02d%02d%02d%02d%02d-%09u-pid%09u.txt",
						s_in_folder_path_without_backslash.c_str(),
						s_in_prefix_log_file.c_str(),
						dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec,
						dwStTick, dw_pid
					);
				}
#else
				time_t t = time(nullptr);
				struct tm* p_dt = NULL;
				p_dt = std::localtime(&t);

				struct timeval tv;
				gettimeofday(&tv, nullptr);
				unsigned long ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

				pid_t pid = getpid();

				//
				if (p_dt) {
					if (s_in_prefix_log_file.empty()) {
						cstring::format(
							s_full_abs_path,
							L"%ls/%02d%02d%02d%02d%02d-%09u-pid%09d.txt",
							s_in_folder_path_without_backslash.c_str(),
							p_dt->tm_mon + 1, p_dt->tm_mday, p_dt->tm_hour, p_dt->tm_min, p_dt->tm_sec,
							ms, pid
						);
					}
					else {
						cstring::format(
							s_full_abs_path,
							L"%ls/%ls-%02d%02d%02d%02d%02d-%09u-pid%09d.txt",
							s_in_folder_path_without_backslash.c_str(),
							s_in_prefix_log_file.c_str(),
							p_dt->tm_mon + 1, p_dt->tm_mday, p_dt->tm_hour, p_dt->tm_min, p_dt->tm_sec,
							ms, pid
						);
					}
				}
#endif

			} while (false);
			return s_full_abs_path;
		}

		void _log_and_set_format(bool b_enable_time, bool b_enable_systick, bool b_enable_pid, const std::wstring& s_message)
		{
			do {
				if (!m_b_enable)
					continue;
				if (m_s_log_file_path.empty())
					continue;
				//
				m_b_enable_time = b_enable_time;
				m_b_enable_systick = b_enable_systick;
				m_b_enable_pid = b_enable_pid;
				_write_file(s_message, b_enable_time, b_enable_systick, b_enable_pid);
			} while (false);
		}

		void _log(bool b_enable_time, bool b_enable_systick, bool b_enable_pid, const std::wstring& s_message)
		{
			do {
				if (!m_b_enable)
					continue;
				if (m_s_log_file_path.empty())
					continue;
				//
				_write_file(s_message, b_enable_time, b_enable_systick, b_enable_pid);
			} while (false);
		}

		void _log_data(const type_v_buffer& v_data, const std::wstring& s_prefix, const std::wstring& s_postfix)
		{
			do {
				if (!m_b_enable)
					continue;
				if (m_s_log_file_path.empty())
					continue;
				//
				std::wstring s_message;
				s_message += s_prefix;

				std::wstring s_data;
				cconvert::hex_string_from_binary(s_data, v_data, L" ");
				s_message += s_data;

				s_message += s_postfix;
				_write_file(s_message, false, false);
			} while (false);
		}

		void _log_ic_request(const type_v_buffer& v_request, const std::wstring& s_prefix, const std::wstring& s_postfix)
		{
			do {
				if (!m_b_enable)
					continue;
				if (m_s_log_file_path.empty())
					continue;
				//
				std::wstring s_message;
				s_message += s_prefix;

				std::wstring s_data;
				cconvert::hex_string_from_binary(s_data, v_request, L" ");
				s_message += _MP_TOOLS_CT_TYPE_IC_REQUEST_PREFIX;
				s_message += s_data;
				s_message += _MP_TOOLS_CT_TYPE_IC_REQUEST_POSTFIX;

				s_message += s_postfix;
				_write_file(s_message, false, false);
			} while (false);
		}

		void _log_ic_response(const type_v_buffer& v_response, const std::wstring& s_prefix, const std::wstring& s_postfix)
		{
			do {
				if (!m_b_enable)
					continue;
				if (m_s_log_file_path.empty())
					continue;
				//
				std::wstring s_message;
				s_message += s_prefix;

				std::wstring s_data;
				cconvert::hex_string_from_binary(s_data, v_response, L" ");
				s_message += _MP_TOOLS_CT_TYPE_IC_RESPONSE_PREFIX;
				s_message += s_data;
				s_message += _MP_TOOLS_CT_TYPE_IC_RESPONSE_POSTFIX;

				s_message += s_postfix;
				_write_file(s_message, false, false);
			} while (false);
		}

		void _log_msr_data(const type_v_buffer& v_msr, const std::wstring& s_prefix, const std::wstring& s_postfix)
		{
			do {
				if (!m_b_enable)
					continue;
				if (m_s_log_file_path.empty())
					continue;
				//
				std::wstring s_message;
				s_message += s_prefix;

				std::wstring s_data;
				cconvert::hex_string_from_binary(s_data, v_msr, L" ");
				s_message += _MP_TOOLS_CT_TYPE_MSR_DATA_PREFIX;
				s_message += s_data;
				s_message += _MP_TOOLS_CT_TYPE_MSR_DATA_POSTFIX;

				s_message += s_postfix;
				_write_file(s_message, false, false);
			} while (false);
		}

		void _write_file(const std::wstring& s_message, bool b_enable_time = true, bool b_enable_systick = true, bool b_enable_pid = true)
		{
			FILE* p_stream = nullptr;

			do {
#ifdef _WIN32
				if (_wfopen_s(&p_stream, m_s_log_file_path.c_str(), L"a+") != 0)
					continue;
#else
				std::string s = cstring::get_mcsc_from_unicode(m_s_log_file_path);
				p_stream = std::fopen(s.c_str(), "a+");
				if (!p_stream) {
					continue;
				}
#endif

				if (b_enable_time) {
#ifdef _WIN32
					time_t t;
					struct tm dt = { 0, };
					t = time(NULL);
					localtime_s(&dt, &t);
					fwprintf(p_stream, L"[%02d-%02d %02d:%02d:%02d] ", dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
#else
					std::time_t t = std::time(nullptr);
					std::tm dt = *std::localtime(&t);
					fwprintf(p_stream, L"[%02d-%02d %02d:%02d:%02d] ", dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
#endif
				}

				if (b_enable_systick) {
#ifdef _WIN32
					DWORD dwStTick = ::GetTickCount();
					fwprintf(p_stream, L"[%09u] ", dwStTick);
#else
					struct timeval tv;
					gettimeofday(&tv, nullptr);
					unsigned long ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;
					fwprintf(p_stream, L"[%09lu] ", ms);
#endif
				}

				if (b_enable_pid) {
#ifdef _WIN32
					DWORD pid = GetCurrentProcessId();
					fwprintf(p_stream, L"[%09u] ", pid);
#else
					pid_t pid = getpid();
					fwprintf(p_stream, L"[%09d] ", pid);
#endif
				}

				if (p_stream) {
					fwprintf(p_stream, L"%ls", s_message.c_str());

					std::fflush(p_stream);
					std::fclose(p_stream);
				}
			} while (false);
		}
	
		bool _remove_log_files_older_then_now(
			unsigned long long ll_limit_day
			, unsigned long long ll_limit_hour
			, unsigned long long ll_limit_minute
			, unsigned long long ll_limit_second
			, cfile::type_found_file_time found_file_time_type = cfile::file_time_create
		)
		{
			bool b_result(false);

			do {
				std::lock_guard<std::mutex>	lock(m_mutex_log);
				if (!m_b_ini)
					continue;
				//
				cfile::type_list_pair_find_data list_pair_found;
				cfile::type_find_folder_area area(cfile::folder_area_current_folder);

				cfile::get_find_file_list(
					list_pair_found
					, m_s_log_folder_path
					, L"*"
					, area);

				b_result = true;

				if (cfile::filtering_find_data(
					list_pair_found
					, found_file_time_type
					, ll_limit_day
					, ll_limit_hour
					, ll_limit_minute
					, ll_limit_second
					, false) == 0)
					continue;//no removed log file.

				//delete only files
				std::for_each(std::begin(list_pair_found), std::end(list_pair_found), [&](const cfile::type_pair_find_data & pair_data) {
					if (!pair_data.first.empty()) {
						if (!cfile::delete_file(pair_data.first)) {
							b_result = false;
						}
					}
					});
			} while (false);
			return b_result;
		}
	private:
		// trackgin is seperated from logging system.
		//for trace system
		std::mutex m_mutex_trace;
		bool m_b_trace;
		cnamed_pipe::type_ptr m_ptr_track_pipe;

		// for logging system
		bool m_b_ini;
		std::wstring m_s_log_file_path;
		std::wstring m_s_log_folder_path;
		std::mutex m_mutex_log;
		size_t m_n_employee_id;
		bool m_b_enable;
		bool m_b_enable_time;
		bool m_b_enable_systick;
		bool m_b_enable_pid;

	private://don't call these methods
		clog(const clog &);
		clog & operator=(const clog &);
	};
}

