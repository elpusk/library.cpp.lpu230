#pragma once


#include <Windows.h>
#include <winsvc.h>
#include <tlhelp32.h>

#include <mutex>
#include <string>

#include <mp_type.h>
#include <mp_clog.h>
#include <mp_csystem_.h>
#include <mp_cstring.h>
#include <mp_cnamed_pipe_.h>

#include <cini_service.h>

#include <manager_of_device_of_client.h>

#include "cwarp.h"

class crun_service
{
private:
	struct ProcessPara {
		std::wstring s_command_line;
		std::wstring s_working_directory;
		UINT n_pause_start;
		UINT n_pause_end;
		bool b_user_interface;
		bool b_restart;
		std::wstring s_user_name;
		std::wstring s_domain;
		std::wstring s_password;
	};

	enum {
		time_out_msec_for_wait_end_process = 5000
	};
public:

	static crun_service& get_instance();

	DWORD create_worker();

	DWORD launch_service();

	bool start_processes_different_session(DWORD dwSessionId);

	bool start_processes_different_session();

	void end_process();

	bool kill_service();
	bool run_service();

	VOID uninstall();
	VOID install();

	const std::wstring& get_service_name() const;
	const std::wstring& get_ini_file_name() const;
	const std::wstring& get_exe_file_name() const;

	//setter

private:
	crun_service(void);
	~crun_service(void);

	bool _connect_wss();
	bool _disconnect_wss();

private:
	static void __stdcall _service_main(DWORD dwArgc, LPTSTR* lpszArgv);
	static DWORD __stdcall _service_handler_ex(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
	static unsigned __stdcall _worker(void* pParam);

	bool _load_ini();

	void _logging_process_info();

	static bool _is_windows_version_or_greater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor);

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

