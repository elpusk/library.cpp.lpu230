#pragma once

#include <mp_type.h>
#include <mp_coffee.h>

/**
* shared global variable area.
* these variables are defined in share.cpp 
*/

class cdef_const {
public:
	~cdef_const() {}
public:
	static std::wstring get_certificate_file()
	{
		static std::wstring s_certificate_file = _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH;
		return s_certificate_file;
	}
	static std::wstring get_private_key_file()
	{
		static std::wstring s_private_key_file = _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH;
		return s_private_key_file;
	}
	static std::wstring get_log_folder_except_backslash()
	{
		static std::wstring s_log_folder_except_backslash = _mp::_coffee::CONST_S_LOG_DIR_EXCEPT_BACKSLASH;
		return s_log_folder_except_backslash;
	}

	static std::wstring get_root_folder_except_backslash()
	{
		static std::wstring s_root_folder_except_backslash = _mp::_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH;
		return s_root_folder_except_backslash;
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