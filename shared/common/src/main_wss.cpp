#include <websocket/mp_win_nt.h>

#if !defined(_WIN32) && defined(_SET_THREAD_NAME_)
#include <pthread.h>
#endif

#include <cstdio>
#include <array>
#include <main.h>

#include <mp_coffee_pipe.h>
#include <mp_cstring.h>
#include <mp_clog.h>
#include <mp_csystem_.h>
#include <websocket/mp_cws_server.h>
#include <server/mp_cserver_.h>
#include <hid/mp_clibhid.h>

#include <cfile_coffee_manager_ini.h>
#include <cdev_lib.h>


static _mp::cnamed_pipe::type_ptr gptr_tx_ctl_pipe, gptr_rx_ctl_pipe;
static std::atomic_bool gb_run_main_loop(true);

static int gn_result(EXIT_FAILURE);
static void _signal_handler(int signum);

#ifdef _WIN32
static BOOL WINAPI _console_handler(DWORD signal);
#endif

/**
* @brief get long long type interval from control pipe data.
* @param s_pipe_data - data from control pipe.
* @param s_expected_prefix - expected prefix of the data.
* @return pair<bool, long long> - first: true if data is valid, false otherwise; second: the interval in milliseconds.
*/
static std::pair<bool, long long> _get_interval_from_ctl_pipe(const std::wstring& s_pipe_data,const std::wstring & s_expected_prefix);

/**
 * run the wss server.
*/
int main_wss(const _mp::type_set_wstring &set_parameters)
{
	bool b_need_remove_pid_file(false);

	std::shared_ptr<boost::interprocess::file_lock> ptr_file_lock_for_single_instance;
	std::wstring s_pipe_name_of_trace(_mp::_coffee::CONST_S_COFFEE_MGMT_TRACE_PIPE_NAME);

	std::wstring s_server_certificate_file = cdef_const::get_certificate_file();
	std::wstring s_server_private_key_file = cdef_const::get_private_key_file();

	std::wstring s_log_root_folder_except_backslash = _mp::ccoffee_path::get_path_of_coffee_logs_root_folder_except_backslash();
	std::wstring s_root_folder_except_backslash = cdef_const::get_root_folder_except_backslash();
	std::wstring s_pid_file_full_path = cdef_const::get_pid_file_full_path();

	std::wstring s_ini_file_full_path_of_coffee_mgmt(_mp::ccoffee_path::get_path_of_coffee_mgmt_ini_file());

	std::wstring s_dev_lib_dll_abs_full_path(_mp::ccoffee_path::get_abs_full_path_of_dev_lib_dll());

	long long ll_server_worker_sleep_interval_mmsec = 3;

#if !defined(_WIN32) && defined(_SET_THREAD_NAME_)
	pthread_setname_np(pthread_self(), "main_wss");
#endif
	
	do {
		if (!_mp::csystem::daemonize_on_linux(s_pid_file_full_path, std::wstring(), _signal_handler)) {
			gn_result = _mp::exit_error_daemonize;
			continue;
		}

#ifdef _WIN32
		SetConsoleCtrlHandler(_console_handler, TRUE);
#endif

		b_need_remove_pid_file = true;

		//////////////////////////////////////////////////////////////
		//check single instance
		ptr_file_lock_for_single_instance = _mp::csystem::get_file_lock_for_single_instance(_mp::_coffee::CONST_S_COFFEE_MGMT_FILE_LOCK_FOR_SINGLE);
		if (!ptr_file_lock_for_single_instance) {
			gn_result = _mp::exit_error_already_running;
			continue;//Another instance is already running.
		}

		//////////////////////////////////////////////////////////////
		//setup controller
		gptr_tx_ctl_pipe = std::make_shared<_mp::cnamed_pipe>(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME, true);
		gptr_rx_ctl_pipe = std::make_shared<_mp::cnamed_pipe>(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME_OF_SERVER_RESPONSE, true);

		// load coffee manager ini file
		cfile_coffee_manager_ini& ini(cfile_coffee_manager_ini::get_instance());
		bool b_ini_load = ini.load_definition_file();

		//////////////////////////////////////////////////////////////
		//setup tracing system
		_mp::clog& log(_mp::clog::get_instance());
		log.enable_trace(s_pipe_name_of_trace, ini.get_log_enable()); //enable trace by server mode

		//////////////////////////////////////////////////////////////
		//setup logging system
		log.config(
			s_log_root_folder_except_backslash
			, 6
			, std::wstring(L"coffee_manager")
			,std::wstring(L"elpusk-hid-d")
			, std::wstring(L"elpusk-hid-d")
		);
		log.remove_log_files_older_then_now_day(ini.get_log_days_to_keep());
		log.enable(ini.get_log_enable());
#ifdef _DEBUG
		log.log_fmt(L"[I] START LOGGING ON DEBUG.\n");
		log.trace(L"[I] START TRACING ON DEBUG.\n");
#else
		log.log_fmt(L"[I] START LOGGING ON RELEASE.\n");
		log.trace(L"[I] START TRACING ON RELEASE.\n");
#endif
		log.log_fmt(L"%ls", ini.get_string().c_str());
		log.trace(L"%ls", ini.get_string().c_str());

		if (!gptr_tx_ctl_pipe) {
			log.log_fmt(L"[E] %ls | create tx controller pipe.\n", __WFUNCTION__);
			log.trace(L"[E] %ls | create tx controller pipe.\n", __WFUNCTION__);
			gn_result = _mp::exit_error_create_ctl_pipe;
			continue;
		}
		if (!gptr_rx_ctl_pipe) {
			log.log_fmt(L"[E] %ls | rx create controller pipe.\n", __WFUNCTION__);
			log.trace(L"[E] %ls | rx create controller pipe.\n", __WFUNCTION__);
			gn_result = _mp::exit_error_create_ctl_pipe;
			continue;
		}

#ifndef _WIN32
		log.log_fmt(L"[I] sid: %5d, pgid: %5d, pid: %5d, ppid: %5d.\n", (int)getsid(0), (int)getpgid(0), (int)getpid(), (int)getppid());
		log.trace(L"[I] sid: %5d, pgid: %5d, pid: %5d, ppid: %5d.\n", (int)getsid(0), (int)getpgid(0), (int)getpid(), (int)getppid());
#endif
		//////////////////////////////////////////////////////////////
		// load dev_lib.dll(.so)
		if (!cdev_lib::get_instance().load(s_dev_lib_dll_abs_full_path,&log)) {
			log.log_fmt(L"[E] %ls | load dev_lib.dll(.so) | %ls.\n", __WFUNCTION__, s_dev_lib_dll_abs_full_path.c_str());
			log.trace(L"[E] %ls | load dev_lib.dll(.so) | %ls.\n", __WFUNCTION__, s_dev_lib_dll_abs_full_path.c_str());
			gn_result = _mp::exit_error_load_dev_lib;
		}

		//////////////////////////////////////////////////////////////
		// setup secure web socket server
		bool b_tls = ini.get_tls_enable();
		unsigned short w_port = (unsigned short)ini.get_server_port();
		int n_thread_for_server(1);

		// create cserver instance.
		// create cctl_svr instance.(_mp::cserver::get_instance() - first time)
		// create cmain_ctl instance.(_mp::cmain_ctl::get_instance() by cctl_svr - first time)
		_mp::cserver & wss_svr(
			_mp::cserver::get_instance(ll_server_worker_sleep_interval_mmsec, &_mp::clog::get_instance())
		);

		wss_svr.set_port(w_port)
			.set_ssl(b_tls)
			.set_cert_file(s_server_certificate_file)
			.set_private_key_file(s_server_private_key_file)
			.set_timeout_websocket_upgrade_req(ini.get_msec_timeout_ws_server_wait_for_websocket_upgrade_req_of_client())
			.set_timeout_idle(ini.get_msec_timeout_ws_server_wait_for_idle())
			.set_timeout_ssl_handshake_complete(ini.get_msec_timeout_ws_server_wait_for_ssl_handshake_complete());

		// start wss
		// create lib_hid instance.
		// create all cdev_ctl instance by cctl_svr.
		if (!wss_svr.start(n_thread_for_server, s_root_folder_except_backslash)) {
			log.log_fmt(L"[E] %ls | cserver::get_instance().start().\n", __WFUNCTION__);
			log.trace(L"[E] - %ls | cserver::get_instance().start().\n", __WFUNCTION__);
			gn_result = _mp::exit_error_start_server;
			continue;
		}
		gn_result = EXIT_SUCCESS;
		log.log_fmt(L"[I] %ls | cserver::get_instance().start().\n", __WFUNCTION__);
		log.trace(L"[I] - %ls | cserver::get_instance().start().\n", __WFUNCTION__);
		std::wstring s_data;

		std::array<std::wstring, 6> ss_pre_req_for_opt_time = {
			_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_EX_REQ_PRE_WOKER_SLEEP_TIME,
			_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_EX_REQ_PRE_DEV_PLUG_IN_OUT_SLEEP_TIME,
			_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_EX_REQ_PRE_DEV_API_TX_LOOP_INTERVAL,
			_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_EX_REQ_PRE_DEV_API_RX_LOOP_INTERVAL,
			_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_EX_REQ_PRE_DEV_RX_Q_LOOP_INTERVAL,
			_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_EX_REQ_PRE_CTL_PIPE_CHECK_INTERVAL
		};

		bool b_req(false);
		int n_vid(0), n_pid(0);
		//lib_hid 는 wss_svr.start() 가 성공하면, 생성되는 instance 이므로, wss_svr.start() 보다 먼저 사용 불가.
		_mp::clibhid& lib_hid(_mp::clibhid::get_instance());
		long long ll_ctl_pipe_check_interval_mmsec(_mp::_coffee::CONST_N_COFFEE_MGMT_SLEEP_INTERVAL_MMSEC);
		do {
			if (!gptr_tx_ctl_pipe) {
				std::this_thread::sleep_for(std::chrono::milliseconds(ll_ctl_pipe_check_interval_mmsec));
				continue;
			}
			if (!gptr_tx_ctl_pipe->read(s_data)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(ll_ctl_pipe_check_interval_mmsec));
				continue;
			}
			if (s_data.empty()) {
				continue; // skip empty input
			}

			if (_mp::ccoffee_pipe::is_ctl_request_for_terminate_server(s_data)) {
				// 응답불요.
				gb_run_main_loop = false;
				log.log_fmt(L"[I] %ls | req - server stop.\n", __WFUNCTION__);
				log.trace(L"[I] - %ls | req - server stop.\n", __WFUNCTION__);
				gn_result = _mp::exit_info_ctl_pipe_requst_terminate;
				continue;
			}

			std::tie(b_req, n_vid, n_pid) = _mp::ccoffee_pipe::is_ctl_request_for_consider_to_removed(s_data);
			if (b_req) {
				//응답 필요.
				if (lib_hid.consider_to_be_removed(n_vid, n_pid)) {
					log.log_fmt(L"[I] %ls | req - consider_to_be_removed.\n", __WFUNCTION__);
					log.trace(L"[I] - %ls | req - consider_to_be_removed.\n", __WFUNCTION__);
				}
				else {
					log.log_fmt(L"[E] %ls | req - consider_to_be_removed.\n", __WFUNCTION__);
					log.trace(L"[E] - %ls | req - consider_to_be_removed.\n", __WFUNCTION__);
				}

				if (!gptr_rx_ctl_pipe) {
					log.log_fmt(L"[E] %ls | rsp - pipe null.\n", __WFUNCTION__);
					log.trace(L"[E] %ls | rsp - pipe null.\n", __WFUNCTION__);
					continue;
				}
				if (!gptr_rx_ctl_pipe->write(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_RSP_STOP_DEV)) {
					log.log_fmt(L"[E] %ls | rsp - consider_to_be_removed.\n", __WFUNCTION__);
					log.trace(L"[E] - %ls | rsp - consider_to_be_removed.\n", __WFUNCTION__);
				}
				continue;
			}
			std::tie(b_req, n_vid, n_pid) = _mp::ccoffee_pipe::is_ctl_request_for_cancel_consider_to_removed(s_data);
			if (b_req) {
				//응답 필요.
				if (lib_hid.cancel_considering_dev_as_removed(n_vid, n_pid)) {
					log.log_fmt(L"[I] %ls | req - cancel_consider_to_be_removed.\n", __WFUNCTION__);
					log.trace(L"[I] - %ls | req - cancel_consider_to_be_removed.\n", __WFUNCTION__);
				}
				else {
					log.log_fmt(L"[E] %ls | req - cancel_consider_to_be_removed.\n", __WFUNCTION__);
					log.trace(L"[E] - %ls | req - cancel_consider_to_be_removed.\n", __WFUNCTION__);
				}

				if (!gptr_rx_ctl_pipe) {
					log.log_fmt(L"[E] %ls | rsp - pipe null.\n", __WFUNCTION__);
					log.trace(L"[E] %ls | rsp - pipe null.\n", __WFUNCTION__);
					continue;
				}
				if (!gptr_rx_ctl_pipe->write(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_RSP_START_DEV)) {
					log.log_fmt(L"[E] %ls | rsp - cancel_consider_to_be_removed.\n", __WFUNCTION__);
					log.trace(L"[E] - %ls | rsp - cancel_consider_to_be_removed.\n", __WFUNCTION__);
				}
				continue;
			}

			// pipe 로 부터 얻은 s_data 가 ss_pre_req_for_opt_time[0] 으로 시작하는 하고,':' 뒤에 숫자만 있으면,
			// 그 숫자를 long long 으로 변환해서 log.trace() 에 출력하는 코드
			long long ll_number(0);
			bool b_valid(false);
			int n_req = 0;
			std::tie(b_valid, ll_number) = _get_interval_from_ctl_pipe(s_data, ss_pre_req_for_opt_time[n_req]);
			if (b_valid) {
				log.log_fmt(L"[I] - %ls | ll_server_worker_sleep_interval_mmsec = %lld\n", __WFUNCTION__, ll_number);
				log.trace(L"[I] - %ls | ll_server_worker_sleep_interval_mmsec = %lld\n", __WFUNCTION__, ll_number);
				wss_svr.set_worker_sleep_interval(ll_number);
				s_data.clear();
				continue;
			}
			//
			++n_req;
			std::tie(b_valid, ll_number) = _get_interval_from_ctl_pipe(s_data, ss_pre_req_for_opt_time[n_req]);
			if (b_valid) {
				log.log_fmt(L"[I] - %ls | ll_dev_plug_in_out_sleep_interval_mmsec = %lld\n", __WFUNCTION__, ll_number);
				log.trace(L"[I] - %ls | ll_dev_plug_in_out_sleep_interval_mmsec = %lld\n", __WFUNCTION__, ll_number);
				wss_svr.set_dev_pluginout_check_interval(ll_number);
				s_data.clear();
				continue;
			}
			//.......
			++n_req;
			std::tie(b_valid, ll_number) = _get_interval_from_ctl_pipe(s_data, ss_pre_req_for_opt_time[n_req]);
			if (b_valid) {
				log.log_fmt(L"[I] - %ls | ll_dev_tx_api_loop_interval_mmsec = %lld\n", __WFUNCTION__, ll_number);
				log.trace(L"[I] - %ls | ll_dev_tx_api_loop_interval_mmsec = %lld\n", __WFUNCTION__, ll_number);
				wss_svr.set_dev_tx_by_api_check_interval(ll_number);
				s_data.clear();
				continue;
			}
			//
			++n_req;
			std::tie(b_valid, ll_number) = _get_interval_from_ctl_pipe(s_data, ss_pre_req_for_opt_time[n_req]);
			if (b_valid) {
				log.log_fmt(L"[I] - %ls | ll_dev_rx_api_loop_interval_mmsec = %lld\n", __WFUNCTION__, ll_number);
				log.trace(L"[I] - %ls | ll_dev_rx_api_loop_interval_mmsec = %lld\n", __WFUNCTION__, ll_number);
				wss_svr.set_dev_rx_by_api_in_rx_worker_check_interval(ll_number);
				s_data.clear();
				continue;
			}

			//rx_q_check_interval must be 1msec(default) - this is tested value.
			++n_req;
			std::tie(b_valid, ll_number) = _get_interval_from_ctl_pipe(s_data, ss_pre_req_for_opt_time[n_req]);
			if (b_valid) {
				log.log_fmt(L"[I] - %ls | ll_dev_rx_q_loop_interval_mmsec = %lld\n", __WFUNCTION__, ll_number);
				log.trace(L"[I] - %ls | ll_dev_rx_q_loop_interval_mmsec = %lld\n", __WFUNCTION__, ll_number);
				wss_svr.set_dev_rx_q_check_interval(ll_number);
				s_data.clear();
				continue;
			}
			//
			++n_req;
			std::tie(b_valid, ll_number) = _get_interval_from_ctl_pipe(s_data, ss_pre_req_for_opt_time[n_req]);
			if (b_valid) {
				log.log_fmt(L"[I] - %ls | ll_ctl_pipe_check_interval_mmsec = %lld\n", __WFUNCTION__, ll_number);
				log.trace(L"[I] - %ls | ll_ctl_pipe_check_interval_mmsec = %lld\n", __WFUNCTION__, ll_number);

				if (ll_number >= 0) {
					ll_ctl_pipe_check_interval_mmsec = ll_number;
				}
				s_data.clear();
				continue;
			}

			log.log_fmt(L"[I] - user input | %ls\n", s_data.c_str());
			log.trace(L"[I] - user input | %ls\n", s_data.c_str());

			s_data.clear();
		} while (gb_run_main_loop);

		gptr_tx_ctl_pipe.reset();
		gptr_rx_ctl_pipe.reset();

		wss_svr.stop();
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
	return gn_result;
}


std::pair<bool, long long> _get_interval_from_ctl_pipe(
	const std::wstring& s_pipe_data
	, const std::wstring& s_expected_prefix
)
{
	bool b_valid(false);
	long long ll_interval(0);

	do {
		if(s_pipe_data.empty()) {
			continue;;
		}
		if (s_pipe_data.find(s_expected_prefix) != 0) {
			continue;
		}
		// remove prefix
		std::wstring number_str = s_pipe_data.substr(s_expected_prefix.length());

		// remaining string should be a number
		if (std::all_of(number_str.begin(), number_str.end(), ::iswdigit)) {
			b_valid = true;
			try {
				ll_interval = std::stoll(number_str);
			}
			catch (const std::exception& e) {
				b_valid = false;
			}
		}

	} while (false);

	return std::make_pair(b_valid, ll_interval);
}

void _signal_handler(int signum)
{
#ifndef _WIN32
	do {

		if (signum == SIGTERM) {
			// Handle termination gracefully
			if (!gptr_tx_ctl_pipe) {
				_exit(gn_result);
			}

			gptr_tx_ctl_pipe.reset();
			gptr_rx_ctl_pipe.reset();

			gb_run_main_loop = false;
			continue;//terminated by main loop
		}
		if (signum == SIGHUP) {
			// notify reload initial file!
		}

	} while (false);
#endif
}

#ifdef _WIN32
BOOL WINAPI _console_handler(DWORD signal)
{
	switch (signal) {
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:

	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:

		gptr_tx_ctl_pipe.reset();
		gptr_rx_ctl_pipe.reset();

		gb_run_main_loop = false;
		return FALSE;// 종료
	default:
		return FALSE;
	}
}
#endif