#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>
#include <time.h>

#include <cstring> // For std::strrchr

#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef _WIN32
#include <windows.h>
#include <ShlObj.h>
#include <Knownfolders.h>
#else
#include <pwd.h>
#include <unistd.h>
#include <linux/limits.h>
#include <libgen.h> // For dirname and basename
#endif

#include <mp_type.h>
#include <mp_cstring.h>

namespace _mp{
    class cfile {
	public:
		enum type_find_folder_area
		{
			folder_area_all_sub_folder = -1,
			folder_area_none = 0,
			folder_area_current_folder = 1,
			folder_area_current_folder_and_next_sub_folder = 2
		};
		enum  type_found_file_time
		{
			file_time_create,
			file_time_last_access,
			file_time_last_write
		};

		typedef std::pair<std::wstring, boost::filesystem::directory_entry> type_pair_find_data;
		typedef std::list<type_pair_find_data>  type_list_pair_find_data;

	public:

#ifdef _WIN32
		/**
		* @return empty or path with backslash 
		*/
		static std::wstring get_temp_path()
		{
			std::wstring s;
			wchar_t temp_path[MAX_PATH];
			DWORD length = GetTempPath(MAX_PATH, temp_path);
			if (length > 0 && length <= MAX_PATH) {
				s = std::wstring(temp_path) + L'\\';
				return s;
			}
			else {
				return s;//error
			}
		}
#else
		/**
		* @return /tmp or path with slash
		*/
		static std::wstring get_temp_path()
		{
			std::wstring s;
			const char* temp_path = std::getenv("TMPDIR");
			if (temp_path == nullptr) {
				temp_path = std::getenv("TMP");
			}
			if (temp_path == nullptr) {
				temp_path = std::getenv("TEMP");
			}
			if (temp_path == nullptr) {
				temp_path = "/tmp";//default
			}

			s = cstring::get_unicode_from_mcsc(temp_path);
			s += L'/';
			return s;
		}
#endif

#ifdef _WIN32
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
#else
		static bool is_exist_folder(const std::wstring& path, bool b_if_not_exist_then_create_folder = false) {
			struct stat info;
			std::string _path = _mp::cstring::get_mcsc_from_unicode(path);

			// '~'�� Ȩ ���丮 ��η� ��ȯ
			if (_path[0] == '~') {
				std::string home_dir = cfile::_get_home_directory();
				_path.replace(0, 1, home_dir);
			}

			if (stat(_path.c_str(), &info) != 0) {
				// ������ �������� ����
				if (b_if_not_exist_then_create_folder) {
					size_t pos = 0;
					while ((pos = _path.find('/', pos)) != std::string::npos) {
						std::string subdir = _path.substr(0, pos++);
						if (subdir.empty()) continue;
						if (stat(subdir.c_str(), &info) != 0) {
							if (mkdir(subdir.c_str(), 0777) != 0 && errno != EEXIST) {
								return false; // ���� ���� ���� ����
							}
						}
						else if (!(info.st_mode & S_IFDIR)) {
							return false; // �̹� �����ϴ� ��ΰ� ���丮�� �ƴ�
						}
					}
					if (mkdir(_path.c_str(), 0777) == 0) {
						return true; // ���� ���� ����
					}
					else {
						return errno == EEXIST; // ���� ���� ����, �̹� �����ϴ� ��� �������� ����
					}
				}
				else {
					return false;
				}
			}
			else if (info.st_mode & S_IFDIR) {
				return true; // ������ ������
			}
			else {
				return false; // path�� ������ �ƴ�
			}
		}

#endif //_WIN32
		static bool split_path(
			const std::wstring& s_in_path
			, std::wstring& s_out_drive
			, std::wstring& s_out_dir
			, std::wstring& s_out_file_name
			, std::wstring& s_out_ext
		)
		{
#ifdef _WIN32
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

#else // Non-Windows (Assumed POSIX compliant)

			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
			std::string utf8_path = converter.to_bytes(s_in_path);

			// utf8_path_cstr�� �纻�� �����Ͽ� dirname�� basename���� ���
			char* utf8_path_cstr_dir = strdup(utf8_path.c_str());
			char* utf8_path_cstr_base = strdup(utf8_path.c_str());

			// dirname�� basename�� ����Ͽ� ��θ� �и�
			char* dir = dirname(utf8_path_cstr_dir);
			char* base = basename(utf8_path_cstr_base);

			// basename���� ���ϸ��� Ȯ���ڸ� �и�
			char* ext = std::strrchr(base, '.');
			if (ext) {
				*ext = '\0'; // Ȯ���� ����
				ext++; // Ȯ���� �̸����� �̵�
			}
			else {
				ext = (char*)""; // �� ���ڿ� ����
			}

			// ��ȯ �� ����
			s_out_drive = L""; // ����̺� ���� ����
			s_out_dir = converter.from_bytes(dir);
			s_out_file_name = converter.from_bytes(base);
			s_out_ext = converter.from_bytes(ext);

			// �Ҵ�� �޸� ����
			free(utf8_path_cstr_dir);
			free(utf8_path_cstr_base);

			return true;
#endif
		}

		static unsigned long long get_folder_size(
			const boost::filesystem::path& s_find_root_folder,
			type_find_folder_area find_area = folder_area_all_sub_folder)
		{
			unsigned long long ll_size = 0;

			if (find_area == folder_area_none)
				return ll_size;

			try
			{
				boost::filesystem::directory_iterator end_iter;
				for (boost::filesystem::directory_iterator dir_iter(s_find_root_folder); dir_iter != end_iter; ++dir_iter)
				{
					if (boost::filesystem::is_directory(dir_iter->status()))
					{
						if (find_area != folder_area_current_folder)
						{
							ll_size += get_folder_size(
								dir_iter->path(),
								static_cast<type_find_folder_area>(find_area - 1));
						}
					}
					else if (boost::filesystem::is_regular_file(dir_iter->status()))
					{
						ll_size += boost::filesystem::file_size(dir_iter->path());
					}
				}
			}
			catch (const boost::filesystem::filesystem_error& e)
			{
				(void)e;
				ll_size = 0;
			}
			catch (const std::exception& e)
			{
				(void)e;
				ll_size = 0;
			}

			return ll_size;
		}

		static void get_find_file_list(
			type_list_wstring& list_found,
			const std::wstring& s_find_root_folder,
			const std::wstring& s_filter,
			type_find_folder_area find_area = folder_area_all_sub_folder,
			bool b_include_folder = true)
		{
			if (find_area == folder_area_none)
				return;

			try
			{
				boost::filesystem::path root_path(s_find_root_folder);
				boost::filesystem::directory_iterator end_iter;

				for (boost::filesystem::directory_iterator dir_iter(root_path); dir_iter != end_iter; ++dir_iter)
				{
					boost::filesystem::path current_path = dir_iter->path();
					std::wstring current_path_str = current_path.wstring();

					if (boost::filesystem::is_directory(current_path))
					{
						if (b_include_folder)
							list_found.push_back(current_path_str);

						if (find_area != folder_area_current_folder)
						{
							get_find_file_list(
								list_found,
								current_path_str,
								s_filter,
								static_cast<type_find_folder_area>(find_area - 1),
								b_include_folder);
						}
					}
					else if (boost::filesystem::is_regular_file(current_path))
					{
						if (s_filter == L"*.*" || boost::algorithm::iends_with(current_path_str, s_filter))
						{
							list_found.push_back(current_path_str);
						}
					}
				}
			}
			catch (const boost::filesystem::filesystem_error& e)
			{
				(void)e;
				list_found.clear();
			}
			catch (const std::exception& e)
			{
				(void)e;
				list_found.clear();
			}
		}

		static void get_find_sub_folder_list(
			type_list_wstring& list_found,
			const std::wstring& s_find_root_folder,
			const std::wstring& s_filter,
			type_find_folder_area find_area = folder_area_all_sub_folder)
		{
			if (find_area == folder_area_none)
				return;

			try
			{
				boost::filesystem::path root_path(s_find_root_folder);
				boost::filesystem::directory_iterator end_iter;

				for (boost::filesystem::directory_iterator dir_iter(root_path); dir_iter != end_iter; ++dir_iter)
				{
					boost::filesystem::path current_path = dir_iter->path();
					std::wstring current_path_str = current_path.wstring();

					if (boost::filesystem::is_directory(current_path))
					{
						if (s_filter == L"\\*" || s_filter == L"*.*")
						{
							list_found.push_back(current_path_str);
						}

						if (find_area != folder_area_current_folder)
						{
							get_find_sub_folder_list(
								list_found,
								current_path_str,
								s_filter,
								static_cast<type_find_folder_area>(find_area - 1));
						}
					}
				}
			}
			catch (const boost::filesystem::filesystem_error& e)
			{
				(void)e;
				list_found.clear();
			}
			catch (const std::exception& e)
			{
				(void)e;
				list_found.clear();
			}
		}

		static void get_find_file_list(
			type_list_pair_find_data& list_pair_found,
			const std::wstring& s_find_root_folder,
			const std::wstring& s_filter,
			type_find_folder_area find_area = folder_area_all_sub_folder)
		{
			if (find_area == folder_area_none)
				return;

			try
			{
				boost::filesystem::path root_path(s_find_root_folder);
				boost::filesystem::directory_iterator end_iter;

				for (boost::filesystem::directory_iterator dir_iter(root_path); dir_iter != end_iter; ++dir_iter)
				{
					boost::filesystem::path current_path = dir_iter->path();
					std::wstring current_path_str = current_path.wstring();

					if (boost::filesystem::is_directory(current_path))
					{
						list_pair_found.push_back(std::make_pair(current_path_str, *dir_iter));

						if (find_area != folder_area_current_folder)
						{
							get_find_file_list(
								list_pair_found,
								current_path_str,
								s_filter,
								static_cast<type_find_folder_area>(find_area - 1));
						}
					}
					else if (boost::filesystem::is_regular_file(current_path))
					{
						if (s_filter == L"*.*" || boost::algorithm::iends_with(current_path_str, s_filter))
						{
							list_pair_found.push_back(std::make_pair(current_path_str, *dir_iter));
						}
					}
				}
			}
			catch (const boost::filesystem::filesystem_error& e)
			{
				(void)e;
				list_pair_found.clear();
			}
			catch (const std::exception& e)
			{
				(void)e;
				list_pair_found.clear();
			}
		}

		static size_t filtering_find_data(
			type_list_pair_find_data& in_out_list_pair_found,
			type_found_file_time found_file_time_type,
			unsigned long long ll_limit_day,
			unsigned long long ll_limit_hour,
			unsigned long long ll_limit_minute,
			unsigned long long ll_limit_second,
			bool b_remove_if_found_file_time_is_past_then_limit_time_from_base_time,
			const boost::posix_time::ptime& base_time = boost::posix_time::second_clock::local_time())
		{
			size_t n_find(0);

			// Calculate the limit time from the base time
			boost::posix_time::ptime limit_time = base_time - boost::gregorian::days(static_cast<long>(ll_limit_day))
				- boost::posix_time::hours(static_cast<long>(ll_limit_hour))
				- boost::posix_time::minutes(static_cast<long>(ll_limit_minute))
				- boost::posix_time::seconds(static_cast<long>(ll_limit_second));

			in_out_list_pair_found.remove_if([&](const type_pair_find_data& pair_data) -> bool {
				bool b_result(false);
				const boost::filesystem::directory_entry& entry = pair_data.second;

				boost::posix_time::ptime file_time;
				if (found_file_time_type == file_time_create)
				{
					file_time = boost::posix_time::from_time_t(boost::filesystem::last_write_time(entry)); // Boost.Filesystem does not support creation time
				}
				else if (found_file_time_type == file_time_last_access)
				{
					file_time = boost::posix_time::from_time_t(boost::filesystem::last_write_time(entry)); // Boost.Filesystem does not support last access time
				}
				else // file_time_last_write
				{
					file_time = boost::posix_time::from_time_t(boost::filesystem::last_write_time(entry));
				}

				if (b_remove_if_found_file_time_is_past_then_limit_time_from_base_time)
				{
					b_result = (file_time < limit_time);
				}
				else
				{
					b_result = (file_time >= limit_time);
				}

				return b_result;
				});

			n_find = in_out_list_pair_found.size();
			return n_find;
		}


#ifdef _WIN32
		/**
		* The returned path does not include a trailing backslash.
		*/
		static std::wstring get_path_ProgramData()
		{
			return cfile::_get_known_path(FOLDERID_ProgramData);
		}
	private:
		/**
		* The returned path does not include a trailing backslash. 
		*/
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
#else
		static std::string _get_home_directory() 
		{
			const char* homedir;
			if ((homedir = getenv("HOME")) == nullptr) {
				homedir = getpwuid(getuid())->pw_dir;
			}
			return std::string(homedir);
		}
#endif

    private:
        cfile();
        cfile(const cfile&);
        cfile& operator=(const cfile&);
    };
}