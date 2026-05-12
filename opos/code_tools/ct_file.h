#pragma once

// c++ header & all inline
// support only unicode.
#include <iostream>     // std::cout, std::ios
#include <sstream>      // std::ostringstream
#include <fstream>
#include <limits>

#include <ct_convert.h>
#include <ct_string.h>

#include <ShlObj.h>
#include <Knownfolders.h>
#include <time.h>

namespace _ns_tools
{
	class ct_file
	{
	public:
		typedef	std::shared_ptr< WIN32_FIND_DATAW >	type_ptr_win32_find_data;
		typedef	std::pair<std::wstring, type_ptr_win32_find_data >	type_pair_find_data;
		typedef	std::list< type_pair_find_data>	type_list_pair_find_data;

	//exported methods
	public:
		static void save_to_file(std::ofstream & o_file, const std::wstring & s_data)
		{
			do {
				size_t n_size = s_data.length() * sizeof(std::wstring::value_type);
				o_file.write((const char*)&n_size, sizeof(n_size));
				if (s_data.empty())
					continue;
				o_file.write((const char*)(s_data.c_str()), s_data.length() * sizeof(std::wstring::value_type));
			} while (false);
		}
		static void load_from_file(std::wstring & s_data, std::ifstream & i_file)
		{
			do {
				size_t n_size(0);
				i_file.read((char*)&n_size, sizeof(n_size));

				s_data.clear();
				if (n_size <= 0) {
					continue;
				}

				_ns_tools::type_v_buffer v_data(n_size, 0);
				i_file.read((char*)&v_data[0], v_data.size());
				s_data.assign((const wchar_t*)&v_data[0], v_data.size() / sizeof(std::wstring::value_type));

			} while (false);
		}
		static void save_to_file(std::ofstream & o_file, const std::string & s_data)
		{
			do {
				size_t n_size = s_data.length() * sizeof(std::string::value_type);
				o_file.write((const char*)&n_size, sizeof(n_size));
				if (s_data.empty())
					continue;
				o_file.write(s_data.c_str(), s_data.length() * sizeof(std::string::value_type));
			} while (false);
		}
		static void load_from_file(std::string & s_data, std::ifstream & i_file)
		{
			do {
				size_t n_size(0);
				i_file.read((char*)&n_size, sizeof(n_size));

				s_data.clear();
				if (n_size <= 0) {
					continue;
				}

				_ns_tools::type_v_buffer v_data(n_size, 0);
				i_file.read((char*)&v_data[0], v_data.size());
				s_data.assign((const char*)&v_data[0], v_data.size() / sizeof(std::string::value_type));

			} while (false);
		}

		static void save_to_file(std::ofstream & o_file, size_t n_size)
		{
			o_file.write((const char*)&n_size, sizeof(n_size));
		}
		static void load_from_file(size_t & n_size, std::ifstream & i_file)
		{
			i_file.read((char*)&n_size, sizeof(n_size));
		}

		static void save_to_file(std::ofstream & o_file, ct_convert::type_value_type type)
		{
			o_file.write((const char*)&type, sizeof(type));
		}
		static void load_from_file(ct_convert::type_value_type & type, std::ifstream & i_file)
		{
			i_file.read((char*)&type, sizeof(type));
		}

		// including backslash
		static std::wstring get_module_abs_path(const std::wstring & s_relative_path = L".")
		{
			std::wstring s_abs_path;
			std::wstring s_file_path;
			_ns_tools::type_v_ws_buffer s_path(_MAX_PATH + 1, 0);

			do {
				if (!::GetModuleFileName(NULL, &s_path[0], (unsigned long)(s_path.size() - 1)))
					continue;
				//
				s_file_path = (WCHAR*)&s_path[0];

				size_t n_pos = s_file_path.rfind(L'\\');
				if (s_relative_path.compare(L".") == 0)
					s_abs_path = s_file_path.substr(0, n_pos + 1);
				else if (s_relative_path.compare(L"..") == 0) {
					s_file_path = s_file_path.substr(0, n_pos);
					n_pos = s_file_path.rfind(L'\\');
					s_abs_path = s_file_path.substr(0, n_pos + 1);
				}

			} while (false);
			return s_abs_path;
		}

		static bool split_path(
			const std::wstring & s_in_path
			, std::wstring & s_out_drive
			, std::wstring & s_out_dir
			, std::wstring & s_out_file_name
			, std::wstring & s_out_ext
		)
		{
			wchar_t s_path_buffer[_MAX_PATH] = { 0, };
			wchar_t s_drive[_MAX_DRIVE] = { 0, };
			wchar_t s_dir[_MAX_DIR] = { 0, };
			wchar_t s_fname[_MAX_FNAME] = { 0, };
			wchar_t s_ext[_MAX_EXT] = { 0, };

			errno_t err = _wsplitpath_s(
				s_in_path.c_str(),
				s_drive,
				_MAX_DRIVE,
				s_dir,
				_MAX_DIR,
				s_fname,
				_MAX_FNAME,
				s_ext,
				_MAX_EXT
			);

			if (err == 0) {
				s_out_drive = s_drive;
				s_out_dir = s_dir;
				s_out_file_name = s_fname;
				s_out_ext = s_ext;
				return true;
			}
			else
				return false;
		}

		static void make_path(
			std::wstring & s_out_path
			, const std::wstring & s_out_drive
			, const std::wstring & s_out_dir
			, const std::wstring & s_out_file_name
			, const std::wstring & s_out_ext
		) 
		{
			s_out_path = s_out_drive;
			s_out_path += s_out_dir;
			s_out_path += s_out_file_name;
			s_out_path += s_out_ext;
		}

		static unsigned long long get_file_size(const std::wstring& s_abs_full_file_path)
		{
			unsigned long long ll_size(0);
			WIN32_FIND_DATA fi = { 0, };
			HANDLE h_find(FindFirstFile(s_abs_full_file_path.c_str(), &fi));
			if (h_find != INVALID_HANDLE_VALUE) {
				ll_size = _NS_TOOLS_MAKE_QWORD(fi.nFileSizeHigh, fi.nFileSizeLow);
				FindClose(h_find);
			}
			return ll_size;
		}
		static bool is_exist_file(const std::wstring& s_abs_path)
		{
			bool b_result(false);

			unsigned long long ll_size(0);
			WIN32_FIND_DATA fi = { 0, };
			HANDLE h_find(FindFirstFile(s_abs_path.c_str(), &fi));
			if (h_find != INVALID_HANDLE_VALUE) {
				b_result = true;
				FindClose(h_find);
			}
			return b_result;
		}
		static bool is_exist_folder(const std::wstring& s_abs_path, bool b_if_not_exist_then_create_folder = false)
		{
			bool b_result(false);

			do {
				DWORD dw_attrib = GetFileAttributes(s_abs_path.c_str());

				// not exist or not folder
				if (dw_attrib == 0xffffffff || (dw_attrib & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) {
					if (b_if_not_exist_then_create_folder) {
						if (ERROR_SUCCESS != SHCreateDirectory(NULL, s_abs_path.c_str())) {
							continue;
						}
					}
					continue;
				}

				b_result = true;
			} while (false);
			return b_result;
		}

		/*!
		* the start default folder path & file path methods.
		*/
		static std::wstring get_default_log_file_abs_path(const std::wstring& s_company, size_t n_employee_id, const std::wstring& s_modual_name)
		{
			std::wstring s_full_abs_path;

			do {
				std::wstring s_folder_abs_path = get_default_log_folder_path(s_company, n_employee_id, s_modual_name, true);
				if (s_folder_abs_path.empty())
					continue;
				//
				struct tm dt = { 0, };
				time_t t;
				t = time(NULL);
				localtime_s(&dt, &t);
				//
				ct_string::format(s_full_abs_path,L"%s\\%02d%02d%02d%02d%02d.txt", s_folder_abs_path.c_str(),dt.tm_mon + 1,dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);

			} while (false);
			return s_full_abs_path;
		}

		static std::wstring get_default_initial_file_abs_path(const std::wstring & s_company, size_t n_employee_id, const std::wstring& s_modual_name)
		{
			std::wstring s_ini_file_path;
			bool b_if_not_exist_create_folder(true);

			do {
				if (s_company.empty())
					continue;
				std::wstring s_file_path;
				if (s_modual_name.empty()) {
					_ns_tools::type_v_ws_buffer s_path(_MAX_PATH + 1, 0);
					if (!::GetModuleFileName(NULL, &s_path[0], (unsigned long)(s_path.size() - 1)))
						continue;
					//
					s_file_path = (WCHAR*)&s_path[0];
				}
				else
					s_file_path = s_modual_name;
				std::wstring s_out_drive;
				std::wstring s_out_dir;
				std::wstring s_out_file_name;
				std::wstring s_out_ext;

				if (!ct_file::split_path(s_file_path, s_out_drive, s_out_dir, s_out_file_name, s_out_ext))
					continue;

				s_ini_file_path = ct_file::get_path_ProgramData();

				std::wstring s_id_path;
				if( n_employee_id == -1) {
					ct_string::format_c_style(s_id_path, L"\\%s\\%s", s_company.c_str(), s_out_file_name.c_str());
				}
				else {
					ct_string::format(s_id_path, L"\\%s\\%08d\\%s", s_company.c_str(), n_employee_id, s_out_file_name.c_str());
				}
				s_ini_file_path += s_id_path;

				if (b_if_not_exist_create_folder) {
					is_exist_folder(s_ini_file_path, true);
				}

				s_ini_file_path += L"\\";
				s_ini_file_path += s_out_file_name;
				s_ini_file_path += L".xml";
			} while (false);
			return s_ini_file_path;
		}

		static std::wstring get_default_initial_file_abs_path_without_backslash_and_file_name(
			const std::wstring & s_company,
			 size_t n_employee_id,
			  const std::wstring& s_modual_name
			  )
		{
			std::wstring s_ini_file_path;

			do {
				if (s_company.empty())
					continue;
				std::wstring s_file_path;
				if (s_modual_name.empty()) {
					_ns_tools::type_v_ws_buffer s_path(_MAX_PATH + 1, 0);
					if (!::GetModuleFileName(NULL, &s_path[0], (unsigned long)(s_path.size() - 1)))
						continue;
					//
					s_file_path = (WCHAR*)&s_path[0];
				}
				else
					s_file_path = s_modual_name;
				std::wstring s_out_drive;
				std::wstring s_out_dir;
				std::wstring s_out_file_name;
				std::wstring s_out_ext;

				if (!ct_file::split_path(s_file_path, s_out_drive, s_out_dir, s_out_file_name, s_out_ext))
					continue;

				s_ini_file_path = ct_file::get_path_ProgramData();

				std::wstring s_id_path;
				if( n_employee_id == -1) {
					ct_string::format_c_style(s_id_path, L"\\%s\\%s", s_company.c_str(), s_out_file_name.c_str());
				}
				else {
					ct_string::format(s_id_path, L"\\%s\\%08d\\%s", s_company.c_str(), n_employee_id, s_out_file_name.c_str());
				}
				s_ini_file_path += s_id_path;
			} while (false);
			return s_ini_file_path;
		}
		// default log abs path is
		//AppData/Local/elpusk/employee id/exe file name/log
		static std::wstring get_default_log_folder_path(const std::wstring& s_company, size_t n_employee_id, const std::wstring& s_modual_name, bool b_if_not_exist_create_folder )
		{
			std::wstring s_log_path;

			do {
				if (s_company.empty())
					continue;
				std::wstring s_file_path;

				if (s_modual_name.empty()) {
					_ns_tools::type_v_ws_buffer s_path(_MAX_PATH + 1, 0);
					if (!::GetModuleFileName(NULL, &s_path[0], (unsigned long)(s_path.size() - 1)))
						continue;
					//
					s_file_path = (WCHAR*)&s_path[0];
				}
				else
					s_file_path = s_modual_name;
				//
				std::wstring s_out_drive;
				std::wstring s_out_dir;
				std::wstring s_out_file_name;
				std::wstring s_out_ext;

				if (!ct_file::split_path(s_file_path, s_out_drive, s_out_dir, s_out_file_name, s_out_ext))
					continue;

				s_log_path = ct_file::get_path_ProgramData();

				std::wstring s_id_path;
				if (n_employee_id == -1) {
					ct_string::format_c_style(s_id_path, L"\\%s\\%s\\log", s_company.c_str(), s_out_file_name.c_str());
				}
				else {
					ct_string::format(s_id_path, L"\\%s\\%08d\\%s\\log", s_company.c_str(), n_employee_id, s_out_file_name.c_str());
				}
				s_log_path += s_id_path;

				if (b_if_not_exist_create_folder) {
					is_exist_folder(s_log_path, true);
				}
			} while (false);
			return s_log_path;
		}

		//default initial file abs path is (initial file is xml format)
		//ProgramData/elpusk/employee id/exe file name
		static std::wstring get_default_initial_file_folder_path(const std::wstring& s_company, size_t n_employee_id, const std::wstring& s_modual_name, bool b_if_not_exist_create_folder)
		{
			std::wstring s_ini_path;

			do {
				if (s_company.empty())
					continue;
				//
				std::wstring s_file_path;

				if (s_modual_name.empty()) {
					_ns_tools::type_v_ws_buffer s_path(_MAX_PATH + 1, 0);
					if (!::GetModuleFileName(NULL, &s_path[0], (unsigned long)(s_path.size() - 1)))
						continue;
					//
					s_file_path = (WCHAR*)&s_path[0];
				}
				else {
					s_file_path = s_modual_name;
				}
				std::wstring s_out_drive;
				std::wstring s_out_dir;
				std::wstring s_out_file_name;
				std::wstring s_out_ext;

				if (!ct_file::split_path(s_file_path, s_out_drive, s_out_dir, s_out_file_name, s_out_ext))
					continue;

				s_ini_path = ct_file::get_path_ProgramData();

				std::wstring s_id_path;
				if (n_employee_id == -1) {
					ct_string::format_c_style(s_id_path, L"\\%s\\%s", s_company.c_str(), s_out_file_name.c_str());
				}
				else {
					ct_string::format(s_id_path, L"\\%s\\%08d\\%s", s_company.c_str(), n_employee_id, s_out_file_name.c_str());
				}
				s_ini_path += s_id_path;

				if (b_if_not_exist_create_folder) {
					is_exist_folder(s_ini_path, true);
				}
			} while (false);
			return s_ini_path;
		}

		// excluding backslash
		//ProgramData/elpusk/employee id/exe file name/data
		static std::wstring get_default_data_folder_path(const std::wstring& s_company, size_t n_employee_id, const std::wstring& s_modual_name, bool b_if_not_exist_create_folder = false)
		{
			std::wstring s_folder(ct_file::get_default_initial_file_folder_path(s_company, n_employee_id, s_modual_name,b_if_not_exist_create_folder));
			do {
				if (s_folder.empty())
					continue;
				s_folder += L"\\";
				s_folder += L"data";
				if (b_if_not_exist_create_folder) {
					is_exist_folder(s_folder, true);
				}
			} while (false);
			return s_folder;
		}

		// excluding backslash
		//on x86 system : Program Files/elpusk/employee id/project name/bin
		//on x64 system & 32 bits app : Program Files(x86)/elpusk/employee id/project name/bin
		//on x64 system & 64 bits app - Program Files/elpusk/employee id/project name/bin
		static std::wstring get_default_application_bin_folder_path(const std::wstring & s_application, bool b_if_not_exist_create_folder = false, size_t n_employee_id = 6)
		{
			std::wstring s_folder(ct_file::get_default_application_folder_path(s_application, b_if_not_exist_create_folder, n_employee_id));
			do {
				if (s_folder.empty())
					continue;

				s_folder += L"\\";
				s_folder += L"bin";
				if (b_if_not_exist_create_folder) {
					is_exist_folder(s_folder, true);
				}
			} while (false);
			return s_folder;
		}

		// excluding backslash
		//on x86 system : Program Files/elpusk/employee id/project name/dll
		//on x64 system & 32 bits app : Program Files(x86)/elpusk/employee id/project name/dll
		//on x64 system & 64 bits app - Program Files/elpusk/employee id/project name/dll
		static std::wstring get_default_application_dll_folder_path(const std::wstring& s_application, bool b_if_not_exist_create_folder = false, size_t n_employee_id = 6)
		{
			return _ns_tools::ct_file::get_default_application_dll_folder_path(L"elpusk", s_application, b_if_not_exist_create_folder, n_employee_id);
		}

		// excluding backslash
		//on x86 system : Program Files/s_company/employee id/project name/dll
		//on x64 system & 32 bits app : Program Files(x86)/s_company/employee id/project name/dll
		//on x64 system & 64 bits app - Program Files/s_company/employee id/project name/dll
		static std::wstring get_default_application_dll_folder_path(const std::wstring& s_company,const std::wstring& s_application, bool b_if_not_exist_create_folder = false, size_t n_employee_id = 6)
		{
			std::wstring s_folder(ct_file::get_default_application_folder_path(s_company,s_application, b_if_not_exist_create_folder, n_employee_id));
			do {
				if (s_folder.empty())
					continue;

				s_folder += L"\\";
				s_folder += L"dll";
				if (b_if_not_exist_create_folder) {
					is_exist_folder(s_folder, true);
				}
			} while (false);
			return s_folder;
		}

		// excluding backslash
		//on x86 system : Program Files/elpusk/employee id/project name
		//on x64 system & 32 bits app : Program Files(x86)/elpusk/employee id/project name
		//on x64 system & 64 bits app - Program Files/elpusk/employee id/project name
		static std::wstring get_default_application_folder_path(const std::wstring& s_application, bool b_if_not_exist_create_folder = false, size_t n_employee_id = 6)
		{
			return _ns_tools::ct_file::get_default_application_folder_path(L"elpusk", s_application, b_if_not_exist_create_folder, n_employee_id);
		}

		// excluding backslash
		//on x86 system : Program Files/s_company/employee id/project name
		//on x64 system & 32 bits app : Program Files(x86)/s_company/employee id/project name
		//on x64 system & 64 bits app - Program Files/s_company/employee id/project name
		static std::wstring get_default_application_folder_path(const std::wstring& s_company, const std::wstring& s_application, bool b_if_not_exist_create_folder = false, size_t n_employee_id = 6)
		{
			std::wstring s_folder(ct_file::get_path_ProgramFiles());
			do {
				if (s_application.empty())
					continue;
				if (s_folder.empty())
					continue;

				std::wstring s_id_path;
				if (s_company.empty()) {
					if (n_employee_id == -1) {
						ct_string::format_c_style(s_id_path, L"\\%s", s_application.c_str());
					}
					else {
						ct_string::format(s_id_path, L"\\%08d\\%s", n_employee_id, s_application.c_str());
					}
				}
				else {
					if (n_employee_id == -1) {
						ct_string::format_c_style(s_id_path, L"\\%s\\%s", s_company.c_str(),s_application.c_str());
					}
					else {
						ct_string::format(s_id_path, L"\\%s\\%08d\\%s", s_company.c_str(),n_employee_id, s_application.c_str());
					}
				}
				s_folder += s_id_path;

				if (b_if_not_exist_create_folder) {
					is_exist_folder(s_folder, true);
				}
			} while (false);
			return s_folder;
		}

		/*!
		basic path methods
		*/
		static std::wstring get_path_AppData()
		{
			return ct_file::_get_known_path({ 0x1e87508d, 0x89c2, 0x42f0, 0x8a, 0x7e, 0x64, 0x5a, 0x0f, 0x50, 0xca, 0x58 });//FOLDERID_AppsFolder
		}
		static std::wstring get_path_AppData_Local()
		{
			return ct_file::_get_known_path(FOLDERID_LocalAppData);
		}
		static std::wstring get_path_AppData_LocalLow()
		{
			return ct_file::_get_known_path(FOLDERID_LocalAppDataLow);
		}
		static std::wstring get_path_AppData_Roaming()
		{
			return ct_file::_get_known_path(FOLDERID_RoamingAppData);
		}
		static std::wstring get_path_ProgramData()
		{
			return ct_file::_get_known_path(FOLDERID_ProgramData);
		}
		static std::wstring get_path_ProgramFiles()
		{
			return ct_file::_get_known_path(FOLDERID_ProgramFiles);
		}
		static std::wstring get_path_ProgramFilesX64()//only for 64bits system
		{
			return ct_file::_get_known_path(FOLDERID_ProgramFilesX64);
		}
		static std::wstring get_path_ProgramFilesX86()
		{
			return ct_file::_get_known_path(FOLDERID_ProgramFilesX86);
		}

		static size_t filtering_find_data(
			type_list_pair_find_data & in_out_list_pair_found
			, type_found_file_time found_file_time_type
			,unsigned long long ll_limit_day
			,unsigned long long ll_limit_hour
			,unsigned long long ll_limit_minute
			,unsigned long long ll_limit_second
			,bool b_reomve_if_found_file_time_is_past_then_limit_time_from_base_time
			,const FILETIME & base_time = FILETIME()
		)
		{
			size_t n_find(0);
			do {
				LARGE_INTEGER li_base_time = { 0, };
				FILETIME base_file_time;

				//change base time format
				memcpy(&li_base_time, &base_time, sizeof(base_time));
				if (li_base_time.QuadPart == 0) {
					SYSTEMTIME sys_time;
					GetLocalTime(&sys_time);
					if (!SystemTimeToFileTime(&sys_time, &base_file_time))
						continue;
					memcpy(&li_base_time, &base_file_time, sizeof(base_file_time));
				}
				
				//move base time to the past by limit time.
				li_base_time.QuadPart -= (LONGLONG)(_NS_TOOLS_CT_TYPE_FILE_TIMEIN_DAY*ll_limit_day);
				li_base_time.QuadPart -= (LONGLONG)(_NS_TOOLS_CT_TYPE_FILE_TIMEIN_HOUR*ll_limit_hour);
				li_base_time.QuadPart -= (LONGLONG)(_NS_TOOLS_CT_TYPE_FILE_TIMEIN_MINUTE*ll_limit_minute);
				li_base_time.QuadPart -= (LONGLONG)(_NS_TOOLS_CT_TYPE_FILE_TIMEIN_SECOND*ll_limit_second);
				memcpy(&base_file_time, &li_base_time, sizeof(base_file_time));

				in_out_list_pair_found.remove_if([&](type_pair_find_data pair_data)->bool {
					bool b_result(false);
					do {
						type_ptr_win32_find_data ptr_find = pair_data.second;
						if (ptr_find == nullptr)
							continue;
						//
						FILETIME *p_file_time(nullptr);
						if (found_file_time_type == file_time_create) {
							p_file_time = &ptr_find->ftCreationTime;
						}
						else if (found_file_time_type == file_time_last_access) {
							p_file_time = &ptr_find->ftLastAccessTime;
						}
						else {
							p_file_time = &ptr_find->ftLastWriteTime;
						}

						//First file time is past than second file time.
						if (CompareFileTime(&base_file_time, p_file_time) == -1)
							b_result = true;
						//
						if (b_reomve_if_found_file_time_is_past_then_limit_time_from_base_time)
							b_result = !b_result;
					} while (false);
					return b_result;
				});

				n_find = in_out_list_pair_found.size();
			} while (false);
			return n_find;
		}

		enum type_find_folder_area
		{
			folder_area_all_sub_folder = -1,
			folder_area_none = 0,
			folder_area_current_folder = 1,
			folder_area_current_folder_and_next_sub_folder = 2
		};

		static void get_find_file_list(
			type_list_pair_find_data & list_pair_found
			, const std::wstring & s_find_root_folder, const std::wstring & s_filter
			, ct_file::type_find_folder_area find_area = ct_file::folder_area_all_sub_folder
		)
		{
			if (find_area == folder_area_none)
				return;

			std::wstring s_found;
			WIN32_FIND_DATAW FindFileData;
			bool b_add_file = true;

			if (s_filter[1] == 0) {
				if (s_filter[0] == L'\\') {
					b_add_file = false;
				}
			}

			s_found = s_find_root_folder;
			s_found += L"\\*.*";

			HANDLE h_find = FindFirstFile(s_found.c_str(), &FindFileData);

			for (; h_find != INVALID_HANDLE_VALUE; ) {
				if (FindFileData.cFileName[0] != L'.' || wcslen(FindFileData.cFileName) > 2) {
					s_found = s_find_root_folder;
					s_found += L"\\";
					s_found += FindFileData.cFileName;

					if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						type_ptr_win32_find_data ptr_data = std::make_shared<WIN32_FIND_DATAW>();
						memcpy(ptr_data.get(), &FindFileData, sizeof(FindFileData));
						list_pair_found.push_back(make_pair(s_found, ptr_data));
						//
						get_find_file_list(
							list_pair_found
							, s_found, s_filter
							, (ct_file::type_find_folder_area)((int)find_area - 1)
						);
					}
					else {
						if (b_add_file) {
							type_ptr_win32_find_data ptr_data = std::make_shared<WIN32_FIND_DATAW>();
							memcpy(ptr_data.get(), &FindFileData, sizeof(FindFileData));
							list_pair_found.push_back(make_pair(s_found, ptr_data));

						}
					}
				}

				if (!FindNextFile(h_find, &FindFileData))
					break;
			}//end for

			FindClose(h_find);
		}

		static void get_find_file_list(
			type_list_wstring & list_found
			, const std::wstring & s_find_root_folder, const std::wstring & s_filter
			, ct_file::type_find_folder_area find_area = ct_file::folder_area_all_sub_folder
			, bool b_include_folder = true
		)
		{
			if (find_area == folder_area_none)
				return;

			std::wstring s_found;
			WIN32_FIND_DATA FindFileData;
			bool b_add_file = true;
			bool b_add_folder = false;

			if (s_filter[1] == 0){
				if (s_filter[0] == L'\\'){
					b_add_file = false;
					b_add_folder = true;
				}
				else if (s_filter[0] == L'*'){
					b_add_folder = true;
				}
			}

			s_found = s_find_root_folder;
			s_found += L"\\";

			if (b_add_folder == true)
				s_found += L"*.*";
			else
				s_found += s_filter;

			HANDLE h_find = FindFirstFile(s_found.c_str(), &FindFileData);

			for (; h_find != INVALID_HANDLE_VALUE; ){
				if (FindFileData.cFileName[0] != L'.' || wcslen(FindFileData.cFileName) > 2){
					s_found = s_find_root_folder;
					s_found += L"\\";
					s_found += FindFileData.cFileName;

					if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
						if (b_add_folder & b_include_folder)
							list_found.push_back(s_found);
						//
						get_find_file_list(
							list_found
							, s_found, s_filter
							, (ct_file::type_find_folder_area)((int)find_area - 1)
							, b_include_folder
						);
					}
					else{
						if (b_add_file)
							list_found.push_back(s_found);
					}
				}

				if (!FindNextFile(h_find, &FindFileData))
					break;
			}//end for

			FindClose(h_find);
		}

		static void get_find_sub_folder_list(
			type_list_wstring & list_found
			, const std::wstring & s_find_root_folder, const std::wstring & s_filter
			, ct_file::type_find_folder_area find_area = ct_file::folder_area_all_sub_folder
		)
		{
			if (find_area == folder_area_none)
				return;

			std::wstring s_found;
			WIN32_FIND_DATA FindFileData;
			bool b_add_folder = false;

			if (s_filter[1] == 0) {
				if (s_filter[0] == L'\\') {
					b_add_folder = true;
				}
				else if (s_filter[0] == L'*') {
					b_add_folder = true;
				}
			}

			s_found = s_find_root_folder;
			s_found += L"\\";

			if (b_add_folder == true)
				s_found += L"*.*";
			else
				s_found += s_filter;

			HANDLE h_find = FindFirstFile(s_found.c_str(), &FindFileData);

			for (; h_find != INVALID_HANDLE_VALUE; ) {
				if (FindFileData.cFileName[0] != L'.' || wcslen(FindFileData.cFileName) > 2) {
					s_found = s_find_root_folder;
					s_found += L"\\";
					s_found += FindFileData.cFileName;

					if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						if (b_add_folder)
							list_found.push_back(s_found);
						//
						get_find_sub_folder_list(
							list_found
							, s_found, s_filter
							, (ct_file::type_find_folder_area)((int)find_area - 1)
						);
					}
				}

				if (!FindNextFile(h_find, &FindFileData))
					break;
			}//end for

			FindClose(h_find);
		}

		static unsigned long long get_folder_size(
			const std::wstring & s_find_root_folder
			, ct_file::type_find_folder_area find_area = ct_file::folder_area_all_sub_folder
		)
		{
			unsigned long long ll_size(0);

			if (find_area == folder_area_none)
				return ll_size;
			
			std::wstring s_found;
			WIN32_FIND_DATA FindFileData;
			bool b_add_file = true;

			s_found = s_find_root_folder;
			s_found += L"\\";
			s_found += L"*.*";

			HANDLE h_find = FindFirstFile(s_found.c_str(), &FindFileData);

			for (; h_find != INVALID_HANDLE_VALUE; ) {
				if (FindFileData.cFileName[0] != L'.' || wcslen(FindFileData.cFileName) > 2) {
					s_found = s_find_root_folder;
					s_found += L"\\";
					s_found += FindFileData.cFileName;

					if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						//
						ll_size += get_folder_size(
							s_found
							, (ct_file::type_find_folder_area)((int)find_area - 1)
						);
					}
					else {
						if (b_add_file)
							ll_size += (_NS_TOOLS_MAKE_QWORD(FindFileData.nFileSizeHigh, FindFileData.nFileSizeLow));
					}
				}

				if (!FindNextFile(h_find, &FindFileData))
					break;
			}//end for

			FindClose(h_find);
			return ll_size;
		}

		static std::wstring get_file_name_only_from_path(const std::wstring& s_path)
		{
			std::wstring file_name_only;

			do {
				if (s_path.empty())
					continue;

				std::wstring s_out_drive;
				std::wstring s_out_dir;
				std::wstring s_out_file_name;
				std::wstring s_out_ext;

				if (!ct_file::split_path(s_path, s_out_drive, s_out_dir, s_out_file_name, s_out_ext))
					continue;

				file_name_only = s_out_file_name;
			} while (false);
			return file_name_only;
		}

		static std::wstring get_file_name_and_extention_only_from_path(const std::wstring& s_path)
		{
			std::wstring file_name_only;

			do {
				if (s_path.empty())
					continue;

				std::wstring s_out_drive;
				std::wstring s_out_dir;
				std::wstring s_out_file_name;
				std::wstring s_out_ext;

				if (!ct_file::split_path(s_path, s_out_drive, s_out_dir, s_out_file_name, s_out_ext))
					continue;

				file_name_only = s_out_file_name+ s_out_ext;
			} while (false);
			return file_name_only;
		}

		static bool remove_folder_contents(const std::wstring & s_folder_abs_path_except_backslash, const std::wstring& s_filter, bool b_except_sub_folder = true)
		{
			bool b_result(false);
			do {
				if (s_folder_abs_path_except_backslash.empty())
					continue;
				//
				type_list_wstring list_found;
				ct_file::type_find_folder_area area(ct_file::folder_area_all_sub_folder);

				if (b_except_sub_folder)
					area = ct_file::folder_area_current_folder;

				//find only files.
				ct_file::get_find_file_list(
					list_found
					, s_folder_abs_path_except_backslash
					, s_filter
					, area
					, false
				);
				//
				b_result = true;

				//delete only files
				std::for_each(std::begin(list_found), std::end(list_found), [&](const std::wstring& s_path) {
					if (s_folder_abs_path_except_backslash.compare(s_path) != 0)
						if (!DeleteFile(s_path.c_str()))
							b_result = false;
					});

				if (!b_result)
					continue;
				if (b_except_sub_folder)
					continue;
				//
				type_list_wstring list_folder_found;
				ct_file::get_find_sub_folder_list(
					list_folder_found
					, s_folder_abs_path_except_backslash
					, L"*"
					, area
				);
				//delete folder
				list_folder_found.sort([=](const std::wstring& s_1, const std::wstring& s_2)->bool {
					size_t n_1(0), n_2(0);
					size_t n_found = s_1.find('\\');
					while (n_found != std::wstring::npos) {
						++n_1;
						n_found = s_1.find('\\', n_found + 1);
					}
					//
					n_found = s_2.find('\\');
					while (n_found != std::wstring::npos) {
						++n_2;
						n_found = s_2.find('\\', n_found + 1);
					}

					if (n_2 < n_1)
						return true;
					else
						return false;
					});
				//
				std::for_each(std::begin(list_folder_found), std::end(list_folder_found), [&](const std::wstring& s_path) {
					if (!SetFileAttributes(s_path.c_str(), FILE_ATTRIBUTE_NORMAL))
						b_result = false;
					else {
						if (!RemoveDirectory(s_path.c_str())) {
							DWORD dw_result = GetLastError();
							b_result = false;
						}
					}
					});

			} while (false);
			return b_result;
		}

		// excluding backslash
		// return first : true(current program is x86), false(current x64)
		// return second on x86 system : Program Files/Easyset/lpu230/bin/components/x86
		// return second on x64 system & 32 bits app : Program Files(x86)/Easyset/lpu230/bin/components/x86
		// return second on x64 system & 64 bits app : Program Files/Easyset/lpu230/bin/components/x64
		static _ns_tools::type_pair_string_bool get_lpu230_component_dll_path()
		{
			std::wstring s_dll_path;
			bool b_x86(true);

			do {
				std::wstring s_programfilesx64(_ns_tools::ct_file::get_path_ProgramFilesX64());

				if (s_programfilesx64.empty()) {
					// this program is x86( system x86 or x64 )
					s_dll_path = _ns_tools::ct_file::get_path_ProgramFiles();
					s_dll_path += L"\\Easyset\\lpu230\\bin\\components\\x86";
					continue;
				}

				// this program is x64( system x64 )
				s_dll_path = s_programfilesx64;
				s_dll_path += L"\\Easyset\\lpu230\\bin\\components\\x64";
				b_x86 = false;
			} while (false);
			return std::make_pair(s_dll_path, b_x86);
		}
	public:
		~ct_file(){}
	
	private:
		static std::wstring _get_known_path(REFKNOWNFOLDERID rfid)
		{
			std::wstring s_path;

			do {
				LPWSTR ps_path;
				if (S_OK != SHGetKnownFolderPath(rfid, 0, NULL, &ps_path))
					continue;
				s_path = ps_path;
				CoTaskMemFree(ps_path);
			} while (false);

			return s_path;
		}

	private://don't call these methods
		ct_file();
		ct_file( const ct_file & );
		ct_file & operator=(const ct_file &);
	
	};
}
