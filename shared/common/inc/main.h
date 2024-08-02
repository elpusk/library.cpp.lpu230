#pragma once

#include <mp_type.h>

/**
* shared global variable area.
* these variables are defined in share.cpp 
*/

class cdef_const {
public:
	~cdef_const() {}
public:
#ifdef _WIN32
	static std::wstring get_certificate_file()
	{
		static std::wstring s_certificate_file = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.crt";
		return s_certificate_file;
	}
	static std::wstring get_private_key_file()
	{
		static std::wstring s_private_key_file = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.key";
		return s_private_key_file;
	}
	static std::wstring get_log_folder_except_backslash()
	{
		static std::wstring s_log_folder_except_backslash = L"C:\\ProgramData\\Elpusk\\00000006\\elpusk-hid-d\\log";
		return s_log_folder_except_backslash;
	}

	static std::wstring get_root_folder_except_backslash()
	{
		static std::wstring s_root_folder_except_backslash = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\win\\ProgramData\\elpusk\\00000006\\vroot";
		return s_root_folder_except_backslash;
	}
#else
	static std::wstring get_certificate_file()
	{
		static std::wstring s_certificate_file = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.crt";
		return s_certificate_file;
	}
	static std::wstring get_private_key_file()
	{
		static std::wstring s_private_key_file = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.key";
		return s_private_key_file;
	}
	static std::wstring get_log_folder_except_backslash()
	{
		static std::wstring s_log_folder_except_backslash = L"/home/tester/fordebug/var/log/elpusk/00000006/elpusk-hid-d";
		return s_log_folder_except_backslash;
	}

	static std::wstring get_root_folder_except_backslash()
	{
		static std::wstring s_root_folder_except_backslash = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/vroot";
		return s_root_folder_except_backslash;
	}
#endif //_WIN32

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