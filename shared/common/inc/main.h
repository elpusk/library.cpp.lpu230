#pragma once

#include <mp_type.h>
#include <mp_coffee.h>

/**
* shared global variable area.
* these variables are defined in share.cpp 
*/

class cdef_const {
public:
	typedef enum : int {
		exit_error_not_supported = 200,
		exit_error_daemonize = 250,
		exit_error_already_running = 251,
		exit_error_create_ctl_pipe = 252,
		exit_error_start_server = 253,
		exit_error_get_ctl_pipe = 254,
		exit_error_create_install_cert = 255,
		exit_error_remove_cert = 256,

		exit_info_ctl_pipe_requst_terminate = 300
	}type_exit_code;

public:
	~cdef_const() {}
public:
	static std::wstring get_certificate_file()
	{
		static std::wstring s = _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH;
		return s;
	}
	static std::wstring get_private_key_file()
	{
		static std::wstring s = _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH;
		return s;
	}
	static std::wstring get_log_folder_except_backslash()
	{
		static std::wstring s = _mp::_coffee::CONST_S_LOG_DIR_EXCEPT_BACKSLASH;
		return s;
	}

	static std::wstring get_root_folder_except_backslash()
	{
		static std::wstring s = _mp::_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH;
		return s;
	}

	static std::wstring get_pid_file_full_path()
	{
#ifdef _WIN32
		static std::wstring s = L"";
#else	// LINUX ONLY
		static std::wstring s = _mp::_coffee::CONST_S_PID_FILE_FULL_PATH;
#endif //_WIN32
		return s;
	}

private:
	cdef_const();
	cdef_const(const cdef_const&);
	cdef_const& operator=(const cdef_const&);
};

extern std::wstring gs_certificate_file;
extern std::wstring gs_private_key_file;

extern std::wstring gs_log_folder_except_backslash;
extern std::wstring gs_root_folder_except_backslash;


/**
 * run the wss server.
*/
int main_wss(const _mp::type_set_wstring &set_parameters);

/**
 * run trace console.
 */
int main_trace(const _mp::type_set_wstring &set_parameters);

/**
 * generate & install certificate 
 */
int main_cert(const _mp::type_set_wstring &set_parameters);

/**
 * remove certificate.
 */
int main_remove_cert(const _mp::type_set_wstring &set_parameters);


/**
 * remove all installed file and paths.
 */
int main_remove_all(const _mp::type_set_wstring& set_parameters);