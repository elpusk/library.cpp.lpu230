#include <websocket/mp_win_nt.h>

#include <cstdio>
#include <main.h>

#include <mp_coffee.h>
#include <mp_cstring.h>
#include <mp_clog.h>
#include <mp_csystem_.h>
#include <websocket/mp_cws_server.h>
#include <server/mp_cserver_.h>

#include <cfile_coffee_manager_ini.h>


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

	std::wstring s_log_root_folder_except_backslash = _mp::ccoffee_path::get_path_of_coffee_logs_root_folder_except_backslash();
	std::wstring s_root_folder_except_backslash = cdef_const::get_root_folder_except_backslash();
	std::wstring s_pid_file_full_path = cdef_const::get_pid_file_full_path();

	std::wstring s_ini_file_full_path_of_coffee_mgmt(_mp::ccoffee_path::get_path_of_coffee_mgmt_ini_file());

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

		// load coffee manager ini file
		cfile_coffee_manager_ini& ini(cfile_coffee_manager_ini::get_instance());
		bool b_ini_load = ini.load_definition_file();


		//setup tracing system
		_mp::clog& log(_mp::clog::get_instance());
		log.enable_trace(s_pipe_name_of_trace, ini.get_log_enable()); //enable trace by server mode

		//setup logging system
		log.config(s_log_root_folder_except_backslash, 6, std::wstring(L"coffee_manager"),std::wstring(L"elpusk-hid-d"), std::wstring(L"elpusk-hid-d"));
		log.remove_log_files_older_then_now_day(3);
		log.enable(ini.get_log_enable());
#ifdef _DEBUG
		log.log_fmt(L"[I] START LOGGING ON DEBUG.\n");
		log.trace(L"[I] START TRACING ON DEBUG.\n");
#else
		log.log_fmt(L"[I] START LOGGING ON RELEASE.\n");
		log.trace(L"[I] START TRACING ON RELEASE.\n");
#endif

		if (!gptr_ctl_pipe) {
			log.log_fmt(L"[E] %ls | create controller pipe.\n", __WFUNCTION__);
			log.trace(L"[E] %ls | create controller pipe.\n", __WFUNCTION__);
			n_result = cdef_const::exit_error_create_ctl_pipe;
			continue;
		}

#ifndef _WIN32
		log.log_fmt(L"[I] sid: %5d, pgid: %5d, pid: %5d, ppid: %5d.\n", (int)getsid(0), (int)getpgid(0), (int)getpid(), (int)getppid());
		log.trace(L"[I] sid: %5d, pgid: %5d, pid: %5d, ppid: %5d.\n", (int)getsid(0), (int)getpgid(0), (int)getpid(), (int)getppid());
#endif
		// loading ini
		if (b_ini_load) {
			log.log_fmt(L"[=============.\n");
			log.log_fmt(L"[I] loaded ini file = %ls.\n", ini.get_ini_file_full_path().c_str());
			log.log_fmt(L"[I] ini value : name = %ls.\n", ini.get_name().c_str());
			log.log_fmt(L"[I] ini value : version = %ls.\n", ini.get_version().c_str());
			log.log_fmt(L"[I] ini value : date = %ls.\n", ini.get_date().c_str());

			if (ini.get_log_enable()) {
				log.log_fmt(L"[I] ini value : log = enable.\n");
			}
			else {
				log.log_fmt(L"[I] ini value : log = disable.\n");
			}

			if (ini.get_tls_enable()) {
				log.log_fmt(L"[I] ini value : tls v1.3 = enable.\n" );
			}
			else {
				log.log_fmt(L"[I] ini value : tls v1.3 = disable.\n");
			}
			log.log_fmt(L"[I] ini value : server port = %d.\n", ini.get_server_port());
			log.log_fmt(L"[I] ini value : server wait timeout for websocket upgrade req of client = %lld [mmsec].\n", ini.get_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client());
			log.log_fmt(L"[I] ini value : server wait timeout for_ dle = %lld.\n", ini.get_msec_timeout_ws_server_wait_for_idle());
			log.log_fmt(L"[I] ini value : server wait timeout for ssl handshake complete = %lld [mmsec].\n", ini.get_msec_timeout_ws_server_wait_for_ssl_handshake_complete());
			log.log_fmt(L"[=============.\n");
			//
			log.trace(L"[=============.\n");
			log.trace(L"[I] loaded ini file = %ls.\n", ini.get_ini_file_full_path().c_str());
			log.trace(L"[I] ini value : name = %ls.\n", ini.get_name().c_str());
			log.trace(L"[I] ini value : version = %ls.\n", ini.get_version().c_str());
			log.trace(L"[I] ini value : date = %ls.\n", ini.get_date().c_str());

			if (ini.get_log_enable()) {
				log.trace(L"[I] ini value : log = enable.\n");
			}
			else {
				log.trace(L"[I] ini value : log = disable.\n");
			}

			if (ini.get_tls_enable()) {
				log.trace(L"[I] ini value : tls v1.3 = enable.\n");
			}
			else {
				log.trace(L"[I] ini value : tls v1.3 = disable.\n");
			}
			log.trace(L"[I] ini value : server port = %d.\n", ini.get_server_port());
			log.trace(L"[I] ini value : server wait timeout for websocket upgrade req of client = %lld [mmsec].\n", ini.get_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client());
			log.trace(L"[I] ini value : server wait timeout for_ dle = %lld.\n", ini.get_msec_timeout_ws_server_wait_for_idle());
			log.trace(L"[I] ini value : server wait timeout for ssl handshake complete = %lld [mmsec].\n", ini.get_msec_timeout_ws_server_wait_for_ssl_handshake_complete());
			log.trace(L"[=============.\n");
		}

		bool b_tls = ini.get_tls_enable();
		unsigned short w_port = (unsigned short)ini.get_server_port();
		int n_thread_for_server(1);

		_mp::cserver::get_instance(&_mp::clog::get_instance()).set_port(w_port)
			.set_ssl(b_tls)
			.set_cert_file(s_server_certificate_file)
			.set_private_key_file(s_server_private_key_file)
			.set_timeout_websocket_upgrade_req(ini.get_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client())
			.set_timeout_idle(ini.get_msec_timeout_ws_server_wait_for_idle())
			.set_timeout_ssl_handshake_complete(ini.get_msec_timeout_ws_server_wait_for_ssl_handshake_complete());

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
