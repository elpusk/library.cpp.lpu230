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

		static bool is_exist_file(const std::wstring& s_file_path)
		{
			return boost::filesystem::exists(s_file_path);
		}

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

			// '~'를 홈 디렉토리 경로로 변환
			if (_path[0] == '~') {
				std::string home_dir = cfile::_get_home_directory();
				_path.replace(0, 1, home_dir);
			}

			if (stat(_path.c_str(), &info) != 0) {
				// 폴더가 존재하지 않음
				if (b_if_not_exist_then_create_folder) {
					size_t pos = 0;
					while ((pos = _path.find('/', pos)) != std::string::npos) {
						std::string subdir = _path.substr(0, pos++);
						if (subdir.empty()) continue;
						if (stat(subdir.c_str(), &info) != 0) {
							if (mkdir(subdir.c_str(), 0777) != 0 && errno != EEXIST) {
								return false; // 하위 폴더 생성 실패
							}
						}
						else if (!(info.st_mode & S_IFDIR)) {
							return false; // 이미 존재하는 경로가 디렉토리가 아님
						}
					}
					if (mkdir(_path.c_str(), 0777) == 0) {
						return true; // 폴더 생성 성공
					}
					else {
						return errno == EEXIST; // 폴더 생성 실패, 이미 존재하는 경우 성공으로 간주
					}
				}
				else {
					return false;
				}
			}
			else if (info.st_mode & S_IFDIR) {
				return true; // 폴더가 존재함
			}
			else {
				return false; // path는 폴더가 아님
			}
		}

		static std::string expand_home_directory_in_linux(const std::string& s_path)
		{
			std::string s_expand;

			do {
				if (s_path.empty()) {
					continue;
				}

				s_expand = s_path;
				if (s_expand[0] == '~') {
					std::string home_dir = cfile::_get_home_directory();
					s_expand.replace(0, 1, home_dir);
				}

			} while (false);

			return s_expand;
		}
#endif //_WIN32

#ifdef _WIN32
		static bool delete_file(const std::wstring &s_file)
		{
			if (s_file.empty())
				return false;
			if (DeleteFile(s_file.c_str()))
				return true;
			else
				return false;
		}
#else
		static bool delete_file(const std::wstring& s_file)
		{
			//
			std::string ps = _mp::cstring::get_mcsc_from_unicode(s_file);
			if (unlink(ps.c_str()) != 0) {
				return false;
			}
			return true;
		}
#endif
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
			std::string utf8_path = cstring::get_mcsc_from_unicode(s_in_path);

			// utf8_path_cstr의 사본을 생성하여 dirname과 basename에서 사용
			char* utf8_path_cstr_dir = strdup(utf8_path.c_str());
			char* utf8_path_cstr_base = strdup(utf8_path.c_str());

			// dirname과 basename을 사용하여 경로를 분리
			char* dir = dirname(utf8_path_cstr_dir);
			char* base = basename(utf8_path_cstr_base);

			// basename에서 파일명과 확장자를 분리
			char* ext = std::strrchr(base, '.');
			if (ext) {
				*ext = '\0'; // 확장자 제거
				ext++; // 확장자 이름으로 이동
			}
			else {
				ext = (char*)""; // 빈 문자열 설정
			}

			// 변환 후 리턴
			s_out_drive = L""; // 드라이브 문자 없음
			s_out_dir = cstring::get_unicode_from_mcsc(dir);
			s_out_file_name = cstring::get_unicode_from_mcsc(base);
			s_out_ext = cstring::get_unicode_from_mcsc(ext);

			// 할당된 메모리 해제
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

				if (!cfile::split_path(s_path, s_out_drive, s_out_dir, s_out_file_name, s_out_ext))
					continue;

				file_name_only = s_out_file_name + s_out_ext;
			} while (false);
			return file_name_only;
		}

#ifdef _WIN32
		/**
		* The returned path does not include a trailing backslash.
		*/
		static std::wstring get_path_ProgramData()
		{
			return cfile::_get_known_path(FOLDERID_ProgramData);
		}

		/**
		* The returned path does not include a trailing backslash.
		*/
		static std::wstring get_path_ProgramFiles()
		{
			return cfile::_get_known_path(FOLDERID_ProgramFiles);
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