#pragma once

#include <string>
#include <filesystem>

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

		/**
		* @brief generate temp rom file name for updating fw.
		* @param n_session - the sesscion number of wss connection.
		* @return wstring - the relative temp-rom file path from virtual drive root path.
		* 
		*	this temp-rom file can be binary file or rom format file.
		* 
		*	( ex session number = 12345678, return - temp/temp_fw_se_12345678.rom )
		*/
		static std::wstring get_virtual_path_of_temp_rom_file_of_session(unsigned long n_session)
		{
			std::filesystem::path p(L"temp");
			p /= std::wstring(L"temp_fw_se_");
			p += (std::to_wstring((unsigned long)n_session) + std::wstring(L".rom"));
			return p.wstring();
		}

		/**
		* @brief get virtual drive root path.
		* @return absolute path of virtual drive root path without directory deliminator \ or /.
		*/
		static std::wstring get_path_of_virtual_drive_root_except_backslash()
		{
			std::wstring s_file;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s_file = _mp::cfile::get_path_ProgramData() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\root");
#else
			s_file = std::wstring(_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH);
#endif
			return s_file;
		}

		/**
		* @brief get virtual drive root path.
		* @return absolute path of virtual drive root path with \ or /.
		*/
		static std::wstring get_path_of_virtual_drive_root_with_backslash()
		{
			std::wstring s_file(ccoffee_path::get_path_of_virtual_drive_root_except_backslash());
			std::filesystem::path p(s_file);

			p /= std::wstring(L"");
			return p.wstring();
		}

		/**
		* @brief get ini file absolute path of elpusk-hid-d.exe or elpusk-hid-d(demon)
		* @return absolute path of elpusk-hid-d.json.
		*/
		static std::wstring get_path_of_coffee_mgmt_ini_file()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramData() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\elpusk-hid-d");
#else
			s = std::wstring(_mp::_coffee::CONST_S_COFFEE_MGMT_INI_DIR_EXCEPT_BACKSLASH);
#endif
			std::filesystem::path p(s);

			p /= std::wstring(L"elpusk-hid-d.json");
			return p.wstring();
		}

		/**
		* @brief get ini file absolute path of coffee_service.exe
		* @return absolute path of coffee_service.json.
		*/
		static std::wstring get_path_of_coffee_svr_ini_file()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramData() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\coffee_service");
#else
			s = std::wstring(_mp::_coffee::CONST_S_COFFEE_SVR_INI_DIR_EXCEPT_BACKSLASH);
#endif
			std::filesystem::path p(s);

			p /= std::wstring(L"coffee_service.json");
			return p.wstring();
		}

		/**
		* @brief get ini file absolute path of tg_lpu237_dll.dll or libtg_lpu237_dll.so
		* @return absolute path of tg_lpu237_dll.ini.
		*/
		static std::wstring get_path_of_coffee_lpu237_dll_ini_file()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramData() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\tg_lpu237_dll");
#else
			s = std::wstring(_mp::_coffee::CONST_S_COFFEE_LPU237_MSR_DLL_INI_DIR_EXCEPT_BACKSLASH);
#endif
			std::filesystem::path p(s);

			p /= std::wstring(L"tg_lpu237_dll.ini");
			return p.wstring();
		}

		/**
		* @brief get ini file absolute path of tg_lpu237_ibutton.dll or libtg_lpu237_ibutton.so
		* @return absolute path of tg_lpu237_ibutton.ini.
		*/
		static std::wstring get_path_of_coffee_lpu237_ibutton_ini_file()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramData() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\tg_lpu237_ibutton");
#else
			s = std::wstring(_mp::_coffee::CONST_S_COFFEE_LPU237_IBUTTON_DLL_INI_DIR_EXCEPT_BACKSLASH);
#endif
			std::filesystem::path p(s);

			p /= std::wstring(L"tg_lpu237_ibutton.ini");
			return p.wstring();
		}

		/**
		* @brief get ini file absolute path of tg_lpu237_fw.dll or libtg_lpu237_fw.so
		* @return absolute path of tg_lpu237_fw.ini.
		*/
		static std::wstring get_path_of_coffee_lpu237_fw_ini_file()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramData() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\tg_lpu237_fw");
#else
			s = std::wstring(_mp::_coffee::CONST_S_COFFEE_LPU237_FW_DLL_INI_DIR_EXCEPT_BACKSLASH);
#endif
			std::filesystem::path p(s);

			p /= std::wstring(L"tg_lpu237_fw.ini");
			return p.wstring();
		}

		/**
		* @brief get root absolute path of log file without deliminator
		* @return 
		* 
		*	windows - L"%ProgramData%\\elpusk";
		* 
		*	linux - run by root user - L"/var/log/elpusk"
		* 
		* 	linux - run by generic user -  absolute path of L"~/.elpusk/log"
		*/
		static std::wstring get_path_of_coffee_logs_root_folder_except_backslash()
		{
			std::wstring s;
#ifdef _WIN32
			s = _mp::cfile::get_path_ProgramData() + std::wstring(L"\\elpusk");
#else
			if (geteuid() == 0) {
				// current user is root.
				s = std::wstring(_mp::_coffee::CONST_S_LOGS_ROOT_DIR_EXCEPT_BACKSLASH);
			}
			else {
				//expand '~' to home directory
				s = _mp::cfile::expand_home_directory_in_linux(std::wstring(_mp::_coffee::CONST_S_NOT_ROOT_USER_LOGS_ROOT_DIR_EXCEPT_BACKSLASH));
			}
#endif //_WIN32
			return s;
		}

		/**
		* @brief get absolute path of wss certificate file
		* @return absolute path
		*/
		static std::wstring get_abs_full_path_of_certificate()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramData() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\data\\server\\coffee_server.crt");
#else
			s = std::wstring(_mp::_coffee::CONST_S_CERT_ABS_FULL_PATH); // win debug & linux

#endif //_WIN32
			return s;
		}

		/**
		* @brief get absolute path of wss private key file
		* @return absolute path
		*/
		static std::wstring get_abs_full_path_of_private_key()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramData() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\data\\server\\coffee_server.key");
#else
			s = std::wstring(_mp::_coffee::CONST_S_PRIVATE_KEY_ABS_FULL_PATH); // win debug
#endif
			return s;
		}

		/**
		* @brief get absolute path of elpusk-hid-d.exe or elpusk-hid-d(demon)
		* @return absolute path.
		*/
		static std::wstring get_abs_full_path_of_coffee_mgmt()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramFiles() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\bin\\elpusk-hid-d.exe");
#else
			s = std::wstring(_mp::_coffee::CONST_S_MGMT_ABS_FULL_PATH); // win debug & linux

#endif //_WIN32
			return s;
		}

		/**
		* @brief get absolute directory path of elpusk-hid-d.exe or elpusk-hid-d(demon) without deliminator.
		* @return absolute path.
		*/
		static std::wstring get_path_of_coffee_mgmt_folder_except_backslash()
		{
			std::wstring s;
#if defined(_WIN32) && !defined(_DEBUG)
			//win release
			s = _mp::cfile::get_path_ProgramFiles() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\bin");
#else
			s = std::wstring(_mp::_coffee::CONST_S_DIR_MGMT_EXCEPT_BACKSLASH); // win debug & linux

#endif //_WIN32
			return s;
		}

		/**
		* @brief get absolute path of lpu230_update.exe or lpu230_update.
		* @return absolute path.
		*/
		static std::wstring get_abs_full_path_of_updater()
		{
			std::wstring s;
#ifdef _WIN32
#ifdef _DEBUG
			//win-debug
			s = std::wstring(_mp::_coffee::CONST_S_DIR_MGMT_EXCEPT_BACKSLASH) + std::wstring(L"\\lpu230_update.exe");
#else
			//win-release
			s = _mp::cfile::get_path_ProgramFiles() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\bin\\lpu230_update.exe");

#endif //_DEBUG
#else
#ifdef _DEBUG
			//linux-debug
			s = std::wstring(_mp::_coffee::CONST_S_DIR_LPU230_UPDATE);
#else
			//linux-release
			s = std::wstring(_mp::_coffee::CONST_S_DIR_MGMT_EXCEPT_BACKSLASH);
			s += std::wstring(L"/lpu230_update");
#endif //_DEBUG

#endif
			return s;

		}

		/**
		* @brief get absolute path of tg_rom.dll or libtg_rom.so.
		* @return absolute path.
		*/
		static std::wstring get_abs_full_path_of_rom_dll()
		{
			std::wstring s;
			
#ifdef _WIN32
			//windows///////////////
#ifdef _WIN64
			//win64
#ifdef _DEBUG
			//win64 + debug
			s = std::wstring(_mp::_coffee::CONST_S_DIR_DLL_EXCEPT_BACKSLASH) + std::wstring(L"\\tg_rom.dll");
#else
			//win64 + release
			s = _mp::cfile::get_path_ProgramFiles() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\dll\\x64\\tg_rom.dll");
#endif
#elif defined(_M_IX86)
			//winx86
#ifdef _DEBUG
			//winx86 + debug
			s = std::wstring(_mp::_coffee::CONST_S_DIR_DLL_EXCEPT_BACKSLASH) + std::wstring(L"\\tg_rom.dll");
#else
			//winx86 + release
			s = _mp::cfile::get_path_ProgramFiles() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\dll\\x86\\tg_rom.dll");
#endif
#else
			//NON support arm
#endif
#else
			//linux x64////////////////////
			s = std::wstring(_mp::_coffee::CONST_S_DIR_DLL_ROM_SO);
#endif
			return s;
		}

		/**
		* @brief get absolute path of dev_lib.dll or libdev_lib.so.
		* @return absolute path.
		*/
		static std::wstring get_abs_full_path_of_dev_lib_dll()
		{
			std::wstring s;

#ifdef _WIN32
			//windows///////////////
#ifdef _WIN64
			//win64
#ifdef _DEBUG
			//win64 + debug
			s = std::wstring(_mp::_coffee::CONST_S_DIR_DLL_EXCEPT_BACKSLASH) + std::wstring(L"\\dev_lib.dll");
#else
			//win64 + release
			s = _mp::cfile::get_path_ProgramFiles() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\dll\\x64\\dev_lib.dll");
#endif
#elif defined(_M_IX86)
			//winx86
#ifdef _DEBUG
			//winx86 + debug
			s = std::wstring(_mp::_coffee::CONST_S_DIR_DLL_EXCEPT_BACKSLASH) + std::wstring(L"\\dev_lib.dll");
#else
			//winx86 + release
			s = _mp::cfile::get_path_ProgramFiles() + std::wstring(L"\\elpusk\\00000006\\coffee_manager\\dll\\x86\\dev_lib.dll");
#endif
#else
			//NON support arm
#endif
#else
			//linux x64////////////////////
			s = std::wstring(_mp::_coffee::CONST_S_DIR_DLL_DEV_LIB_SO);
#endif
			return s;
		}
	};
}