#pragma once

#include <string>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <mp_coffee.h>
#include <mp_cfile.h>

namespace _mp{

    /**
    * //the defined path
    */
    class ccoffee_path
    {
    public:
		static std::wstring get_virtual_path_of_temp_rom_file_of_session(unsigned long n_session)
		{
#ifdef _WIN32
			std::wstring s_file(L"temp\\temp_fw_se_");
#else
			std::wstring s_file(L"temp/temp_fw_se_");
#endif //_WIN32
			s_file += std::to_wstring((unsigned long)n_session);
			s_file += L".rom";
			return s_file;
		}
		static std::wstring get_path_of_virtual_drive_root_except_backslash()
		{
			std::wstring s_file;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s_file = _mp::cfile::get_path_ProgramData();
			s_file += L"\\elpusk\\00000006\\coffee_manager\\root";
#else
			s_file = _coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH;
#endif
			return s_file;
		}

		static std::wstring get_path_of_coffee_mgmt_ini_file()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramData();
			s += L"\\elpusk\\00000006\\coffee_manager\\elpusk-hid-d";
#else
			s = _mp::_coffee::CONST_S_COFFEE_MGMT_INI_DIR_EXCEPT_BACKSLASH;
#endif

#ifdef _WIN32
			s += L"\\elpusk-hid-d.json";
#else
			s += L"/elpusk-hid-d.json";
#endif //_WIN32
			return s;
		}

		static std::wstring get_path_of_coffee_svr_ini_file()
		{
			std::wstring s;

#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramData();
			s += L"\\elpusk\\00000006\\coffee_manager\\coffee_service";
#else
			s = _mp::_coffee::CONST_S_COFFEE_SVR_INI_DIR_EXCEPT_BACKSLASH;
#endif

#ifdef _WIN32
			s += L"\\coffee_service.json";
#else
			s += L"/coffee_service.json";
#endif //_WIN32
			return s;
		}

		static std::wstring get_path_of_coffee_lpu237_dll_ini_file()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramData();
			s += L"\\elpusk\\00000006\\coffee_manager\\tg_lpu237_dll";
#else
			s = _mp::_coffee::CONST_S_COFFEE_LPU237_MSR_DLL_INI_DIR_EXCEPT_BACKSLASH;
#endif

#ifdef _WIN32
			s += L"\\tg_lpu237_dll.ini";
#else
			s += L"/tg_lpu237_dll.ini";
#endif //_WIN32
			return s;
		}

		static std::wstring get_path_of_coffee_lpu237_ibutton_ini_file()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramData();
			s += L"\\elpusk\\00000006\\coffee_manager\\tg_lpu237_ibutton";
#else
			s = _mp::_coffee::CONST_S_COFFEE_LPU237_IBUTTON_DLL_INI_DIR_EXCEPT_BACKSLASH;
#endif
			//
#ifdef _WIN32
			s += L"\\tg_lpu237_ibutton.ini";
#else
			s += L"/tg_lpu237_ibutton.ini";
#endif //_WIN32
			return s;
		}

		static std::wstring get_path_of_coffee_logs_root_folder_except_backslash()
		{
			std::wstring s;
#ifdef _WIN32
			s = _mp::cfile::get_path_ProgramData();
			s += L"\\elpusk";
#else
			if (geteuid() == 0) {
				// current user is root.
				s = _mp::_coffee::CONST_S_LOGS_ROOT_DIR_EXCEPT_BACKSLASH;
			}
			else {
				//expand '~' to home directory
				auto _s = _mp::cstring::get_mcsc_from_unicode(_mp::_coffee::CONST_S_NOT_ROOT_USER_LOGS_ROOT_DIR_EXCEPT_BACKSLASH);
				s = _mp::cstring::get_unicode_from_mcsc(_mp::cfile::expand_home_directory_in_linux(_s));
			}
#endif //_WIN32
			return s;
		}

		static std::wstring get_abs_full_path_of_certificate()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramData();
			s += L"\\elpusk\\00000006\\coffee_manager\\data\\server\\coffee_server.crt";
#else
			s = _mp::_coffee::CONST_S_CERT_ABS_FULL_PATH; // win debug & linux

#endif //_WIN32
			return s;
		}

		static std::wstring get_abs_full_path_of_private_key()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			s = _mp::cfile::get_path_ProgramData();
			s += L"\\elpusk\\00000006\\coffee_manager\\data\\server\\coffee_server.key";
#else
			s = _mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH; // win debug
#endif
			return s;
		}

		static std::wstring get_abs_full_path_of_coffee_mgmt()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramFiles();
			s += L"\\elpusk\\00000006\\coffee_manager\\bin\\elpusk-hid-d.exe";
#else
			s = _mp::_coffee::CONST_S_MGMT_ABS_FULL_PATH; // win debug & linux

#endif //_WIN32
			return s;
		}

		static std::wstring get_path_of_coffee_mgmt_folder_except_backslash()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramFiles();
			s += L"\\elpusk\\00000006\\coffee_manager\\bin";
#else
			s = _mp::_coffee::CONST_S_DIR_MGMT_EXCEPT_BACKSLASH; // win debug & linux

#endif //_WIN32
			return s;
		}

		static std::wstring get_abs_full_path_of_rom_dll()
		{
			std::wstring s_rom_abs_full_path;
			
			do {
				s_rom_abs_full_path = _mp::_coffee::CONST_S_DIR_DLL_EXCEPT_BACKSLASH;
#ifdef _DEBUG
#ifdef _WIN32
				//debug win
				s_rom_abs_full_path += L"\\tg_rom.dll";
#else
				//debug linux 64
				s_rom_abs_full_path += L"/libtg_rom.so";
#endif
#else
#ifdef _WIN32
				//release win
#ifdef _WIN64
				// x64 Windows
				s_rom_abs_full_path += L"\\x64\\tg_rom.dll";
#elif defined(_M_IX86)
				// x86 32 bits Win
				s_rom_abs_full_path += L"\\x86\\tg_rom.dll";
#else
#endif
#else
				//release linux 64
				s_rom_abs_full_path += L"/libtg_rom.so";
#endif //_WIN32
#endif //_DEBUG
			} while (false);
			return s_rom_abs_full_path;
		}
	};
}