#include <websocket/mp_win_nt.h>

#include <cstdio>
#include <main.h>

#include <mp_coffee.h>
#include <mp_cstring.h>
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
	bool b_need_remove_pid_file(false);

	int n_result(EXIT_FAILURE);
	std::shared_ptr<boost::interprocess::file_lock> ptr_file_lock_for_single_instance;
	std::wstring s_pipe_name_of_trace(_mp::_coffee::CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME);

	std::wstring s_server_certificate_file = cdef_const::get_certificate_file();
	std::wstring s_server_private_key_file = cdef_const::get_private_key_file();

	std::wstring s_log_folder_except_backslash = cdef_const::get_log_folder_except_backslash();
	std::wstring s_root_folder_except_backslash = cdef_const::get_root_folder_except_backslash();
	std::wstring s_pid_file_full_path = cdef_const::get_pid_file_full_path();

	do {
		if (!_mp::csystem::daemonize_on_linux(s_pid_file_full_path, std::wstring(), _signal_handler)) {
			n_result = cdef_const::exit_error_daemonize;
			continue;
		}

		b_need_remove_pid_file = true;
		
		//check single instance
		ptr_file_lock_for_single_instance = _mp::csystem::get_file_lock_for_single_instance(_mp::_coffee::CONST_S_COFFEE_MGMT_FILE_LOCK_FOR_SINGLE);
		if (!ptr_file_lock_for_single_instance) {
			n_result = cdef_const::exit_error_already_running;
			continue;//Another instance is already running.
		}

		//setup controller
		gptr_ctl_pipe = std::make_shared<_mp::cnamed_pipe>(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME, true);

		//setup tracing system
		_mp::clog& log(_mp::clog::get_instance());
		log.enable_trace(s_pipe_name_of_trace, true);

		//setup logging system
		log.config(s_log_folder_except_backslash, 6);
		log.remove_log_files_older_then_now_day(3);
		log.enable(true);
		log.log_fmt(L"[I] START LOGGING.\n");
		log.trace(L"[I] - START TRACING.\n");

		if (!gptr_ctl_pipe) {
			log.log_fmt(L"[E] %ls | create controller pipe.\n", __WFUNCTION__);
			log.trace(L"[E] - %ls | create controller pipe.\n", __WFUNCTION__);
			n_result = cdef_const::exit_error_create_ctl_pipe;
			continue;
		}

#ifndef _WIN32
		log.log_fmt(L"[I] - sid: %5d, pgid: %5d, pid: %5d, ppid: %5d.\n", (int)getsid(0), (int)getpgid(0), (int)getpid(), (int)getppid());
		log.trace(L"[I] - sid: %5d, pgid: %5d, pid: %5d, ppid: %5d.\n", (int)getsid(0), (int)getpgid(0), (int)getpid(), (int)getppid());
#endif

		bool b_debug = true;
		bool b_tls = true;
		unsigned short w_port = _mp::_ws_tools::WEBSOCKET_SECURITY_SERVER_PORT_COFFEE_MANAGER;
		int n_thread_for_server(1);

		_mp::cserver::get_instance(&_mp::clog::get_instance()).set_port(w_port)
			.set_ssl(b_tls)
			.set_cert_file(s_server_certificate_file)
			.set_private_key_file(s_server_private_key_file);

		if (!_mp::cserver::get_instance().start(n_thread_for_server, s_root_folder_except_backslash)) {
			log.log_fmt(L"[E] %ls | cserver::get_instance().start().\n", __WFUNCTION__);
			log.trace(L"[E] - %ls | cserver::get_instance().start().\n", __WFUNCTION__);
			n_result = cdef_const::exit_error_start_server;
			continue;
		}
		n_result = EXIT_SUCCESS;
		log.log_fmt(L"[I] %ls | cserver::get_instance().start().\n", __WFUNCTION__);
		//log.trace(L"[I] - %ls | cserver::get_instance().start().\n", __WFUNCTION__);
		std::wstring s_data;

		do {
			if (gptr_ctl_pipe->read(s_data)) {
				if (s_data.compare(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_REQ) == 0) {
					gb_run_main_loop = false;
					log.log_fmt(L"[I] %ls | req - server stop.\n", __WFUNCTION__);
					log.trace(L"[I] - %ls | req - server stop.\n", __WFUNCTION__);
					n_result = cdef_const::exit_info_ctl_pipe_requst_terminate;
					continue;
				}
				s_data.clear();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(_mp::_coffee::CONST_N_COFFEE_MGMT_SLEEP_INTERVAL_MMSEC));

		} while (gb_run_main_loop);

		_mp::cserver::get_instance().stop();
		log.log_fmt(L"[I] %ls | cserver::get_instance().stop().\n", __WFUNCTION__);
		log.trace(L"[I] - %ls | cserver::get_instance().stop().\n", __WFUNCTION__);

	} while (false);

#ifndef _WIN32
	//linux only
	if (b_need_remove_pid_file) {//normally! removed pid file.
		std::string s_pid_file = _mp::cstring::get_mcsc_from_unicode(s_pid_file_full_path);
		remove(s_pid_file.c_str());
	}
#endif
	return n_result;
}


void _signal_handler(int signum)
{
#ifndef _WIN32
	do {

		if (signum == SIGTERM) {
			// Handle termination gracefully
			if (!gptr_ctl_pipe) {
				_exit(EXIT_SUCCESS);
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
