#include <websocket/mp_win_nt.h>

#include <main.h>

#include <mp_coffee.h>
#include <mp_clog.h>
#include <mp_csystem_.h>
#include <websocket/mp_cws_server.h>
#include <server/mp_cserver_.h>


static _mp::cnamed_pipe::type_ptr gptr_ctl_pipe;
static std::atomic_bool gb_run_main_loop(true);

static void _signal_handler(int signum);

/**
 * run the wss server.
*/
int main_wss(const _mp::type_set_wstring &set_parameters)
{
	int n_result(EXIT_FAILURE);
	std::shared_ptr<boost::interprocess::file_lock> ptr_file_lock_for_single_instance;
	std::wstring s_pipe_name_of_trace(_mp::_coffee::CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME);
#ifdef _WIN32
	std::wstring s_log_folder_except_backslash = L"C:\\ProgramData\\Elpusk\\00000006\\elpusk-hid-d\\log";
	std::wstring s_certificate_file = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.crt";
	std::wstring s_private_key_file = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\cert\\coffee_server.key";
	std::wstring s_root_folder_except_backslash = L"C:\\job\\library.cpp.lpu230\\shared\\data_for_debug\\win\\ProgramData\\elpusk\\00000006\\vroot";
#else
	std::wstring s_log_folder_except_backslash = L"/home/tester/fordebug/var/log/elpusk/00000006/elpusk-hid-d";
	std::wstring s_certificate_file = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.crt";
	std::wstring s_private_key_file = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/Debug/coffee_server.key";
	std::wstring s_root_folder_except_backslash = L"/home/tester/projects/LiElpuskHidDaemon/bin/x64/vroot";
#endif

	do {/*
		if (!_mp::csystem::daemonize_on_linux(L"elpusk-hid-daemon", _signal_handler)) {
			continue;
		}
		*/
		//check single instance
		ptr_file_lock_for_single_instance = _mp::csystem::get_file_lock_for_single_instance(_mp::_coffee::CONST_S_COFFEE_MGMT_FILE_LOCK_FOR_SINGLE);
		if (!ptr_file_lock_for_single_instance) {
			//std::wcout << L"Another instance is already running." << std::endl;
			continue;//Another instance is already running.
		}

		//setup controller
		gptr_ctl_pipe = std::make_shared<_mp::cnamed_pipe>(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME, true);

		//setup tracing system
		_mp::clog& log(_mp::clog::get_instance());
		log.enable_trace(s_pipe_name_of_trace, true);

		//setup logging system
		log.config(s_log_folder_except_backslash, 3);
		log.remove_log_files_older_then_now_day(3);
		log.enable(true);
		log.log(L"[I] START LOGGING.\n");

		if (!gptr_ctl_pipe) {
			log.log(L"[E] %ls | create controller pipe.\n", __WFUNCTION__);
			log.trace(L"[E] %ls | create controller pipe.\n", __WFUNCTION__);
			continue;
		}

#ifndef _WIN32
		log.trace(L"[i] sid: %5d, pgid: %5d, pid: %5d, ppid: %5d.\n", (int)getsid(0), (int)getpgid(0), (int)getpid(), (int)getppid());
#endif

		bool b_debug = true;
		bool b_tls = true;
		unsigned short w_port = _mp::_ws_tools::WEBSOCKET_SECURITY_SERVER_PORT_COFFEE_MANAGER;
		int n_thread_for_server(1);

		_mp::cserver::get_instance(&_mp::clog::get_instance()).set_port(w_port)
			.set_ssl(b_tls)
			.set_cert_file(s_certificate_file)
			.set_private_key_file(s_private_key_file);
		if (!_mp::cserver::get_instance().start(n_thread_for_server, s_root_folder_except_backslash)) {
			log.log(L"[E] %ls | cserver::get_instance().start().\n", __WFUNCTION__);
			log.trace(L"[E] %ls | cserver::get_instance().start().\n", __WFUNCTION__);
			continue;
		}

		n_result = EXIT_SUCCESS;
		log.log(L"[I] %ls | cserver::get_instance().start().\n", __WFUNCTION__);
		log.trace(L"[I] %ls | cserver::get_instance().start().\n", __WFUNCTION__);
		std::wstring s_data;

		do {
			if (gptr_ctl_pipe->read(s_data)) {
				if (s_data.compare(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_REQ) == 0) {
					gb_run_main_loop = false;
					log.log(L"[I] %ls | req - server stop.\n", __WFUNCTION__);
					log.trace(L"[I] %ls | req - server stop.\n", __WFUNCTION__);
					continue;
				}
				s_data.clear();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(_mp::_coffee::CONST_N_COFFEE_MGMT_SLEEP_INTERVAL_MMSEC));

		} while (gb_run_main_loop);

		_mp::cserver::get_instance().stop();
		log.log(L"[I] %ls | cserver::get_instance().stop().\n", __WFUNCTION__);
		log.trace(L"[I] %ls | cserver::get_instance().stop().\n", __WFUNCTION__);

	} while (false);

	return n_result;
}


void _signal_handler(int signum)
{
#ifndef _WIN32
	do {

		if (signum == SIGTERM) {
			// Handle termination gracefully
			if (!gptr_ctl_pipe) {
				exit(EXIT_SUCCESS);
			}

			gb_run_main_loop = false;
			continue;//terminated by main loop
		}
		if (signum == SIGHUP) {
			// notify reload initial file!
		}

	} while (false);
#endif
}
