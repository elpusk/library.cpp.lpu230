#pragma once

#include <string>

#include <mp_coffee.h>

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
			std::wstring s_file(_coffee::CONST_S_ROOT_DIR_EXCEPT_BACKSLASH);
			return s_file;
		}

		static std::wstring get_path_of_coffee_mgmt_ini_file()
		{
			std::wstring s = _mp::_coffee::CONST_S_COFFEE_MGMT_INI_DIR_EXCEPT_BACKSLASH;
#ifdef _WIN32
			s += L"\\elpusk-hid-d.json";
#else
			s += L"/elpusk-hid-d.json";
#endif //_WIN32
			return s;
		}

		static std::wstring get_path_of_coffee_lpu237_dll_ini_file()
		{
			std::wstring s = _mp::_coffee::CONST_S_COFFEE_MSR_INI_DIR_EXCEPT_BACKSLASH;
#ifdef _WIN32
			s += L"\\tg_lpu237_dll.ini";
#else
			s += L"/tg_lpu237_dll.ini";
#endif //_WIN32
			return s;
		}

		static std::wstring get_path_of_coffee_lpu237_ibutton_ini_file()
		{
			std::wstring s = _mp::_coffee::CONST_S_COFFEE_IBUTTON_INI_DIR_EXCEPT_BACKSLASH;
#ifdef _WIN32
			s += L"\\tg_lpu237_ibutton.ini";
#else
			s += L"/tg_lpu237_ibutton.ini";
#endif //_WIN32
			return s;
		}

    };
}