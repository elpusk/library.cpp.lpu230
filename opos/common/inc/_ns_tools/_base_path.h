#pragma once
#include <string>
#include <ct_file.h>

//the defined path
class _cbase_path
{
public:
	typedef	enum {
		type_current,
		type_elpusk,
		type_easyset

	}type_mode;
public:
	static _cbase_path& get_instance()
	{
		static _cbase_path obj;
		return obj;
	}

	// excluding backslash
	std::wstring get_component_dll_folder_path_except_backslash()
	{
		std::wstring s_folder(_ns_tools::ct_file::get_default_application_folder_path(L"Easyset",L"lpu230",false,-1));//except backslash
		do {
			if (s_folder.empty())
				continue;
#if defined _M_IX86
			s_folder += L"\\bin\\components\\x86";
#elif defined _M_X64
			s_folder += L"\\bin\\components\\x64";
#else
#error "this condition is not accepted,"
#endif
		} while (false);
		return s_folder;
	}

	std::wstring get_easyset_path_of_ini_file(const std::wstring& s_modual_name,bool b_except_backslash_name_ext = false)
	{
		std::wstring s_file_name_only = _ns_tools::ct_file::get_file_name_only_from_path(s_modual_name);
		std::wstring s_file(_ns_tools::ct_file::get_default_initial_file_folder_path(
			L"Easyset"
			,0
			, s_modual_name
			,false
		));
		//C:\ProgramData\Easyset\00000000\lpu230
		do {
			if (s_file_name_only.empty()) {
				s_file.clear();
				continue;
			}
			if (s_file.empty())
				continue;
			if (!b_except_backslash_name_ext) {
				s_file += L"\\";
				s_file += s_file_name_only;
				s_file += L".xml";
			}
		} while (false);

		return s_file;
	}

	std::wstring get_elpusk_path_of_ini_file(const std::wstring& s_modual_name, bool b_except_backslash_name_ext = false)
	{
		std::wstring s_file_name_only = _ns_tools::ct_file::get_file_name_only_from_path(s_modual_name);
		std::wstring s_file(_ns_tools::ct_file::get_default_initial_file_folder_path(
			L"Elpusk"
			, 6
			, s_modual_name
			, false
		));
		//C:\ProgramData\Elpusk\00000006\lpu230
		do {
			if (s_file_name_only.empty()) {
				s_file.clear();
				continue;
			}
			if (s_file.empty())
				continue;
			if (!b_except_backslash_name_ext) {
				s_file += L"\\";
				s_file += s_file_name_only;
				s_file += L".xml";
			}
		} while (false);

		return s_file;
	}

	std::wstring get_module_path_of_ini_file(const std::wstring& s_modual_name, bool b_except_backslash_name_ext = false)
	{
		std::wstring s_file_name_only = _ns_tools::ct_file::get_file_name_only_from_path(s_modual_name);
		std::wstring s_file(_ns_tools::ct_file::get_module_abs_path());
		//the path of current module
		do {
			if (s_file_name_only.empty()) {
				s_file.clear();
				continue;
			}
			if (s_file.empty())
				continue;
			if (!b_except_backslash_name_ext) {
				s_file += s_file_name_only;
				s_file += L".xml";
			}
		} while (false);

		return s_file;
	}

	std::wstring get_path_of_ini_file_folder_except_backslash(const std::wstring& s_modual_name)
	{
		std::wstring s;

		if (m_cur_mode == _cbase_path::type_current) {
			s = get_module_path_of_ini_file(s_modual_name,true);
		}
		else if (m_cur_mode == _cbase_path::type_elpusk) {
			s = get_elpusk_path_of_ini_file(s_modual_name, true);
		}
		else if (m_cur_mode == _cbase_path::type_easyset) {
			s = get_easyset_path_of_ini_file(s_modual_name, true);
		}
		return s;
	}

	/**
	* thie function not create file. folder can be create!
	*/
	std::wstring get_easyset_path_of_temp_rom_file(unsigned long n_device_index,bool b_if_not_exist_foler_then_create_folder)
	{
		std::wstring s_file(_ns_tools::ct_file::get_default_data_folder_path(
			L"Easyset"
			, 0
			, L"lpu230"
			, b_if_not_exist_foler_then_create_folder
		));
		//C:\ProgramData\Easyset\00000000\lpu230\data
		do {
			if (s_file.empty())
				continue;
			s_file += L"\\temp_fw_";
			s_file += std::to_wstring((unsigned long)n_device_index);
			s_file += L".tmp";
		} while (false);
		return s_file;
	}

	/**
	* thie function not create file. folder can be create!
	*/
	std::wstring get_elpusk_path_of_temp_rom_file(unsigned long n_device_index, bool b_if_not_exist_foler_then_create_folder)
	{
		std::wstring s_file(_ns_tools::ct_file::get_default_data_folder_path(
			L"Elpusk"
			, 6
			, L"lpu230"
			, b_if_not_exist_foler_then_create_folder
		));
		//C:\ProgramData\Elpusk\00000006\lpu230\data
		do {
			if (s_file.empty())
				continue;
			s_file += L"\\temp_fw_";
			s_file += std::to_wstring((unsigned long)n_device_index);
			s_file += L".tmp";
		} while (false);
		return s_file;
	}

	/**
	* thie function not create file and folder!
	* param b_if_not_exist_foler_then_create_folder - no used
	*/
	std::wstring get_module_path_of_temp_rom_file(unsigned long n_device_index, bool b_if_not_exist_foler_then_create_folder)
	{
		std::wstring s_file(_ns_tools::ct_file::get_module_abs_path());
		//the path of current module
		do {
			if (s_file.empty())
				continue;
			s_file += L"temp_fw_";
			s_file += std::to_wstring((unsigned long)n_device_index);
			s_file += L".tmp";
		} while (false);
		return s_file;
	}

	/**
	* thie function not create file. folder can be create!
	*/
	std::wstring get_path_of_temp_rom_file(unsigned long n_device_index, bool b_if_not_exist_foler_then_create_folder)
	{
		std::wstring s;

		if (m_cur_mode == _cbase_path::type_current) {
			s = get_module_path_of_temp_rom_file(n_device_index, b_if_not_exist_foler_then_create_folder);
		}
		else if (m_cur_mode == _cbase_path::type_elpusk) {
			s = get_elpusk_path_of_temp_rom_file(n_device_index, b_if_not_exist_foler_then_create_folder);
		}
		else if (m_cur_mode == _cbase_path::type_easyset) {
			s = get_easyset_path_of_temp_rom_file(n_device_index, b_if_not_exist_foler_then_create_folder);
		}
		return s;
	}

	virtual ~_cbase_path() {}
	_cbase_path() : m_cur_mode(_cbase_path::type_current)
	{
	}

	void set_mode(_cbase_path::type_mode mode)
	{
		m_cur_mode = mode;
	}

	_cbase_path::type_mode get_mode() const
	{
		return m_cur_mode;
	}
private:
	_cbase_path::type_mode m_cur_mode;
private:
	
	_cbase_path(const _cbase_path&) = delete;
	_cbase_path& operator=(const _cbase_path&) = delete;
};
