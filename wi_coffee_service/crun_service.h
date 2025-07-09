#pragma once


#include <Windows.h>
#include <winsvc.h>
#include <tlhelp32.h>

#include <mutex>
#include <string>

#define BOOST_JSON_HEADER_ONLY
#include <boost/json/src.hpp>

#include <mp_type.h>
#include <mp_clog.h>
#include <mp_csystem_.h>
#include <mp_coffee_path.h>
#include <mp_cstring.h>
#include <mp_cnamed_pipe_.h>

#include <cini_service.h>

#include "cwarp.h"

#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "Wtsapi32.lib")

using namespace std;

class crun_service
{
private:
	struct ProcessPara {
		wstring s_command_line;
		wstring s_working_directory;
		UINT n_pause_start;
		UINT n_pause_end;
		bool b_user_interface;
		bool b_restart;
		wstring s_user_name;
		wstring s_domain;
		wstring s_password;
	};

	enum {
		time_out_msec_for_wait_end_process = 5000
	};
public:

	static crun_service & get_instance()
	{
		static crun_service service;
		return service;
	}

	DWORD create_worker()
	{
		cwarp::chandle h_thread((HANDLE)::_beginthreadex(NULL, 0, crun_service::_worker, NULL, 0, NULL));
		if (h_thread.get_handle() == NULL)
			return ::GetLastError();	//return error code
		else {
			return 0;	//success
		}
	}

	DWORD launch_service()
	{
		m_dispatch_table[0].lpServiceName = &m_v_DupServiceName[0];
		m_dispatch_table[0].lpServiceProc = crun_service::_service_main;

		m_dispatch_table[1].lpServiceName = NULL;
		m_dispatch_table[1].lpServiceProc = NULL;

		if (!StartServiceCtrlDispatcher(m_dispatch_table)) {
			return ::GetLastError();	//return error code 
		}
		else
			return 0;
	}

	bool start_processes_different_session(DWORD dwSessionId)
	{
		bool b_result(false);

		do {
			_mp::cnamed_pipe::type_ptr ptr_ctl_pipe;
			ptr_ctl_pipe = std::make_shared<_mp::cnamed_pipe>(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME, false);
			if (!ptr_ctl_pipe) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | cannot created control pipe.\n", __WFUNCTION__);
				continue;
			}

			if (ptr_ctl_pipe->is_ini()) {
				_mp::clog::get_instance().log_fmt(L"[W] %s | manager is executed already.\n", __WFUNCTION__);
				ptr_ctl_pipe.reset();
				continue;
			}
			ptr_ctl_pipe.reset();

			PROCESS_INFORMATION pi;
			STARTUPINFO si;
			BOOL bResult = FALSE;
			DWORD winlogonPid;
			DWORD dwCreationFlags;
			//////////////////////////////////////////
			// Find the winlogon process
			////////////////////////////////////////

			PROCESSENTRY32 procEntry;

			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (hSnap == INVALID_HANDLE_VALUE){
				_mp::clog::get_instance().log_fmt(L"[E] %s | CreateToolhelp32Snapshot error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}

			cwarp::chandle handle_snap(hSnap);

			procEntry.dwSize = sizeof(PROCESSENTRY32);
			if (!Process32First(handle_snap.get_handle(), &procEntry)){
				_mp::clog::get_instance().log_fmt(L"[E] %s | Process32First error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}

			do{
				if (_wcsicmp((const wchar_t*)procEntry.szExeFile, L"winlogon.exe") == 0)
				{
					// We found a winlogon process...make sure it's running in the console session
					DWORD winlogonSessId = 0;
					if (ProcessIdToSessionId(procEntry.th32ProcessID, &winlogonSessId) && winlogonSessId == dwSessionId){
						winlogonPid = procEntry.th32ProcessID;
						break;
					}
				}

			} while (Process32Next(hSnap, &procEntry));

			////////////////////////////////////////////////////////////////////////
			/*
			HANDLE hUserToken;
			WTSQueryUserToken(dwSessionId,&hUserToken);
			Cns_WarpHandle UserTokenHandle( hUserToken );
			*/
			wchar_t sDesktop[] = L"winsta0\\default";

			dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
			ZeroMemory(&si, sizeof(STARTUPINFO));
			si.cb = sizeof(STARTUPINFO);
			si.lpDesktop = sDesktop;
			ZeroMemory(&pi, sizeof(pi));
			TOKEN_PRIVILEGES tp;
			LUID luid;

			cwarp::chandle ProcessHandle(::OpenProcess(MAXIMUM_ALLOWED, FALSE, winlogonPid));

			HANDLE hPToken;

			if (!::OpenProcessToken(
				ProcessHandle.get_handle(),
				TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID | TOKEN_READ | TOKEN_WRITE,
				&hPToken)
				){
				_mp::clog::get_instance().log_fmt(L"[E] %s | Process token open Error = %u.\n", __WFUNCTION__ ,GetLastError());
				continue;
			}
			cwarp::chandle PTokenHandle(hPToken);

			if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)){
				_mp::clog::get_instance().log_fmt(L"[E] %s | LookupPrivilegeValue Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = luid;
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			//
			HANDLE hUserTokenDup;
			::DuplicateTokenEx(PTokenHandle.get_handle(), MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hUserTokenDup);
			
			cwarp::chandle UserTokenDupHandle(hUserTokenDup);

			//Adjust Token privilege
			::SetTokenInformation(UserTokenDupHandle.get_handle(), TokenSessionId, (void*)&dwSessionId, sizeof(dwSessionId));

			if (!AdjustTokenPrivileges(UserTokenDupHandle.get_handle(), FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, NULL)){
				_mp::clog::get_instance().log_fmt(L"[E] %s | AdjustTokenPrivileges Error = %u.\n", __WFUNCTION__, GetLastError());
			}

			if (GetLastError() == ERROR_NOT_ALL_ASSIGNED){
				_mp::clog::get_instance().log_fmt(L"[E] %s | Token does not have the provilege.\n", __WFUNCTION__);
			}

			LPVOID pEnv = NULL;

			if (::CreateEnvironmentBlock(&pEnv, UserTokenDupHandle.get_handle(), TRUE)){
				dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
			}
			else
				pEnv = NULL;
			//
			cwarp::cenvironment_block EnvBlock(pEnv);

			// Launch the process in the client's logon session.
			_mp::type_v_ws_buffer v(_MAX_PATH,0);
			m_process.s_command_line.copy(&v[0], m_process.s_command_line.size() + 1);

			bResult = ::CreateProcessAsUser(
				UserTokenDupHandle.get_handle(),            // client's access token
				&v[0],              // file to execute
				NULL,     // command line
				NULL,              // pointer to process SECURITY_ATTRIBUTES
				NULL,              // pointer to thread SECURITY_ATTRIBUTES
				FALSE,             // handles are not inheritable
				dwCreationFlags,  // creation flags
				EnvBlock.get_block(),              // pointer to new environment block 
				m_process.s_working_directory.size() == 0 ? NULL : m_process.s_working_directory.c_str(),              // name of current directory 
				&si,               // pointer to STARTUPINFO structure
				&m_process_info		// receives information about new process
			);

			if (!bResult) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | CreateProcessAsUser Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}

			// End impersonation of client.
			_mp::clog::get_instance().log_fmt(L"[I] %s | manager has been started..\n", __WFUNCTION__);
			_logging_process_info();
			//
			b_result = true;
		} while (false);
		return b_result;
	}

	bool start_processes_different_session()
	{
		return start_processes_different_session(::WTSGetActiveConsoleSessionId());
	}

	void end_process()
	{
		// end a program started by the service
		_mp::cnamed_pipe::type_ptr ptr_ctl_pipe;
		ptr_ctl_pipe = std::make_shared<_mp::cnamed_pipe>(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_PIPE_NAME, false);
		if (ptr_ctl_pipe) {
			if (!ptr_ctl_pipe->is_ini()) {
				_mp::clog::get_instance().log_fmt(L"[W] %s | manageris already killed.\n", __WFUNCTION__);
				ptr_ctl_pipe.reset();
				return;
			}

			if (ptr_ctl_pipe->write(_mp::_coffee::CONST_S_COFFEE_MGMT_CTL_REQ)) {
				::Sleep(100);
				_mp::clog::get_instance().log_fmt(L"[I] %s | sevice send Kill-signal to manager.\n", __WFUNCTION__);
			}
			else {
				_mp::clog::get_instance().log_fmt(L"[E] %s | cannot write to control pipe.\n", __WFUNCTION__);
			}
		}
		else {
			_mp::clog::get_instance().log_fmt(L"[E] %s | cannot created control pipe.\n", __WFUNCTION__);
		}
		ptr_ctl_pipe.reset();


		//wait the end if process
		DWORD dw_check_interval_msec(100);
		DWORD dwCode = 0;
		DWORD dw_remain_time = crun_service::time_out_msec_for_wait_end_process;
		bool b_run(true);
		do {
			if (!::GetExitCodeProcess(m_process_info.hProcess, &dwCode)) {
				_mp::clog::get_instance().log_fmt(L"[W] %s | GetExitCodeProcess failed : error code = %u.\n", __WFUNCTION__,GetLastError());
				b_run = false;
				Sleep(5*dw_check_interval_msec);
				continue;
			}

			if (dwCode == STILL_ACTIVE) {
				if (dw_remain_time < dw_check_interval_msec) {
					dw_remain_time = 0;
					_mp::clog::get_instance().log_fmt(L"[E] %s | time-out | waiting for the end of manager.\n", __WFUNCTION__);
					b_run = false;
					continue;
				}

				dw_remain_time -= dw_check_interval_msec;//decrease 100 msec timeout
				Sleep(dw_check_interval_msec);
				continue;//check continue
			}
			//terminated proccess
			_mp::clog::get_instance().log_fmt(L"[I] %s | manager is terminated.\n", __WFUNCTION__);
			b_run = false;
		} while (b_run);

		try // close handles to avoid ERROR_NO_SYSTEM_RESOURCES
		{
			::CloseHandle(m_process_info.hThread);
			::CloseHandle(m_process_info.hProcess);
		}
		catch (...) {}
	}

	bool kill_service()
	{
		// kill service with given name
		bool b_result(false);

		do{
			cwarp::cservice_handle schSCManager(::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS));
			if (schSCManager.get_handle() == NULL) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | OpenSCManager Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}

			// open the service
			cwarp::cservice_handle schService(::OpenService(schSCManager.get_handle(), m_s_service_name.c_str(), SERVICE_ALL_ACCESS));
			if (schService.get_handle() == NULL) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | OpenService Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}

			// call ControlService to kill the given service
			SERVICE_STATUS status;
			if (!::ControlService(schService.get_handle(), SERVICE_CONTROL_STOP, &status)){
				unsigned long l_result(GetLastError());
				if (l_result != ERROR_SERVICE_NOT_ACTIVE) {
					_mp::clog::get_instance().log_fmt(L"[E] %s | ControlService Error = %u.\n", __WFUNCTION__, GetLastError());
					continue;
				}
				_mp::clog::get_instance().log_fmt(L"[W] %s | ControlService | SERVICE_NOT_ACTIVE.\n", __WFUNCTION__);
			}
			//
			b_result = true;
		} while (false);
		return b_result;
	}
	bool run_service()
	{
		// run service with given name
		bool b_result(false);
		do {
			cwarp::cservice_handle schSCManager(::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS));
			if (schSCManager.get_handle() == NULL) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | OpenSCManager Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}
			// open the service
			cwarp::cservice_handle schService(::OpenService(schSCManager.get_handle(), m_s_service_name.c_str(), SERVICE_ALL_ACCESS));
			if (schService.get_handle() == NULL) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | OpenService Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}
			// call StartService to run the service
			if (!StartService(schService.get_handle(), 0, NULL)){
				_mp::clog::get_instance().log_fmt(L"[E] %s | StartService Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}
			//
			b_result = true;
		} while (false);
		return b_result;
	}

	VOID uninstall()
	{
		do {
			cwarp::cservice_handle schSCManager(::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS));
			if (schSCManager.get_handle() == NULL) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | OpenSCManager Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}

			cwarp::cservice_handle schService(::OpenService(schSCManager.get_handle(), m_s_service_name.c_str(), SERVICE_ALL_ACCESS));
			if (schService.get_handle() == NULL) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | OpenService Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}
			if (!DeleteService(schService.get_handle())) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | DeleteService Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}
			_mp::clog::get_instance().log_fmt(L"[I] %s | Service is uninstalled.\n", __WFUNCTION__);
		} while (false);
	}
	VOID install()
	{
		do {
			SERVICE_DESCRIPTION sd;
			wchar_t szDesc[] = L"The coffee device manager for usb devices. 2nd";

			cwarp::cservice_handle schSCManager(::OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE));
			if (schSCManager.get_handle() == NULL) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | OpenSCManager Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}

			cwarp::cservice_handle schService(::CreateService
			(
				schSCManager.get_handle(),	/* SCManager database      */
				m_s_service_name.c_str(),			/* name of service         */
				m_s_service_name.c_str(),			/* service name to display */
				SERVICE_ALL_ACCESS,        /* desired access          */
				//SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS , /* service type            */ 
				SERVICE_WIN32_OWN_PROCESS, /* service type            */
				SERVICE_AUTO_START,      /* start type              */
				SERVICE_ERROR_NORMAL,      /* error control type      */
				m_s_service_exe_file_name.c_str(),			/* service's binary        */
				NULL,                      /* no load ordering group  */
				NULL,                      /* no tag identifier       */
				NULL,                      /* no dependencies         */
				NULL,                      /* LocalSystem account     */
				NULL
			));                   /* no password             */
			if (schService.get_handle() == NULL) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | CreateService Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}

			sd.lpDescription = szDesc;

			if (!ChangeServiceConfig2(schService.get_handle(), SERVICE_CONFIG_DESCRIPTION, &sd)) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | ChangeServiceConfig2 Error = %u.\n", __WFUNCTION__, GetLastError());
			}

			_mp::clog::get_instance().log_fmt(L"[I] %s | Service is installed.\n", __WFUNCTION__);
		} while (false);
	}

	const wstring & get_service_name() const { return m_s_service_name; }
	const wstring & get_ini_file_name() const { return  m_s_ini_file_name; }
	const wstring & get_exe_file_name() const { return m_s_service_exe_file_name; }

	//setter

private:
	crun_service(void) : 
		m_h_service_status(NULL)
		,m_v_DupServiceName(_MAX_PATH, 0)
		, m_h_GEvent(NULL)
		, m_service_status({ 0, })
	{
		_load_ini();

		cini_service& cini(cini_service::get_instance());

		std::wstring s_log_root_folder_except_backslash = _mp::ccoffee_path::get_path_of_coffee_logs_root_folder_except_backslash();
		_mp::clog::get_instance().config(s_log_root_folder_except_backslash, 6, std::wstring(L"coffee_manager"), std::wstring(L"coffee_service"), std::wstring(L"coffee_service"));
		_mp::clog::get_instance().remove_log_files_older_then_now_day(cini.get_log_days_to_keep());
		_mp::clog::get_instance().enable(cini.get_log_enable());
		
		_logging_ini();
	}
	~crun_service(void)
	{

	}

private:
	static VOID WINAPI _service_main(DWORD dwArgc, LPTSTR *lpszArgv)
	{
		do {
			DWORD   status = 0;
			DWORD   specificError = 0xfffffff;

			crun_service & svr = crun_service::get_instance();

			svr.m_service_status.dwServiceType = SERVICE_WIN32;
			svr.m_service_status.dwCurrentState = SERVICE_START_PENDING;
			svr.m_service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SESSIONCHANGE;
			svr.m_service_status.dwWin32ExitCode = 0;
			svr.m_service_status.dwServiceSpecificExitCode = 0;
			svr.m_service_status.dwCheckPoint = 0;
			svr.m_service_status.dwWaitHint = 0;

			svr.m_h_service_status = ::RegisterServiceCtrlHandlerEx(svr.m_s_service_name.c_str(), crun_service::_service_handler_ex, NULL);
			if (svr.m_h_service_status == NULL) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | RegisterServiceCtrlHandler Error = %u.\n", __WFUNCTION__, GetLastError());
				continue;
			}

			// Initialization complete - report running status 
			svr.m_service_status.dwCurrentState = SERVICE_RUNNING;
			svr.m_service_status.dwCheckPoint = 0;
			svr.m_service_status.dwWaitHint = 0;
			if (!SetServiceStatus(svr.m_h_service_status, &(svr.m_service_status))) {
				_mp::clog::get_instance().log_fmt(L"[E] %s | SetServiceStatus Error = %u.\n", __WFUNCTION__, GetLastError());
			}

			svr.m_process_info.hProcess = NULL;

			_mp::clog::get_instance().log_fmt(L"[I] %s | StartProcessesDifferentSession.\n", __WFUNCTION__);
			svr.start_processes_different_session();
		} while (false);
	}
	static DWORD WINAPI _service_handler_ex(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
	{
		DWORD dwResult = ERROR_CALL_NOT_IMPLEMENTED;
		wstring s_msg;
		int i;

		crun_service & svr = crun_service::get_instance();

		switch (dwControl) {
		case SERVICE_CONTROL_PAUSE:
			_mp::clog::get_instance().log_fmt(L"[I] %s | SERVICE_CONTROL_PAUSE.\n", __WFUNCTION__);
			svr.m_service_status.dwCurrentState = SERVICE_PAUSED;
			dwResult = NO_ERROR;
			break;
		case SERVICE_CONTROL_CONTINUE:
			_mp::clog::get_instance().log_fmt(L"[I] %s | SERVICE_CONTROL_CONTINUE.\n", __WFUNCTION__);
			svr.m_service_status.dwCurrentState = SERVICE_RUNNING;
			dwResult = NO_ERROR;
			break;
		case SERVICE_CONTROL_STOP:
			_mp::clog::get_instance().log_fmt(L"[I] %s | SERVICE_CONTROL_STOP.\n", __WFUNCTION__);
			svr.m_service_status.dwWin32ExitCode = 0;
			svr.m_service_status.dwCurrentState = SERVICE_STOPPED;
			svr.m_service_status.dwCheckPoint = 0;
			svr.m_service_status.dwWaitHint = 0;

			// terminate process
			svr.end_process();
			dwResult = NO_ERROR;
			break;
		case SERVICE_CONTROL_SHUTDOWN:
			_mp::clog::get_instance().log_fmt(L"[I] %s | SERVICE_CONTROL_SHUTDOWN.\n", __WFUNCTION__);
			svr.m_service_status.dwWin32ExitCode = 0;
			svr.m_service_status.dwCurrentState = SERVICE_STOPPED;
			svr.m_service_status.dwCheckPoint = 0;
			svr.m_service_status.dwWaitHint = 0;

			// terminate process
			svr.end_process();
			dwResult = NO_ERROR;
			break;
		case SERVICE_CONTROL_SESSIONCHANGE:
			_mp::clog::get_instance().log_fmt(L"[I] %s | SERVICE_CONTROL_SESSIONCHANGE.\n", __WFUNCTION__);
			WTSSESSION_NOTIFICATION wtsno;
			CopyMemory(&wtsno, lpEventData, sizeof(WTSSESSION_NOTIFICATION));
			switch (dwEventType)
			{
			case WTS_SESSION_LOGON:
				/*
				OSVERSIONINFO osi;
				osi.dwOSVersionInfoSize = sizeof(osi);
				if (GetVersionEx(&osi))
				{
					//XP에서는새로운세션로그인시바로프로그램을실행시키면
					//CreateProcessAsUser 에서233번오류가발생한다.
					//XP 자체의 오류로 Vista에서는 해결되었다.
					//그래서, 로그온화면이사라질때까지기다린후프로그램들을실행시킨다.
					if ((osi.dwMajorVersion <= 5) && (wtsno.dwSessionId > 0))
					{
						Sleep (3000);
					}
				}
				*/

				svr.end_process();
				
				if (!crun_service::_is_windows_version_or_greater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0) && (wtsno.dwSessionId > 0)) {
					// GetVersionEx can not be use from windows 8.
					Sleep(3000);
				}


				//run program
				_mp::clog::get_instance().log_fmt(L"[I] %s | StartProcessesDifferentSession : dwSessionId = %u\n", __WFUNCTION__, wtsno.dwSessionId);

				for (i = 0; i < 50; i++) {
					Sleep(100);
					if (svr.start_processes_different_session(wtsno.dwSessionId))
						break;//exit for
				}//end for
				if (i == 50) {
					_mp::clog::get_instance().log_fmt(L"[E] %s | Fail start manager\n", __WFUNCTION__);
				}
				break;
			default:
				break;
			};

			dwResult = NO_ERROR;
			break;
		default:
			_mp::clog::get_instance().log_fmt(L"[I] %s | SERVICE_CONTROL_UNKNOWN.\n", __WFUNCTION__);
			dwResult = ERROR_CALL_NOT_IMPLEMENTED;
			break;
		}//end switch

		if (!SetServiceStatus(svr.m_h_service_status, &(svr.m_service_status))) {
			_mp::clog::get_instance().log_fmt(L"[E] %s | SetServiceStatus Error = %u.\n", __WFUNCTION__, GetLastError());
		}

		return dwResult;
	}
	static unsigned WINAPI _worker(void *pParam)
	{
		crun_service & svr = crun_service::get_instance();

		int nCheckProcessSeconds(static_cast<int>(svr.m_n_check_process_seconds));

		while (nCheckProcessSeconds > 0) {

			::Sleep(1000 * nCheckProcessSeconds);

			if (svr.m_process_info.hProcess == NULL)
				continue;
			if (!svr.m_process.b_restart)
				continue;

			DWORD dwCode = 0;
			if (!::GetExitCodeProcess(svr.m_process_info.hProcess, &dwCode)) {
				_mp::clog::get_instance().log_fmt(L"[W] %s | GetExitCodeProcess failed.\n", __WFUNCTION__);
				continue;
			}

			if (dwCode == STILL_ACTIVE)
				continue;
			try 
			{
				// close handles to avoid ERROR_NO_SYSTEM_RESOURCES
				::CloseHandle(svr.m_process_info.hThread);
				::CloseHandle(svr.m_process_info.hProcess);
			}
			catch (...) {}

			_mp::clog::get_instance().log_fmt(L"[I] %s | start_processes_different_session.\n", __WFUNCTION__);
			//
			if (svr.start_processes_different_session())
				_mp::clog::get_instance().log_fmt(L"[W] %s | restart manager.\n", __WFUNCTION__);
			else
				_mp::clog::get_instance().log_fmt(L"[E] %s | start_processes_different_session.\n", __WFUNCTION__);
		}//end while

		return 0;
	}

	bool _load_ini()
	{
		bool b_result(false);

		cini_service& cini(cini_service::get_instance());

		b_result =  cini.load_definition_file(_mp::ccoffee_path::get_path_of_coffee_svr_ini_file());

		do {
			//load ini .json file
			std::wstring ws_path_ini(_mp::ccoffee_path::get_path_of_coffee_svr_ini_file());
			m_s_ini_file_name = cini.get_ini_file_full_path();
			//
			m_s_service_exe_file_name = _mp::csystem::get_cur_exe_abs_path();
			//
			m_s_service_name = cini.get_name();
			m_n_check_process_seconds = cini.get_process_check_interval_sec();
			m_process.s_command_line = cini.get_command_line();
			m_process.s_working_directory = cini.get_working_directory();
			//
			//
			m_process.n_pause_start = 10; // default 10
			m_process.n_pause_end = 10; // default 10
			//
			m_process.b_user_interface = false; //default
			m_process.b_restart = false;//default
			//
			m_process.s_user_name.clear();
			m_process.s_domain.clear();
			m_process.s_password.clear();
			//
			b_result = true;
		} while (false);
		return b_result;
	}

	void _logging_process_info()
	{
		_mp::clog::get_instance().log_fmt(
			L"[I] | Process Information.\n Process ID = %u\n Thread ID = %u\n Process Handle = 0x%x\n Thread Handle = 0x%x\n"
			, m_process_info.dwProcessId
			, m_process_info.dwThreadId
			, m_process_info.hProcess
			, m_process_info.hThread
		);
	}

	void _logging_ini()
	{
		_mp::clog::get_instance().log_fmt(L"[I] %s | service file  = %s.\n", __WFUNCTION__, m_s_service_exe_file_name.c_str());
		_mp::clog::get_instance().log_fmt(L"[I] %s | process_check_interval_sec = %u.\n", __WFUNCTION__, m_n_check_process_seconds);
		
		_mp::clog::get_instance().log_fmt(L"[I] %s | s_command_line = %s.\n", __WFUNCTION__, m_process.s_command_line.c_str());
		_mp::clog::get_instance().log_fmt(L"[I] %s | s_working_directory = %s.\n", __WFUNCTION__, m_process.s_working_directory.c_str());
		_mp::clog::get_instance().log_fmt(L"[I] %s | n_pause_start = %u.\n", __WFUNCTION__, m_process.n_pause_start);
		_mp::clog::get_instance().log_fmt(L"[I] %s | n_pause_end = %u.\n", __WFUNCTION__, m_process.n_pause_end);

		if(m_process.b_user_interface)
			_mp::clog::get_instance().log_fmt(L"[I] %s | b_user_interface = yes.\n", __WFUNCTION__);
		else
			_mp::clog::get_instance().log_fmt(L"[I] %s | b_user_interface = no.\n", __WFUNCTION__);

		if(m_process.b_restart)
			_mp::clog::get_instance().log_fmt(L"[I] %s | b_restart = yes.\n", __WFUNCTION__);
		else
			_mp::clog::get_instance().log_fmt(L"[I] %s | b_restart = no.\n", __WFUNCTION__);

		_mp::clog::get_instance().log_fmt(L"[I] %s | s_user_name = %s.\n", __WFUNCTION__, m_process.s_user_name.c_str());
		_mp::clog::get_instance().log_fmt(L"[I] %s | s_domain = %s.\n", __WFUNCTION__, m_process.s_domain.c_str());
		_mp::clog::get_instance().log_fmt(L"[I] %s | s_password = %s.\n", __WFUNCTION__, m_process.s_password.c_str());
	}

	static bool _is_windows_version_or_greater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
	{
		OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
		DWORDLONG const dwlConditionMask = VerSetConditionMask(
			VerSetConditionMask(
				VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL),
				VER_MINORVERSION, VER_GREATER_EQUAL),
			VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

		osvi.dwMajorVersion = wMajorVersion;
		osvi.dwMinorVersion = wMinorVersion;
		osvi.wServicePackMajor = wServicePackMajor;

		if (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask))
			return true;
		else
			return false;
	}

private:

	enum { C_TableSize = 2 };

	std::mutex m_mutex;

	SERVICE_STATUS m_service_status;
	SERVICE_STATUS_HANDLE m_h_service_status;

	_mp::type_v_ws_buffer m_v_DupServiceName;
	std::wstring m_s_service_name;
	std::wstring m_s_ini_file_name;
	std::wstring m_s_service_exe_file_name;

	UINT m_n_check_process_seconds;
	ProcessPara m_process;

	PROCESS_INFORMATION m_process_info;

	SERVICE_TABLE_ENTRY m_dispatch_table[C_TableSize];

	HANDLE m_h_GEvent;

private://don't call these method.

	crun_service(const crun_service &);
	crun_service & operator=(const crun_service &);

};

