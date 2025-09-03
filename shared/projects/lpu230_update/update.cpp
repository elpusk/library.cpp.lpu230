#include <cstdlib>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

#include <mp_csystem_.h>
#include <mp_cfile.h>
#include <mp_coffee_path.h>

#include "update.h"


static std::vector<std::filesystem::path> _find_rom_files();

static void _signal_handler(int signum);

int update_main
(
	const std::string& s_abs_full_rom_file,
	const std::string& s_device_path,
	bool b_display,
	bool b_log_file,
	bool b_mmd1100_iso_mode,
	cupdater::Lpu237Interface lpu237_interface_after_update,
	bool b_run_by_cf
)
{
	int n_result(EXIT_FAILURE);
	std::wstring s_pid_file_full_path;
	bool b_need_remove_pid_file(false);


	do {
		/////////////////////////////////
		//setting display
		if (!b_display) {
#ifdef _WIN32
			HWND consoleWindow = GetConsoleWindow();
			ShowWindow(consoleWindow, SW_HIDE); // 콘솔 창 숨기기
#else
			s_pid_file_full_path = _mp::_coffee::CONST_S_PID_FILE_FULL_PATH_LPU230_UPDATE;
			if (!_mp::csystem::daemonize_on_linux(s_pid_file_full_path, std::wstring(), _signal_handler)) {
				continue;
			}
			b_need_remove_pid_file = true;
#endif
		}

		std::filesystem::path path_cur_exe = _mp::cfile::get_cur_exe_abs_path();
		std::filesystem::path path_cur = path_cur_exe.parent_path();

		/////////////////////////////////
		// setup log
		_mp::clog& log(_mp::clog::get_instance());
		if (b_run_by_cf) {
			std::wstring s_log_root_folder_except_backslash = _mp::ccoffee_path::get_path_of_coffee_logs_root_folder_except_backslash();
			log.config(
				s_log_root_folder_except_backslash
				,6
				, std::wstring(L"coffee_manager")
				, std::wstring(L"lpu230_update")
				, std::wstring(L"lpu230_update")
				);
		}
		else {
			log.config(
				path_cur.wstring() // without backslash
				,-1
				,std::wstring()
				, std::wstring()
				, std::wstring()
			);
		}

		log.enable(b_log_file);
		log.remove_log_files_older_then_now_day(3);

		/////////////////////////////////
		// find rom file
		std::string s_rom(s_abs_full_rom_file);

		if (!b_display && s_rom.empty()) {
			std::vector<std::filesystem::path> v_roms = _find_rom_files();
			if (v_roms.empty()) {
				log.log_fmt(L"[E] none rom file.\n");
				continue;
			}

			s_rom = v_roms[0].string();
		}

		///////////////////////////////////////////////////
		// setup info
		if (b_display) {
			log.log_fmt(L"[I] Enable UI\n");
		}
		log.log_fmt(L"[I] after done,change interface %ls.\n",
			_mp::cstring::get_unicode_from_mcsc(cupdater::get_string(lpu237_interface_after_update)).c_str()
			);
		
		log.log_fmt(L"[I] rom file : %ls\n",s_rom.c_str());
		if (s_device_path.empty()) {
			log.log_fmt(L"[I] target device : auto detected device.\n");
		}
		else{
			log.log_fmt(L"[I] target device : %ls.\n",s_device_path.c_str());
		}

		if (b_mmd1100_iso_mode) {
			log.log_fmt(L"[I] after updating,mmd1100 is set to iso mode.\n");
		}
		///////////////////////////////////////////////////

		cupdater updater(log,b_display, b_log_file, lpu237_interface_after_update);

		updater.set_rom_file(s_rom).set_device_path(s_device_path).set_mmd1100_iso_mode(b_mmd1100_iso_mode);

		updater.initial_update();

		if (!updater.start_update()) {
			log.log_fmt(L"[E] start_update().\n");
			continue;
		}

		updater.ui_main_loop();

		log.log_fmt(L"[I] end ui_main_loop().\n");
		n_result = EXIT_SUCCESS;
	}while(false);

#ifndef _WIN32
	//linux only
	if (b_need_remove_pid_file) {//normally! removed pid file.
		std::string s_pid_file = _mp::cstring::get_mcsc_from_unicode(s_pid_file_full_path);
		remove(s_pid_file.c_str());
	}
#endif

	return n_result;
}

std::vector<std::filesystem::path> _find_rom_files()
{
	std::vector<std::filesystem::path> rom_files;

	// 현재 실행 디렉터리
	std::filesystem::path current_exe = _mp::cfile::get_cur_exe_abs_path();

	std::filesystem::path current_dir = current_exe.parent_path();

	// 디렉터리 순회
	for (const auto& entry : std::filesystem::directory_iterator(current_dir)) {
		if (entry.is_regular_file() && entry.path().extension() == ".rom") {
			rom_files.push_back(entry.path());
		}
	}

	// 내림차순 정렬
	std::sort(rom_files.begin(), rom_files.end(),
		[](const std::filesystem::path& a, const std::filesystem::path& b) {
			return a.filename().string() > b.filename().string();
		});

	return rom_files;
}

void _signal_handler(int signum)
{
#ifndef _WIN32
	do {

		if (signum == SIGTERM) {
			// Handle termination gracefully
			_exit(EXIT_SUCCESS);
			continue;
		}
		if (signum == SIGHUP) {
			// notify reload initial file!
		}

	} while (false);
#endif
}
