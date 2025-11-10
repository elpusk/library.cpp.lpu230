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
#include <mp_coffee_pipe.h>

#include <cdev_lib.h>
#include "update.h"

static void _notify_cleanup_to_server_on_exit();

class _cleanup_anager {
public:
	_cleanup_anager() : 
		m_b_need_restore_console_mode(false)
		, m_dw_mode(0)
	{
		// 리소스 획득 (예: 임시 파일 열기, 연결 설정 등)
	}

	~_cleanup_anager() {
		// 프로그램 종료 시 반드시 실행되어야 하는 클린업 코드
		// 여기에 정리 코드를 배치합니다.
		do {
#ifdef _WIN32
			//SetConsoleCtrlHandler(NULL, FALSE);  // 모든 핸들러 제거 → Ctrl+C 다시 동작

			HWND consoleWindow = GetConsoleWindow();
			if (consoleWindow) {
				ShowWindow(consoleWindow, SW_SHOW); // 콘솔 창 표시(복구)
				HMENU sysMenu = GetSystemMenu(consoleWindow, TRUE);// 닫기 버튼 복원
			}

			if (m_b_need_restore_console_mode) {
				// 콘솔 모드 복원
				auto h_std = GetStdHandle(STD_INPUT_HANDLE);
				if (INVALID_HANDLE_VALUE != h_std) {
					SetConsoleMode(h_std, m_dw_mode);
				}
			}
#endif
			_notify_cleanup_to_server_on_exit();

		} while (false);
	}

	void set_need_restore_console_mode(bool b_need_restore, unsigned long dw_mode) 
	{
		m_b_need_restore_console_mode = b_need_restore;
		m_dw_mode = dw_mode;
	}

private:
	bool m_b_need_restore_console_mode;
	unsigned long m_dw_mode;
};

static int gn_result(EXIT_FAILURE);

static std::vector<std::filesystem::path> _find_rom_files();

#ifdef _WIN32
static BOOL WINAPI _console_handler(DWORD signal);
#else
static void _signal_handler(int signum);
#endif

/**
* @param b_display : true - display ui, false - no display ui
* @return first - success setup display
* @return second - need to remove pid file when exit
*/
static std::tuple<bool,bool,unsigned long,bool> setup_display(bool b_display);

static void setup_log(bool b_run_by_coffee_manager, bool b_enable);

/**
*
* @return first - true : success processing, second - error code. if first is true, second must be EXIT_SUCCESS.
*/
static std::pair<bool, int> setup_dev_io_dll(bool b_run_by_coffee_manager, _mp::clog& log);


/**
*
* @return first - true : success processing, second - error code. if first is true, second must be EXIT_SUCCESS.
*/
static std::pair<bool, int> setup_rom_dll(bool b_run_by_coffee_manager,_mp::clog& log);

/**
* @brief 사용자로 부터 받은 command option 울 받아.
* 
*	GUI 를 생성하고, 업데이트 manager 를 실행.
*	
*	software component 검증.
*/
int update_main
(
	const std::string& s_abs_full_rom_file,
	const std::string& s_device_path,
	bool b_display,
	bool b_log_file,
	bool b_mmd1100_iso_mode,
	cshare::Lpu237Interface lpu237_interface_after_update,
	bool b_run_by_cf
)
{
	std::wstring s_pid_file_full_path;
	bool b_need_remove_pid_file(false);
	bool b_result_setup_display(false);
	bool b_need_restore_console_mode(false);
	unsigned long dw_original_console_mode(0);

	do {
		_cleanup_anager _cls_mgmt;

		setup_log(b_run_by_cf, b_log_file);
		// from this line, _mp::clog::get_instance() can be used.
		_mp::clog& log(_mp::clog::get_instance());

		if (!_mp::csystem::is_running_as_admin_or_root()) {
			log.log_fmt(L"[E] need to run as admin(windows) or root(linux).\n");
			gn_result = _mp::exit_error_need_admin_or_root_for_running;
			if (b_display) {
				// setup_display() 후에 하면 안됌, setup_display() 에서 admin 권한을 요구하는 함수 사용 가능성있음.
				std::cout << "Error: need to run as admin(windows) or root(linux)." << std::endl;
			}
			continue;//error
		}
		std::tie(b_result_setup_display,b_need_remove_pid_file, dw_original_console_mode, b_need_restore_console_mode) = setup_display(b_display);
		if(!b_result_setup_display) {
			log.log_fmt(L"[E] setup_display() failed.\n");
			gn_result = _mp::exit_error_setup_display;
			if (b_display) {
				std::cout << "Error: setup display failed." << std::endl;
			}
			continue;
		}
		if (b_need_restore_console_mode) {
			_cls_mgmt.set_need_restore_console_mode(b_need_restore_console_mode, dw_original_console_mode);
		}

		auto result_dev_io_dll = setup_dev_io_dll(b_run_by_cf, log);//logging 도 포함.
		if (!result_dev_io_dll.first) {
			gn_result = result_dev_io_dll.second;
			continue;
		}
		// from this line, cdev_lib::get_instance() can be used. 

		auto result_rom_dll = setup_rom_dll(b_run_by_cf, log);//logging 도 포함.
		if (!result_rom_dll.first) {
			gn_result = result_rom_dll.second;
			continue;
		}

		// from this line, CHidBootManager::GetInstance() can be used.
		// from this line, cshare::get_instance() can be used. because cshare use CHidBootManager.

		// b_run_by_cf 조건 조사, b_run_by_cf 가 enable 되어 있으면 아래의 조건을 만족해야 한다.
		// 1. rom 파일 path 가 주어져야 한다.
		// 2. device path 가 주어져야 한다.
		if (b_run_by_cf) {
			if (s_abs_full_rom_file.empty()) {
				log.log_fmt(L"[E] b_run_by_cf is set, but none rom file.\n");
				gn_result = _mp::exit_error_run_by_cf_rom_file;
				continue;
			}
			if (s_device_path.empty()) {
				log.log_fmt(L"[E] b_run_by_cf is set, but none devie.\n");
				gn_result = _mp::exit_error_run_by_cf_device;
				continue;
			}
			log.log_fmt(L"[I] run by cf is on.\n");
			cshare::get_instance().set_run_by_cf(b_run_by_cf);
		}

		cshare::get_instance().set_display_ui(b_display);
		///////////////////////////////////////////////////
		// setup info
		if (b_display) {
			log.log_fmt(L"[I] Enable UI\n");
		}
		else {
			log.log_fmt(L"[I] Disable UI\n");
		}

		log.log_fmt(L"[I] after done,change interface %ls.\n",
			_mp::cstring::get_unicode_from_mcsc(cshare::get_string(lpu237_interface_after_update)).c_str()
			);
		cshare::get_instance().set_lpu23x_interface_change_after_update(lpu237_interface_after_update);

		
		if (s_abs_full_rom_file.empty()) {
			log.log_fmt(L"[I] rom file : auto detected rom.\n");
		}
		else {
			log.log_fmt(L"[I] rom file : %s.\n", s_abs_full_rom_file.c_str());
			cshare::get_instance().set_rom_file_abs_full_path(s_abs_full_rom_file);//커맨드라인에서 주어지는 rom/bin 파일 저장.
		}
		//
		if (s_device_path.empty()) {
			log.log_fmt(L"[I] target device : auto detected device.\n");
		}
		else{
			log.log_fmt(L"[I] target device : %s.\n",s_device_path.c_str());
			cshare::get_instance().set_device_path(s_device_path);//사용자로 부터 받은 장비 경로 저장.
		}

		if (b_mmd1100_iso_mode) {
			log.log_fmt(L"[I] after updating,mmd1100 is set to iso mode.\n");
			cshare::get_instance().set_iso_mode_after_update(b_mmd1100_iso_mode);
		}

		//
		
		///////////////////////////////////////////////////
		// 여기서 cupdater 를 shared_ptr 로 생성하는 이유는 
		// cupdater 에서 생성하는 ftxui 라이브러리에서 콘솔 모드를 변경했다가 복원하는데,
		// 이 함수의 지역변수인 _cls_mgmt 도 콘솔 모드를 복원하기 때문에, 복원 순서를 맞추기 위해
		// _cls_mgmt 보다 먼저 cupdater instance 가 소멸되도록 하기 위함임.
		cupdater::type_ptr ptr_updater(new cupdater(log,b_display, b_log_file));

		if (!ptr_updater->initial_update()) {
			log.log_fmt(L"[E] initial_update().\n");
			continue;
		}

		if (b_display) {
			ptr_updater->ui_main_loop();
		}
		else {
			ptr_updater->non_ui_main_loop();
		}

		ptr_updater.reset();

		log.log_fmt(L"[I] end ui_main_loop().\n");
		gn_result = EXIT_SUCCESS;
	}while(false);

#ifndef _WIN32
	//linux only
	if (b_need_remove_pid_file) {//normally! removed pid file.
		std::string s_pid_file = _mp::cstring::get_mcsc_from_unicode(s_pid_file_full_path);
		remove(s_pid_file.c_str());
	}
#endif

	return gn_result;
}

void _notify_cleanup_to_server_on_exit()
{
	// 프로그램 종료 시 반드시 실행되어야 하는 클린업 코드
	do {

		cshare& sh(cshare::get_instance());

		int n_vid(0), n_pid(0);
		bool b_stop(false);

		std::tie(b_stop, n_vid, n_pid) = sh.is_executed_server_stop_use_target_dev();
		if (!b_stop) {
			_mp::clibhid_dev_info plugin_dev_info_after_update = sh.get_plugin_device_info_after_update();
			if(plugin_dev_info_after_update.is_valid()){
				n_vid = (int)plugin_dev_info_after_update.get_vendor_id();
				n_pid = (int)plugin_dev_info_after_update.get_product_id();
			}
			else {
				continue;
			}
		}
		
		sh.clear_executed_server_stop_use_target_dev();
		//
		_mp::cnamed_pipe::type_ptr ptr_tx_ctl_pipe, ptr_rx_ctl_pipe;
		ptr_tx_ctl_pipe = std::make_shared<_mp::cnamed_pipe>(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME, false);
		if (!ptr_tx_ctl_pipe) {
			continue;
		}
		if (!ptr_tx_ctl_pipe->is_ini()) {
			continue;
		}
		ptr_rx_ctl_pipe = std::make_shared<_mp::cnamed_pipe>(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME_OF_SERVER_RESPONSE, false);
		if (!ptr_rx_ctl_pipe) {
			continue;
		}
		if (!ptr_rx_ctl_pipe->is_ini()) {
			continue;
		}

		// 서버 control pipe 에 연결 설공.
		std::wstring s_req_ctl_pip = _mp::ccoffee_pipe::generate_ctl_request_for_cancel_considering_dev_as_removed(
			n_vid,n_pid
		);
		if (s_req_ctl_pip.empty()) {
			continue;
		}
		if (!ptr_tx_ctl_pipe->write(s_req_ctl_pip)) {
			continue;
		}

		uint32_t n_server_rsp_check_interval_mm = 10;
		int n_n_server_rsp_check_times = 300; // n_server_rsp_check_interval_mm 를 몇번 검사 할 건지.

		std::wstring s_rx;
		int i = 0;
		for (; i < n_n_server_rsp_check_times; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(n_server_rsp_check_interval_mm)); // 서버의 작업 시간을 위한 sleep.
			if (!ptr_rx_ctl_pipe->read(s_rx)) {
				continue;
			}
			if (s_rx.compare(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_RSP_START_DEV) == 0) {
				// 정상 응답 성공.
				_mp::clog::get_instance().log_fmt(L"[D]_cleanup : (vid:pid) = (0x%x,0x%x).\n", n_vid,n_pid);
				break; //exit for
			}
		}//end for

	} while (false);
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

#ifdef _WIN32
BOOL WINAPI _console_handler(DWORD signal)
{
	cshare& sh(cshare::get_instance());

	switch (signal) {
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:

	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		if (sh.is_possible_exit()) {
			ATLTRACE("EXIT REQ!!!!!\n");
			return FALSE;// 종료
		}
		else {
			ATLTRACE("EXIT REQ is ignored!!!!!\n");
			return TRUE; // 종료 무시
		}
	default:
		return FALSE;
	}
}
#else
void _signal_handler(int signum)
{
	do {
		cshare& sh(cshare::get_instance());
		if (sh.is_possible_exit()) 
		{
			_mp::clog::get_instance().log_fmt(L"[D]_signal_handler : enabled exit = %d.\n", signum);
			switch (signum)
			{
			case SIGINT:// Handle Ctrl+C gracefully
			case SIGHUP ://close button

			case SIGTERM:
			case SIGTSTP:
				_notify_cleanup_to_server_on_exit();
				_exit(gn_result);
				break;
			default:
				break;//ignore
			}
			continue;
		}
		else {
			_mp::clog::get_instance().log_fmt(L"[D]_signal_handler : disabled exit = %d.\n", signum);
			_notify_cleanup_to_server_on_exit();
			_exit(gn_result);
			continue;
		}

	} while (false);
}
#endif

std::tuple<bool, bool, unsigned long, bool> setup_display(bool b_display)
{
	bool b_result(false);
	bool b_need_remove_pid_file(false);
	unsigned long dw_original_mode(0);
	bool b_need_restore_console_mode(false);

	do {
		if (!b_display) {
#ifdef _WIN32
			HWND consoleWindow = GetConsoleWindow();
			if (consoleWindow == NULL) {
				continue; // error
			}
			ShowWindow(consoleWindow, SW_HIDE); // 콘솔 창 숨기기
#else
			// 이후 출력 억제
			std::cout.setstate(std::ios_base::failbit);
			std::cerr.setstate(std::ios_base::failbit);

			std::wstring s_pid_file_full_path = _mp::_coffee::CONST_S_PID_FILE_FULL_PATH_LPU230_UPDATE;
			if (_mp::csystem::daemonize_on_linux(s_pid_file_full_path, std::wstring(), _signal_handler)) {
				b_need_remove_pid_file = true;
			}
#endif
		}
		else {
#ifdef _WIN32
			HWND consoleWindow = GetConsoleWindow();
			if (consoleWindow == NULL) {
				continue; // error
			}
			ShowWindow(consoleWindow, SW_SHOW); // 콘솔 창 표시

			HMENU sysMenu = GetSystemMenu(consoleWindow, FALSE);
			if (sysMenu == NULL) {
				continue; // error
			}
			// 시스템 메뉴에서 '닫기(SC_CLOSE)' 항목을 제거하여 버튼을 비활성화합니다.
			DeleteMenu(sysMenu, SC_CLOSE, MF_BYCOMMAND);
			
			DWORD mode(0);
			auto h_std = GetStdHandle(STD_INPUT_HANDLE);
			if (INVALID_HANDLE_VALUE != h_std) {
				if (GetConsoleMode(h_std, &mode)) {
					dw_original_mode = mode;
					mode &= ~ENABLE_PROCESSED_INPUT;  // CTRL+C 를 입력으로 전달, ftxui lib 에서 ctrl-c 처리 하도록 함.
					if (SetConsoleMode(h_std, mode)) {
						b_need_restore_console_mode = true;
						ATLTRACE("++++++++++ OK set console mode\n");
					}
					else {
						ATLTRACE("++++++++++ Could not set console mode\n");
					}
				}
				else {
					ATLTRACE("++++++++++ Could not get console mode\n");
				}
			}
			else {
				ATLTRACE("++++++++++ Could not get std handle\n");
			}
			/* 위에서 ENABLE_PROCESSED_INPUT 제거 했으므로, 아래 코드는 필요 없음.
			SetConsoleCtrlHandler(NULL, FALSE); //등록 전 기본값 복원.
			// Ctrl+C , Ctrl+Break 이벤트에 의한 종료 막기 핸들러 등록
			if (!SetConsoleCtrlHandler(_console_handler, TRUE)) {
				ATLTRACE("Could not set control handler\n");
			}
			else {
				ATLTRACE("OK set control handler\n");
			}
			*/
#else
			// SIGINT (Ctrl+C) 처리
			signal(SIGINT, _signal_handler);

			// SIGHUP (터미널 창 닫기 또는 세션 종료) 처리
			signal(SIGHUP, _signal_handler);

			// SIGTERM (시스템 종료 또는 kill 명령) 처리
			signal(SIGTERM, _signal_handler);

			// SIGTSTP (Ctrl+Z, 일시 중지)를 무시하여 백그라운드 전환 방지
			signal(SIGTSTP, SIG_IGN);
#endif
		}

		b_result = true;
	} while (false);

	return std::make_tuple(b_result,b_need_remove_pid_file,dw_original_mode,b_need_restore_console_mode);
}

void setup_log(bool b_run_by_coffee_manager, bool b_enable)
{
	std::filesystem::path path_cur_exe = _mp::cfile::get_cur_exe_abs_path();
	std::filesystem::path path_cur = path_cur_exe.parent_path();

	/////////////////////////////////
	// setup log
	_mp::clog& log(_mp::clog::get_instance());
	if (b_run_by_coffee_manager) {
		std::wstring s_log_root_folder_except_backslash = _mp::ccoffee_path::get_path_of_coffee_logs_root_folder_except_backslash();
		log.config(
			s_log_root_folder_except_backslash
			, 6
			, std::wstring(L"coffee_manager")
			, std::wstring(L"lpu230_update")
			, std::wstring(L"lpu230_update")
		);
	}
	else {
#ifdef _DEBUG
		log.config(
			path_cur.wstring() // without backslash
			, -1
			, std::wstring()
			, std::wstring()
			, std::wstring()
		);
#else 
		//release
		std::wstring s_coffee_mgmt_folder_except_backslash = _mp::ccoffee_path::get_path_of_coffee_mgmt_folder_except_backslash();
		if (s_coffee_mgmt_folder_except_backslash.compare(path_cur.wstring()) == 0) {
			// 현재 업데이터가 실행 중인 곳이 installer 에 의해 설치된 곳이면, 로그를 b_run_by_coffee_manager 에 의한 곳과 동일한 곳에 만듬.
			std::wstring s_log_root_folder_except_backslash = _mp::ccoffee_path::get_path_of_coffee_logs_root_folder_except_backslash();
			log.config(
				s_log_root_folder_except_backslash
				, 6
				, std::wstring(L"coffee_manager")
				, std::wstring(L"lpu230_update")
				, std::wstring(L"lpu230_update")
			);
		}
		else {
			log.config(
				path_cur.wstring() // without backslash
				, -1
				, std::wstring()
				, std::wstring()
				, std::wstring()
			);

		}
#endif
	}

	log.enable(b_enable);
	log.remove_log_files_older_then_now_day(3);

}

/**
*
* @return first - true : success processing, second - error code. if first is true, second must be EXIT_SUCCESS.
*/
std::pair<bool, int> setup_dev_io_dll(bool b_run_by_coffee_manager,_mp::clog& log)
{
	bool b_result(false);
	int n_error_code(EXIT_SUCCESS);

	std::filesystem::path path_cur_exe = _mp::cfile::get_cur_exe_abs_path();
	std::filesystem::path path_cur = path_cur_exe.parent_path();

	do {
		// load dev_lib.dll(.so)
		std::wstring s_dev_lib_dll_abs_full_path;
		if (b_run_by_coffee_manager) {
			s_dev_lib_dll_abs_full_path = _mp::ccoffee_path::get_abs_full_path_of_dev_lib_dll();
		}
		else {//단독 실행의 경우
			s_dev_lib_dll_abs_full_path = path_cur.wstring();
#ifdef _WIN32
			s_dev_lib_dll_abs_full_path += L"\\dev_lib.dll";
#else
			s_dev_lib_dll_abs_full_path += L"/libdev_lib.so";
#endif
		}

		if (!cdev_lib::get_instance().load(s_dev_lib_dll_abs_full_path, &log)) {
			if (b_run_by_coffee_manager) {
				log.log_fmt(L"[E] %ls | load dev_lib.dll(.so) | %ls.\n", __WFUNCTION__, s_dev_lib_dll_abs_full_path.c_str());
				log.trace(L"[E] %ls | load dev_lib.dll(.so) | %ls.\n", __WFUNCTION__, s_dev_lib_dll_abs_full_path.c_str());
				n_error_code = _mp::exit_error_load_dev_lib;
				continue;
			}
			//단독 실행의 경우.
			//현재 디렉토리에서 라이브러리 못찾으면, 기본path 에서 찾기.
			s_dev_lib_dll_abs_full_path = _mp::ccoffee_path::get_abs_full_path_of_dev_lib_dll();
			if (!cdev_lib::get_instance().load(s_dev_lib_dll_abs_full_path, &log)) {
				log.log_fmt(L"[E] %ls | load dev_lib.dll(.so) | %ls.\n", __WFUNCTION__, s_dev_lib_dll_abs_full_path.c_str());
				log.trace(L"[E] %ls | load dev_lib.dll(.so) | %ls.\n", __WFUNCTION__, s_dev_lib_dll_abs_full_path.c_str());
				n_error_code = _mp::exit_error_load_dev_lib;
				continue;
			}
		}

		b_result = true;
	} while (false);
	return std::make_pair(b_result, n_error_code);
}


/**
*
* @return first - true : success processing, second - error code. if first is true, second must be EXIT_SUCCESS.
*/
std::pair<bool, int> setup_rom_dll(bool b_run_by_coffee_manager,_mp::clog& log)
{
	bool b_result(false);
	int n_error_code(EXIT_SUCCESS);

	std::filesystem::path path_cur_exe = _mp::cfile::get_cur_exe_abs_path();
	std::filesystem::path path_cur = path_cur_exe.parent_path();

	do {
		std::filesystem::path path_rom_dll_full;
		if (b_run_by_coffee_manager) {
			path_rom_dll_full = _mp::ccoffee_path::get_abs_full_path_of_rom_dll();
		}
		else {//단독 실행의 경우
			path_rom_dll_full = path_cur.wstring();
#ifdef _WIN32
			path_rom_dll_full += L"\\tg_rom.dll";
#else
			path_rom_dll_full += L"/libtg_rom.so";
#endif
		}

		if (!CHidBootManager::GetInstance()->load_rom_library(path_rom_dll_full)) {
			if (b_run_by_coffee_manager) {
				log.log_fmt(L"[E] setup rom library(%ls).\n", path_rom_dll_full.wstring().c_str());
				log.trace(L"[E] setup rom library(%ls).\n", path_rom_dll_full.wstring().c_str());
				n_error_code = _mp::exit_error_load_rom_lib;
				continue;
			}
			//단독 실행의 경우.
			//현재 디렉토리에서 라이브러리 못찾으면, 기본path 에서 찾기.
			path_rom_dll_full = _mp::ccoffee_path::get_abs_full_path_of_rom_dll();
			if (!CHidBootManager::GetInstance()->load_rom_library(path_rom_dll_full)) {
				log.log_fmt(L"[E] setup rom library(%ls).\n", path_rom_dll_full.wstring().c_str());
				log.trace(L"[E] setup rom library(%ls).\n", path_rom_dll_full.wstring().c_str());
				n_error_code = _mp::exit_error_load_rom_lib;
				continue;
			}
		}

		b_result = true;
	} while (false);
	return std::make_pair(b_result, n_error_code);
}